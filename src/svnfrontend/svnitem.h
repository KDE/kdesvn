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

#ifndef SVN_ITEM_H
#define SVN_ITEM_H

#include "src/svnqt/status.h"
#include <kmimetype.h>
#include <QString>
#include <QDateTime>
#include <QPixmap>
#include <QScopedPointer>
#include <QList>

class SvnItemModelNode;
class SvnItem_p;
class SvnActions;
class KFileItem;
class KUrl;

namespace svn
{
class Revision;
}

class SvnItem
{
public:
    SvnItem();
    explicit SvnItem(const svn::StatusPtr &);
    virtual ~SvnItem();

    virtual const QString &fullName()const;
    virtual const QString &shortName()const;
    virtual const QString &Url()const;
    virtual const KUrl &kdeName(const svn::Revision &);
    virtual KMimeType::Ptr mimeType();
    virtual const QDateTime &fullDate()const;
    virtual bool isDir()const;
    virtual bool isVersioned()const;
    virtual bool isConflicted()const;
    virtual bool isValid()const;
    virtual bool isRealVersioned()const;
    virtual bool isIgnored()const;
    virtual bool isRemoteAdded()const;
    virtual bool isChanged()const;
    virtual bool isLocalAdded()const;
    virtual QString infoText()const;
    virtual QString cmtAuthor()const;
    virtual long int cmtRev()const;
    virtual bool isLocked()const;
    virtual QString lockOwner()const;
    virtual QString getParentDir()const = 0;
    virtual SvnItem *getParentItem()const = 0;
    virtual svn::Revision correctPeg()const = 0;
    virtual svn::Revision revision()const;
    virtual void refreshStatus(bool children = false) = 0;

    QPixmap getPixmap(int size, bool overlay = true);
    QPixmap getPixmap(const QPixmap &, int size, bool overlay = true);

    virtual SvnItemModelNode *sItem()
    {
        return 0;
    }
    virtual void setStat(const svn::StatusPtr &);
    virtual const svn::StatusPtr &stat()const;
    virtual bool isModified()const;
    virtual bool isChildModified()const;
    bool isNormal()const;
    bool isMissing()const;
    bool isDeleted()const;
    const QString &getToolTipText();
    virtual void generateToolTip(const svn::InfoEntry &entry);
    bool hasToolTipText();
    KFileItem fileItem();

protected:
    bool m_overlaycolor;
    enum color_type {
        NONE = 0,
        UPDATES = 1,
        MODIFIED = 2,
        LOCKED = 3,
        ADDED = 4,
        DELETED = 5,
        MISSING = 6,
        NOTVERSIONED = 7,
        CONFLICT = 8,
        NEEDLOCK = 9
    };
    color_type m_bgColor;
    QScopedPointer<SvnItem_p> p_Item;
    virtual SvnActions *getWrapper()const = 0;

    static QPixmap internalTransform(const QPixmap &, int size);

};

#endif
