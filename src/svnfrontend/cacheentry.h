/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef HELPERSCACHEENTRY_H
#define HELPERSCACHEENTRY_H

#include "svnqt/svnqttypes.h"
#include "svnqt/status.h"

// std::map 'cause QMap isn't usable, it don't work with with the typenames in class
#include <map>
#include <algorithm>
#include <QString>
#include <QStringList>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>

namespace helpers
{

/**
    Class for fast search of path based items.

    @author Rajko Albrecht <ral@alwins-world.de>
*/
template<class C> class cacheEntry
{
public:
    typedef cacheEntry<C> cache_type;
    typedef typename std::map<QString, cache_type> cache_map_type;

protected:
    QString m_key;
    bool m_isValid;
    C m_content;
    cache_map_type m_subMap;

public:
    cacheEntry();
    cacheEntry(const QString &key);
    cacheEntry(const cacheEntry<C> &other);

    virtual ~cacheEntry() = default;

    virtual bool find(QStringList &, QList<C> &)const;
    //! Checks if cache contains a specific item
    /*!
     * the keylist will manipulated - so copy-operations aren't needed.
     * \param what Stringlist containing the components to search for
     * \return true if found (may or may not valid!) otherwise false
     */
    virtual bool find(QStringList &what)const;
    //! Checks if cache contains a specific valid item
    /*!
     * if yes, the content will stored in st
     * \param what the keylist to search for
     * \param st target status to store content if found
     * \return true if found
     */
    virtual bool findSingleValid(QStringList &what, C &st)const;
    //! Checks if cache contains a specific valid item
    /*!
     * in difference to virtual bool find(QStringList&,svn::StatusEntries&)const no copy operations
     * are made inside so it works much faster for simple find.
     * \param what the keylist to search for
     * \param check_valid_subs if true, return true if a subitem is valid even the item isn't valid
     * \return true if found
     */
    virtual bool findSingleValid(QStringList &what, bool check_valid_subs)const;
    template<class T> void listsubs_if(QStringList &_what, T &oper)const;

    virtual void appendValidSub(QList<C> &)const;
    virtual bool isValid()const
    {
        return m_isValid;
    }
    virtual const C &content()const
    {
        return m_content;
    }
    virtual bool deleteKey(QStringList &, bool exact);
    virtual void insertKey(QStringList &, const C &);
    virtual void setValidContent(const QString &key, const C &st)
    {
        m_key = key;
        m_isValid = true;
        m_content = st;
    }
    virtual bool hasValidSubs()const;
    virtual void markInvalid()
    {
        m_content = C();
        m_isValid = false;
    }
    const QString &key()const
    {
        return m_key;
    }

    cacheEntry<C> &operator=(const cacheEntry<C> &other);
#if 0
    void dump_tree(int level = 0)const
    {
        QString pre;
        pre.fill('-', level);
        for (auto it = m_subMap.begin(); it != m_subMap.end(); ++it) {
            std::cout << pre.latin1() << it->first.latin1() << " (" << it->second.m_key.latin1() << ")" << std::endl;
            it->second.dump_tree(level + 1);
        }
    }
#endif
};

typedef cacheEntry<svn::StatusPtr> statusEntry;

template<class C> inline cacheEntry<C>::cacheEntry()
    : m_key(), m_isValid(false), m_content()
{
}

template<class C> inline cacheEntry<C>::cacheEntry(const QString &key)
    : m_key(key), m_isValid(false), m_content()
{
}

template<class C> inline cacheEntry<C>::cacheEntry(const cacheEntry<C> &other)
    : m_key(other.m_key), m_isValid(other.m_isValid),
      m_content(other.m_content), m_subMap(other.m_subMap)
{
}

template<class C> inline cacheEntry<C> &cacheEntry<C>::operator=(const cacheEntry<C> &other)
{
    m_key = other.m_key;
    m_isValid = other.m_isValid;
    m_content = other.m_content;
    m_subMap = other.m_subMap;
    return *this;
}

template<class C> inline  bool cacheEntry<C>::find(QStringList &what, QList<C> &t) const
{
    if (what.empty()) {
        return false;
    }
    const auto it = m_subMap.find(what.at(0));
    if (it == m_subMap.end()) {
        return false;
    }
    if (what.count() == 1) {
        if (it->second.isValid()) {
            t.append(it->second.content());
        }
        it->second.appendValidSub(t);
        return true;
    }
    what.erase(what.begin());
    return it->second.find(what, t);
}

template<class C> inline bool cacheEntry<C>::find(QStringList &what) const
{
    if (what.isEmpty()) {
        return false;
    }
    const auto it = m_subMap.find(what.at(0));
    if (it == m_subMap.end()) {
        return false;
    }
    if (what.count() == 1) {
        return true;
    }
    what.erase(what.begin());
    return it->second.find(what);
}

template<class C> inline bool cacheEntry<C>::findSingleValid(QStringList &what, C &t) const
{
    if (what.isEmpty()) {
        return false;
    }
    const auto it = m_subMap.find(what.at(0));
    if (it == m_subMap.end()) {
        return false;
    }
    if (what.count() == 1) {
        t = it->second.content();
        return it->second.isValid();
    }
    what.erase(what.begin());
    return it->second.findSingleValid(what, t);
}

template<class C> inline bool cacheEntry<C>::findSingleValid(QStringList &what, bool check_valid_subs) const
{
    if (what.isEmpty()) {
        return false;
    }
    const auto it = m_subMap.find(what.at(0));
    if (it == m_subMap.end()) {
        return false;
    }
    if (what.count() == 1) {
        return it->second.isValid() || (check_valid_subs && it->second.hasValidSubs());
    }
    what.erase(what.begin());
    return it->second.findSingleValid(what, check_valid_subs);
}

template<class C> inline void cacheEntry<C>::appendValidSub(QList<C> &t) const
{
    for (const auto &it : m_subMap) {
        if (it.second.isValid()) {
            t.append(it.second.content());
        }
        it.second.appendValidSub(t);
    }
}

template<class C> inline bool cacheEntry<C>::deleteKey(QStringList &what, bool exact)
{
    if (what.isEmpty()) {
        return true;
    }
    const auto it = m_subMap.find(what.at(0));
    if (it == m_subMap.end()) {
        return true;
    }
    bool caller_must_check = false;
    /* first stage - we are the one holding the right key */
    if (what.count() == 1) {
        if (!exact || !it->second.hasValidSubs()) {
            m_subMap.erase(it);
            caller_must_check = true;
        } else {
            it->second.markInvalid();
        }
    } else {
        /* otherwise go trough tree */
        what.erase(what.begin());
        bool b = it->second.deleteKey(what, exact);
        if (b && !it->second.hasValidSubs()) {
            m_subMap.erase(it);
            caller_must_check = true;
        }
    }
    return caller_must_check;
}

template<class C> inline bool cacheEntry<C>::hasValidSubs() const
{
    for (const auto &it : m_subMap) {
        if (it.second.isValid() || it.second.hasValidSubs()) {
            return true;
        }
    }
    return false;
}

template<class C> inline void cacheEntry<C>::insertKey(QStringList &what, const C &st)
{
    if (what.isEmpty()) {
        return;
    }
    const QString m = what.at(0);

    if (m_subMap.find(m) == m_subMap.end()) {
        m_subMap[m].m_key = m;
    }
    if (what.count() == 1) {
        m_subMap[m].setValidContent(m, st);
        return;
    }
    what.erase(what.begin());
    m_subMap[m].insertKey(what, st);
}

template<class C> template<class T> inline void cacheEntry<C>::listsubs_if(QStringList &what, T &oper) const
{
    if (what.isEmpty()) {
        /* we are the one to get the list for*/
        oper = for_each(m_subMap.begin(), m_subMap.end(), oper);
        return;
    }
    /* otherwise find next */
    const auto it = m_subMap.find(what.at(0));
    if (it == m_subMap.end()) {
        /* not found */
        return;
    }
    what.erase(what.begin());
    it->second.listsubs_if(what, oper);
}

template<class C> class itemCache
{
public:
    typedef cacheEntry<C> cache_type;
    typedef typename std::map<QString, cache_type> cache_map_type;

protected:
    cache_map_type m_contentMap;

    mutable QReadWriteLock m_RWLock;

public:
    itemCache() = default;
    virtual ~itemCache() = default;

    void clear()
    {
        QWriteLocker locker(&m_RWLock);
        m_contentMap.clear();
    }
    //! Checks if cache contains a specific item
    /*!
     * the keylist will manipulated - so copy-operations aren't needed.
     * \param what Stringlist containing the components to search for
     * \return true if found (may or may not valid!) otherwise false
     */
    virtual bool find(const QString &what)const;

    virtual void deleteKey(const QString &what, bool exact);
    virtual void insertKey(const C &, const QString &path);
    virtual bool findSingleValid(const QString &what, C &)const;
    virtual bool findSingleValid(const QString &what, bool check_valid_subs)const;

    template<class T>void listsubs_if(const QString &what, T &oper)const;

    void dump_tree();
};


template<class C> inline void itemCache<C>::insertKey(const C &st, const QString &path)
{
    QStringList _keys = path.split(QLatin1Char('/'));
    if (_keys.isEmpty()) {
        return;
    }
    QWriteLocker locker(&m_RWLock);

    const QString m = _keys.at(0);
    const auto it = m_contentMap.find(m);

    if (it == m_contentMap.end()) {
        m_contentMap[m] = cache_type(m);
    }
    if (_keys.count() == 1) {
        m_contentMap[m].setValidContent(m, st);
    } else {
        _keys.erase(_keys.begin());
        m_contentMap[m].insertKey(_keys, st);
    }
}

template<class C> inline bool itemCache<C>::find(const QString &what)const
{
    QReadLocker locker(&m_RWLock);

    if (m_contentMap.empty()) {
        return false;
    }
    QStringList _keys = what.split(QLatin1Char('/'));
    if (_keys.isEmpty()) {
        return false;
    }
    const auto it = m_contentMap.find(_keys.at(0));
    if (it == m_contentMap.end()) {
        return false;
    }
    if (_keys.count() == 1) {
        return true;
    }
    _keys.erase(_keys.begin());
    return it->second.find(_keys);
}

template<class C> inline void itemCache<C>::deleteKey(const QString &_what, bool exact)
{
    QWriteLocker locker(&m_RWLock);

    if (m_contentMap.empty()) {
        return;
    }
    QStringList what = _what.split(QLatin1Char('/'));
    if (what.isEmpty()) {
        return;
    }
    const auto it = m_contentMap.find(what.at(0));
    if (it == m_contentMap.end()) {
        return;
    }
    /* first stage - we are the one holding the right key */
    if (what.count() == 1) {
        if (!exact || !it->second.hasValidSubs()) {
            /* if it has no valid subs delete it */
            m_contentMap.erase(it);
        } else {
            /* otherwise mark as invalid */
            it->second.markInvalid();
        }
        return;
    } else {
        /* otherwise go trough tree */
        what.erase(what.begin());
        bool b = it->second.deleteKey(what, exact);
        if (b && !it->second.hasValidSubs()) {
            m_contentMap.erase(it);
        }
    }
}

template<class C> inline void itemCache<C>::dump_tree()
{
    QReadLocker locker(&m_RWLock);
    for (auto it = m_contentMap.begin(); it != m_contentMap.end(); ++it) {
//        std::cout<<it->first.latin1() << " (" << it->second.key().latin1() << ")"<<std::endl;
//        it->second.dump_tree(1);
    }
}

template<class C> inline bool itemCache<C>::findSingleValid(const QString &_what, C &st)const
{
    QReadLocker locker(&m_RWLock);

    if (m_contentMap.empty()) {
        return false;
    }
    QStringList what = _what.split(QLatin1Char('/'));
    if (what.isEmpty()) {
        return false;
    }
    const auto it = m_contentMap.find(what.at(0));
    if (it == m_contentMap.end()) {
        return false;
    }
    if (what.count() == 1) {
        if (it->second.isValid()) {
            st = it->second.content();
            return true;
        }
        return false;
    }
    what.erase(what.begin());
    return it->second.findSingleValid(what, st);
}

template<class C> inline bool itemCache<C>::findSingleValid(const QString &_what, bool check_valid_subs)const
{
    QReadLocker locker(&m_RWLock);

    if (m_contentMap.empty()) {
        return false;
    }
    QStringList what = _what.split(QLatin1Char('/'));
    if (what.isEmpty()) {
        return false;
    }
    const auto it = m_contentMap.find(what.at(0));
    if (it == m_contentMap.end()) {
        return false;
    }
    if (what.count() == 1) {
        return it->second.isValid() || (check_valid_subs && it->second.hasValidSubs());
    }
    what.erase(what.begin());
    return it->second.findSingleValid(what, check_valid_subs);
}

template<class C> template<class T> inline void itemCache<C>::listsubs_if(const QString &_what, T &oper)const
{
    QReadLocker locker(&m_RWLock);

    if (m_contentMap.empty()) {
        return;
    }
    QStringList what = _what.split(QLatin1Char('/'));
    if (what.isEmpty()) {
        return;
    }
    const auto it = m_contentMap.find(what.at(0));
    if (it == m_contentMap.end()) {
        return;
    }
    if (what.count() == 1) {
        oper = for_each(m_contentMap.begin(), m_contentMap.end(), oper);
        return;
    }
    what.erase(what.begin());
    it->second.listsubs_if(what, oper);
}

typedef cacheEntry<svn::StatusPtr> ptrEntry;
typedef itemCache<svn::StatusPtr> statusCache;

class ValidRemoteOnly
{
    svn::StatusEntries m_List;
public:
    ValidRemoteOnly(): m_List() {}
    void operator()(const std::pair<QString, helpers::ptrEntry> &_data)
    {
        if (_data.second.isValid() && _data.second.content()->validReposStatus() && !_data.second.content()->validLocalStatus()) {
            m_List.push_back(_data.second.content());
        }
    }
    const svn::StatusEntries &liste()const
    {
        return m_List;
    }
};

}

#endif
