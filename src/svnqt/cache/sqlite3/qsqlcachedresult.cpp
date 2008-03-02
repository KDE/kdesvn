/****************************************************************************
**
** Copyright (C) 1992-2005 Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsqlcachedresult.h"

#include <qvariant.h>
#include <qdatetime.h>
#include <qvaluevector.h>

static const uint initial_cache_size = 128;

class QSqlCachedResultPrivate
{
public:
    QSqlCachedResultPrivate();
    bool canSeek(int i) const;
    inline int cacheCount() const;
    void init(int count, bool fo);
    void cleanup();
    int nextIndex();
    void revertLast();

    QSqlCachedResult::ValueCache cache;
    int rowCacheEnd;
    int colCount;
    bool forwardOnly;
};

QSqlCachedResultPrivate::QSqlCachedResultPrivate():
    rowCacheEnd(0), colCount(0), forwardOnly(false)
{
}

void QSqlCachedResultPrivate::cleanup()
{
    cache.clear();
    forwardOnly = false;
    colCount = 0;
    rowCacheEnd = 0;
}

void QSqlCachedResultPrivate::init(int count, bool fo)
{
    Q_ASSERT(count);
    cleanup();
    forwardOnly = fo;
    colCount = count;
    if (fo) {
        cache.resize(count);
        rowCacheEnd = count;
    } else {
        cache.resize(initial_cache_size * count);
    }
}

int QSqlCachedResultPrivate::nextIndex()
{
    if (forwardOnly)
        return 0;
    int newIdx = rowCacheEnd;
    if (rowCacheEnd == (int)cache.size())
        cache.resize(cache.size() * 2);
/*    if (newIdx + colCount > cache.size()){
        if(cache.size() * 2 < cache.size() + 10000)
            cache.resize(cache.size() * 2);
        else
            cache.resize(cache.size() + 10000);
    }*/
    rowCacheEnd += colCount;

    return newIdx;
}

bool QSqlCachedResultPrivate::canSeek(int i) const
{
    if (forwardOnly || i < 0)
        return false;
    return rowCacheEnd >= (i + 1) * colCount;
}

void QSqlCachedResultPrivate::revertLast()
{
    if (forwardOnly)
        return;
    rowCacheEnd -= colCount;
}

inline int QSqlCachedResultPrivate::cacheCount() const
{
    Q_ASSERT(!forwardOnly);
    Q_ASSERT(colCount);
    return rowCacheEnd / colCount;
}

//////////////

QSqlCachedResult::QSqlCachedResult(const QSqlDriver * db): QSqlResult (db)
{
    d = new QSqlCachedResultPrivate();
}

QSqlCachedResult::~QSqlCachedResult()
{
    delete d;
}

void QSqlCachedResult::init(int colCount)
{
    d->init(colCount, isForwardOnly());
}

bool QSqlCachedResult::fetch(int i)
{
    if ((!isActive()) || (i < 0))
        return false;
    if (at() == i)
        return true;
    if (d->forwardOnly) {
        // speed hack - do not copy values if not needed
        if (at() > i || at() == QSql::AfterLast)
            return false;
        while(at() < i - 1) {
            if (!gotoNext(d->cache, -1))
                return false;
            setAt(at() + 1);
        }
        if (!gotoNext(d->cache, 0))
            return false;
        setAt(at() + 1);
        return true;
    }
    if (d->canSeek(i)) {
        setAt(i);
        return true;
    }
    if (d->rowCacheEnd > 0)
        setAt(d->cacheCount()-1);
    while (at() < i) {
        if (!cacheNext())
            return false;
    }
    return true;
}

bool QSqlCachedResult::fetchNext()
{
    if (d->canSeek(at() + 1)) {
        setAt(at() + 1);
        return true;
    }
    return cacheNext();
}

bool QSqlCachedResult::fetchPrevious()
{
    return fetch(at() - 1);
}

bool QSqlCachedResult::fetchFirst()
{
    if (d->forwardOnly && at() != QSql::BeforeFirst) {
        return false;
    }
    if (d->canSeek(0)) {
        setAt(0);
        return true;
    }
    return cacheNext();
}

bool QSqlCachedResult::fetchLast()
{
    if (at() == QSql::AfterLast) {
        if (d->forwardOnly)
            return false;
        else
            return fetch(d->cacheCount() - 1);
    }

    int i = at();
    while (fetchNext())
        ++i; /* brute force */
    if (d->forwardOnly && at() == QSql::AfterLast) {
        setAt(i);
        return true;
    } else {
        return fetch(i);
    }
}

QVariant QSqlCachedResult::data(int i)
{
    int idx = d->forwardOnly ? i : at() * d->colCount + i;
    if (i >= d->colCount || i < 0 || at() < 0 || idx >= d->rowCacheEnd)
        return QVariant();

    return d->cache.at(idx);
}

bool QSqlCachedResult::isNull(int i)
{
    int idx = d->forwardOnly ? i : at() * d->colCount + i;
    if (i > d->colCount || i < 0 || at() < 0 || idx >= d->rowCacheEnd)
        return true;

    return d->cache.at(idx).isNull();
}

void QSqlCachedResult::cleanup()
{
    setAt(QSql::BeforeFirst);
    setActive(false);
    d->cleanup();
}

bool QSqlCachedResult::cacheNext()
{
    if (!gotoNext(d->cache, d->nextIndex())) {
        d->revertLast();
        return false;
    }
    setAt(at() + 1);
    return true;
}

int QSqlCachedResult::colCount() const
{
    return d->colCount;
}

QSqlCachedResult::ValueCache &QSqlCachedResult::cache()
{
    return d->cache;
}

