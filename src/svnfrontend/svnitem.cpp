/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht   *
 *   ral@alwins-world.de   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "svnitem.h"
#include "helpers/smart_pointer.h"
#include "svncpp/status.hpp"
#include "helpers/stl2qt.h"
#include "helpers/sub2qt.h"
#include "kdesvn_part.h"
#include "svnactions.h"

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmimetype.h>
#include <kdebug.h>
#include <kiconeffect.h>

#include <qstring.h>
#include <qfileinfo.h>
#include <qimage.h>

class SvnItem_p:public ref_count
{
    friend class SvnItem;
public:
    SvnItem_p();
    SvnItem_p(const svn::Status&);
    virtual ~SvnItem_p();
protected:
    svn::Status m_Stat;
    void init();
    QString m_url,m_full,m_short;
    QDateTime m_fullDate;
};

SvnItem_p::SvnItem_p()
    :ref_count(),m_Stat()
{
    init();
}

SvnItem_p::SvnItem_p(const svn::Status&aStat)
    :ref_count(),m_Stat(aStat)
{
    init();
}

SvnItem_p::~SvnItem_p()
{
}

void SvnItem_p::init()
{
    m_full = m_Stat.path();
    while (m_full.endsWith("/")) {
        /* dir name possible */
        m_full.truncate(m_full.length()-1);
    }
    int p = m_full.findRev("/");
    if (p>-1) {
        ++p;
        m_short = m_full.right(m_full.length()-p);
    } else {
        m_short = m_full;
    }
    m_url = m_Stat.entry().url();
    m_fullDate = helpers::sub2qt::apr_time2qt(m_Stat.entry().cmtDate());
}

SvnItem::SvnItem()
    : p_Item(new SvnItem_p())
{
}

SvnItem::SvnItem(const svn::Status&aStat)
    : p_Item(new SvnItem_p(aStat))
{
}

SvnItem::~SvnItem()
{
}

void SvnItem::setStat(const svn::Status&aStat)
{
    p_Item = new SvnItem_p(aStat);
}

const QString&SvnItem::fullName()const
{
    return (p_Item->m_full);
}

const QString&SvnItem::shortName()const
{
    return (p_Item->m_short);
}

const QString&SvnItem::Url()const
{
    return (p_Item->m_url);
}

bool SvnItem::isDir()const
{
    if (p_Item->m_Stat.entry().isValid()) {
        return p_Item->m_Stat.entry().kind()==svn_node_dir;
    }
    /* must be a local file */
    QFileInfo f(fullName());
    return f.isDir();
}

const QDateTime&SvnItem::fullDate()const
{
    return (p_Item->m_fullDate);
}


QPixmap SvnItem::getPixmap(int size,bool overlay)
{
    QPixmap p;
    bool _local = false;
    /* yes - different way to "isDir" above 'cause here we try to use the
       mime-features of KDE on ALL not just unversioned entries.
     */
    if (QString::compare(p_Item->m_Stat.entry().url(),p_Item->m_Stat.path())==0) {
        /* remote access */
        if (isDir()) {
            p = kdesvnPart::iconLoader()->loadIcon("folder",KIcon::Desktop,size);
        } else {
            p = kdesvnPart::iconLoader()->loadIcon("unknown",KIcon::Desktop,size);
        }
    } else {
        _local = true;
        p = KMimeType::pixmapForURL(fullName(),0,KIcon::Desktop,size);
    }
    if (overlay && _local && isRealVersioned()) {
        SvnActions*wrap = getWrapper();
        bool mod = false;
        QPixmap p2 = QPixmap();
        if (wrap->isUpdated(p_Item->m_Stat.path())) {
            p2 = kdesvnPart::iconLoader()->loadIcon("svnupdates",KIcon::Desktop,size);
        } else if (p_Item->m_Stat.textStatus()==svn_wc_status_deleted) {
            p2 = kdesvnPart::iconLoader()->loadIcon("svndeleted",KIcon::Desktop,size);
        } else if (p_Item->m_Stat.textStatus()==svn_wc_status_added ) {
            p2 = kdesvnPart::iconLoader()->loadIcon("svnadded",KIcon::Desktop,size);
        } else if (p_Item->m_Stat.textStatus ()==svn_wc_status_modified||p_Item->m_Stat.propStatus()==svn_wc_status_modified) {
            mod = true;
        } else if (isDir()&&wrap) {
            svn::StatusEntries dlist;
            svn::StatusEntries::const_iterator it;
            wrap->checkUpdateCache(fullName(),dlist);
            if (dlist.size()>0) {
                p2 = kdesvnPart::iconLoader()->loadIcon("svnupdates",KIcon::Desktop,size);
            } else {
                wrap->checkModifiedCache(fullName(),dlist);
                for (it=dlist.begin();it!=dlist.end();++it) {
                    if ( (*it).textStatus()==svn_wc_status_modified||
                        (*it).textStatus()==svn_wc_status_added||
                        (*it).textStatus()==svn_wc_status_deleted||
                        (*it).textStatus()==svn_wc_status_conflicted ||
                        (*it).propStatus()==svn_wc_status_modified) {
                        mod = true;
                        break;
                    }
                }
            }
        }
        if (mod) {
            p2 = kdesvnPart::iconLoader()->loadIcon("exclam",KIcon::Desktop,size);
        }
        if (!p2.isNull()) {
            QImage i1; i1 = p;
            QImage i2; i2 = p2;
            KIconEffect::overlay(i1,i2);
            p = i1;
        }
    }
    return p;
}

bool SvnItem::isVersioned()const
{
    return p_Item->m_Stat.isVersioned();
}

bool SvnItem::isValid()const
{
    if (isVersioned()) {
        return true;
    }
    QFileInfo f(fullName());
    return f.exists();
}

bool SvnItem::isRealVersioned()const
{
    return p_Item->m_Stat.isRealVersioned();
}

bool SvnItem::isIgnored()const
{
    return p_Item->m_Stat.textStatus()==svn_wc_status_ignored;
}

QString SvnItem::infoText()const
{
    QString info_text = "";
    if (getWrapper()->isUpdated(p_Item->m_Stat.path())) {
        info_text = i18n("Needs update");
    } else {
    switch(p_Item->m_Stat.textStatus ()) {
    case svn_wc_status_modified:
        info_text = i18n("Locally modified");
        break;
    case svn_wc_status_added:
        info_text = i18n("Locally added");
        break;
    case svn_wc_status_missing:
        info_text = i18n("Missing");
        break;
    case svn_wc_status_deleted:
        info_text = i18n("Deleted");
        break;
    case svn_wc_status_replaced:
        info_text = i18n("Replaced");
        break;
    case svn_wc_status_ignored:
        info_text = i18n("Ignored");
        break;
    case svn_wc_status_external:
        info_text=i18n("External");
        break;
    case svn_wc_status_conflicted:
        info_text=i18n("Conflict");
        break;
    case svn_wc_status_merged:
        info_text=i18n("Merged");
        break;
    case svn_wc_status_incomplete:
        info_text=i18n("Incomplete");
        break;
    default:
        break;
    }
    if (info_text.isEmpty()) {
        switch (p_Item->m_Stat.propStatus ()) {
        case svn_wc_status_modified:
            info_text = i18n("Property modified");
            break;
        default:
            break;
        }
    }
    }
    return info_text;
}

QString SvnItem::cmtAuthor()const
{
    return p_Item->m_Stat.entry().cmtAuthor();
}

long int SvnItem::cmtRev()const
{
    return p_Item->m_Stat.entry().cmtRev();
}

bool SvnItem::isLocked()const
{
    return p_Item->m_Stat.entry().lockEntry().Locked();
}

QString SvnItem::lockOwner()const
{
    return p_Item->m_Stat.entry().lockEntry().Owner();
}
