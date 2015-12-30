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

#include "svnitem.h"
#include "svnactions.h"
#include "kdesvn_part.h"
#include "src/settings/kdesvnsettings.h"
#include "src/svnqt/status.h"
#include "src/svnqt/url.h"
#include "helpers/ktranslateurl.h"

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kiconeffect.h>
#include <kfileitem.h>

#include <QString>
#include <QFileInfo>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QMutexLocker>

class SvnItem_p
{
public:
    SvnItem_p();
    explicit SvnItem_p(const svn::StatusPtr &);

    KFileItem &createItem(const svn::Revision &peg);
    KUrl &kdeName(const svn::Revision &);
    KMimeType::Ptr mimeType(bool dir);

    svn::StatusPtr m_Stat;
    void init();
    QString m_url, m_full, m_short;
    KUrl m_kdename;
    QDateTime m_fullDate;
    QString m_infoText;
    KFileItem m_fitem;
    bool isWc;
    svn::Revision lRev;
    KMimeType::Ptr mptr;
    QMutex _infoTextMutex;
};

SvnItem_p::SvnItem_p()
    : m_Stat(new svn::Status())
{
    init();
}

SvnItem_p::SvnItem_p(const svn::StatusPtr &aStat)
    : m_Stat(aStat)
{
    init();
}


void SvnItem_p::init()
{
    isWc = false;
    m_full = m_Stat->path();
    m_kdename.clear();
    mptr = 0;
    lRev = svn::Revision::UNDEFINED;
    while (m_full.endsWith(QLatin1Char('/'))) {
        /* dir name possible */
        m_full.chop(1);
    }
    int p = m_full.lastIndexOf(QLatin1Char('/'));
    if (p > -1) {
        ++p;
        m_short = m_full.right(m_full.length() - p);
    } else {
        m_short = m_full;
    }
    m_url = m_Stat->entry().url();
    m_fullDate = svn::DateTime(m_Stat->entry().cmtDate());
    m_infoText.clear();
}

KMimeType::Ptr SvnItem_p::mimeType(bool dir)
{
    if (!mptr || m_kdename.isEmpty()) {
        if (m_kdename.isEmpty()) {
            kdeName(svn::Revision::UNDEFINED);
        }
        if (dir) {
            mptr = KMimeType::mimeType("inode/directory");
        } else {
            mptr = KMimeType::findByUrl(m_kdename, 0, isWc, !isWc);
        }
    }
    return mptr;
}

KUrl &SvnItem_p::kdeName(const svn::Revision &r)
{
    isWc = !svn::Url::isValid(m_Stat->path());
    if (!(r == lRev) || m_kdename.isEmpty()) {
        lRev = r;
        if (!isWc) {
            m_kdename = m_Stat->entry().url();
            QString proto = helpers::KTranslateUrl::makeKdeUrl(m_kdename.protocol());
            m_kdename.setProtocol(proto);
            QString revstr = lRev.toString();
            if (revstr.length() > 0) {
                m_kdename.setQuery("?rev=" + revstr);
            }
        } else {
            m_kdename = KUrl::fromPathOrUrl(m_Stat->path());
        }
    }
    return m_kdename;
}

KFileItem &SvnItem_p::createItem(const svn::Revision &peg)
{
    if (m_fitem.isNull() || !(peg == lRev)) {
        m_fitem = KFileItem(KFileItem::Unknown, KFileItem::Unknown, kdeName(peg));
    }
    return m_fitem;
}

SvnItem::SvnItem()
    : m_overlaycolor(false)
    , m_bgColor(NONE)
    , p_Item(new SvnItem_p())
{
}

SvnItem::SvnItem(const svn::StatusPtr &aStat)
    : m_overlaycolor(false)
    , m_bgColor(NONE)
    , p_Item(new SvnItem_p(aStat))
{
}

SvnItem::~SvnItem()
{
}

void SvnItem::setStat(const svn::StatusPtr &aStat)
{
    m_overlaycolor = false;
    p_Item.reset(new SvnItem_p(aStat));
    SvnActions *wrap = getWrapper();
    if (isChanged() || isConflicted()) {
        wrap->addModifiedCache(aStat);
    } else {
        wrap->deleteFromModifiedCache(fullName());
    }
}

const QString &SvnItem::fullName()const
{
    return (p_Item->m_full);
}

const QString &SvnItem::shortName()const
{
    return (p_Item->m_short);
}

const QString &SvnItem::Url()const
{
    return (p_Item->m_url);
}

bool SvnItem::isDir()const
{
    if (isRemoteAdded() || p_Item->m_Stat->entry().isValid()) {
        return p_Item->m_Stat->entry().kind() == svn_node_dir;
    }
    /* must be a local file */
    QFileInfo f(fullName());
    return f.isDir();
}

const QDateTime &SvnItem::fullDate()const
{
    return (p_Item->m_fullDate);
}

QPixmap SvnItem::internalTransform(const QPixmap &first, int size)
{
    if (first.isNull()) {
        return QPixmap();
    }
    QPixmap _p = first.scaled(QSize(size, size), Qt::KeepAspectRatio);
    if (_p.width() == size && _p.height() == size) {
        return _p;
    }
    QPixmap result(size, size);
    result.fill(Qt::transparent);
    QPainter pa;
    pa.begin(&result);
    int w = _p.width() > size ? size : _p.width();
    int h = _p.height() > size ? size : _p.height();
    pa.drawPixmap(0, 0, _p, 0, 0, w, h);
    pa.end();
    return result;
}

QPixmap SvnItem::getPixmap(const QPixmap &_p, int size, bool overlay)
{
    if (!isVersioned()) {
        m_bgColor = NOTVERSIONED;
    } else if (isRealVersioned()) {
        SvnActions *wrap = getWrapper();
        bool mod = false;
        QPixmap p2 = QPixmap();
        if (p_Item->m_Stat->textStatus() == svn_wc_status_conflicted) {
            m_bgColor = CONFLICT;
            if (overlay) {
                p2 = KIconLoader::global()->loadIcon("kdesvnconflicted", KIconLoader::Desktop, size);
            }
        } else if (p_Item->m_Stat->textStatus() == svn_wc_status_missing) {
            m_bgColor = MISSING;
        } else if (isLocked() || (wrap && wrap->checkReposLockCache(fullName()))) {
            if (overlay) {
                p2 = KIconLoader::global()->loadIcon("kdesvnlocked", KIconLoader::Desktop, size);
            }
            m_bgColor = LOCKED;
        } else if (Kdesvnsettings::check_needslock() && !isRemoteAdded() && wrap && wrap->isLockNeeded(this, svn::Revision::UNDEFINED)) {
            if (overlay) {
                p2 = KIconLoader::global()->loadIcon("kdesvnneedlock", KIconLoader::Desktop, size);
            }
            m_bgColor = NEEDLOCK;
        } else if (wrap && wrap->isUpdated(p_Item->m_Stat->path())) {
            if (overlay) {
                p2 = KIconLoader::global()->loadIcon("kdesvnupdates", KIconLoader::Desktop, size);
            }
            m_bgColor = UPDATES;
        } else if (p_Item->m_Stat->textStatus() == svn_wc_status_deleted) {
            if (overlay) {
                p2 = KIconLoader::global()->loadIcon("kdesvndeleted", KIconLoader::Desktop, size);
            }
            m_bgColor = DELETED;
        } else if (p_Item->m_Stat->textStatus() == svn_wc_status_added) {
            if (overlay) {
                p2 = KIconLoader::global()->loadIcon("kdesvnadded", KIconLoader::Desktop, size);
            }
            m_bgColor = ADDED;
        } else if (isModified()) {
            mod = true;
        } else if (isDir() && wrap) {
            if (isRemoteAdded() || wrap->checkUpdateCache(fullName())) {
                if (overlay) {
                    p2 = KIconLoader::global()->loadIcon("kdesvnupdates", KIconLoader::Desktop, size);
                }
                m_bgColor = UPDATES;
            } else if (wrap->checkConflictedCache(fullName())) {
                m_bgColor = CONFLICT;
                if (overlay) {
                    p2 = KIconLoader::global()->loadIcon("kdesvnconflicted", KIconLoader::Desktop, size);
                }
            } else {
                mod = wrap->checkModifiedCache(fullName());
            }
        }
        if (mod) {
            m_bgColor = MODIFIED;
            if (overlay) {
                p2 = KIconLoader::global()->loadIcon("kdesvnmodified", KIconLoader::Desktop, size);
            }
        }
        if (!p2.isNull()) {
            QPixmap p;
            if (_p.width() != size || _p.height() != size) {
                p = internalTransform(_p, size);
            } else {
                p = _p;
            }
            if (p2.width() != size || p2.height() != size) {
                p2 = internalTransform(p2, size);
            }
            m_overlaycolor = true;
            QImage i1(p.toImage());
            QImage i2(p2.toImage());

            KIconEffect::overlay(i1, i2);
            return QPixmap::fromImage(i1);
        }
    }
    return _p;
}

QPixmap SvnItem::getPixmap(int size, bool overlay)
{
    QPixmap p;
    m_overlaycolor = false;
    m_bgColor = NONE;
    /* yes - different way to "isDir" above 'cause here we try to use the
       mime-features of KDE on ALL not just unversioned entries.
     */
#ifdef DEBUG_TIMER
    QTime _counttime;
    _counttime.start();
#endif
    if (svn::Url::isValid(p_Item->m_Stat->path())) {
        /* remote access */
        p = KIconLoader::global()->loadMimeTypeIcon(p_Item->mimeType(isDir())->iconName(),
                KIconLoader::Desktop,
                size,
                KIconLoader::DefaultState);
        if (isLocked()) {
            m_bgColor = LOCKED;
            if (overlay) {
                QPixmap p2 = KIconLoader::global()->loadIcon("kdesvnlocked", KIconLoader::Desktop, size);
                if (!p2.isNull()) {
                    QImage i1; i1 = p.toImage();
                    QImage i2; i2 = p2.toImage();
                    KIconEffect::overlay(i1, i2);
                    p.fromImage(i1);
                }
            }
        }
    } else {
        if (isRemoteAdded()) {
            if (isDir()) {
                p = KIconLoader::global()->loadIcon("folder", KIconLoader::Desktop, size);
            } else {
                p = KIconLoader::global()->loadIcon("unknown", KIconLoader::Desktop, size);
            }
        } else {
            KUrl uri;
            uri.setPath(fullName());
            p = KIconLoader::global()->loadMimeTypeIcon(KMimeType::iconNameForUrl(uri), KIconLoader::Desktop, size);
            p = getPixmap(p, size, overlay);
        }
    }
#ifdef DEBUG_TIMER
    //kDebug()<<"Time getting icon: "<<_counttime.elapsed();
#endif
    return p;
}

bool SvnItem::isVersioned()const
{
    return p_Item->m_Stat->isVersioned();
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
    return p_Item->m_Stat->isRealVersioned();
}

bool SvnItem::isIgnored()const
{
    return p_Item->m_Stat->textStatus() == svn_wc_status_ignored;
}

bool SvnItem::isRemoteAdded()const
{
    return getWrapper()->isUpdated(p_Item->m_Stat->path()) &&
           p_Item->m_Stat->validReposStatus() && !p_Item->m_Stat->validLocalStatus();
}

bool SvnItem::isLocalAdded()const
{
    return p_Item->m_Stat->textStatus() == svn_wc_status_added;
}

QString SvnItem::infoText()const
{
    QString info_text;
    if (!isVersioned()) {
        info_text = i18n("Not versioned");
    } else if (getWrapper()->isUpdated(p_Item->m_Stat->path())) {
        if (p_Item->m_Stat->validReposStatus() && !p_Item->m_Stat->validLocalStatus()) {
            info_text = i18n("Added in repository");
        } else {
            info_text = i18n("Needs update");
        }
    } else {
        switch (p_Item->m_Stat->textStatus()) {
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
            info_text = i18n("External");
            break;
        case svn_wc_status_conflicted:
            info_text = i18n("Conflict");
            break;
        case svn_wc_status_merged:
            info_text = i18n("Merged");
            break;
        case svn_wc_status_incomplete:
            info_text = i18n("Incomplete");
            break;
        default:
            break;
        }
        if (info_text.isEmpty()) {
            switch (p_Item->m_Stat->propStatus()) {
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
    return p_Item->m_Stat->entry().cmtAuthor();
}

long int SvnItem::cmtRev()const
{
    return p_Item->m_Stat->entry().cmtRev();
}

bool SvnItem::isLocked()const
{
    return p_Item->m_Stat->entry().lockEntry().Locked();
}

QString SvnItem::lockOwner()const
{
    if (p_Item->m_Stat->entry().lockEntry().Locked()) {
        return p_Item->m_Stat->entry().lockEntry().Owner();
    }
    svn::StatusPtr tmp;
    if (getWrapper()->checkReposLockCache(fullName(), tmp) && tmp) {
        return tmp->lockEntry().Owner();
    }
    return QString();
}

/*!
    \fn SvnItem::isModified()
 */
bool SvnItem::isModified()const
{
    return p_Item->m_Stat->textStatus() == svn_wc_status_modified || p_Item->m_Stat->propStatus() == svn_wc_status_modified
           || p_Item->m_Stat->textStatus() == svn_wc_status_replaced;
}

bool SvnItem::isChanged()const
{
    return isRealVersioned() && (isModified() || isDeleted() || isLocalAdded());
}

bool SvnItem::isChildModified()const
{
    return getWrapper()->checkModifiedCache(fullName());
}

const svn::StatusPtr &SvnItem::stat()const
{
    return p_Item->m_Stat;
}

/*!
    \fn SvnItem::isNormal()const
 */
bool SvnItem::isNormal()const
{
    return p_Item->m_Stat->textStatus() == svn_wc_status_normal;
}

bool SvnItem::isMissing()const
{
    return p_Item->m_Stat->textStatus() == svn_wc_status_missing;
}

bool SvnItem::isDeleted()const
{
    return p_Item->m_Stat->textStatus() == svn_wc_status_deleted;
}

bool SvnItem::isConflicted()const
{
    return p_Item->m_Stat->textStatus() == svn_wc_status_conflicted;
}

bool SvnItem::hasToolTipText()
{
    QMutexLocker ml(&(p_Item->_infoTextMutex));
    return !p_Item->m_infoText.isNull();
}

svn::Revision SvnItem::revision() const
{
    if (isRealVersioned() && !p_Item->m_Stat->entry().url().isEmpty()) {
        return p_Item->m_Stat->entry().revision();
    }
    return svn::Revision::UNDEFINED;
}

/*!
    \fn SvnItem::getToolTipText()
 */
const QString &SvnItem::getToolTipText()
{
    if (!hasToolTipText()) {
        kDebug() << "Try getting text" << endl;
        QString text;
        if (isRealVersioned() && !p_Item->m_Stat->entry().url().isEmpty()) {
            SvnActions *wrap = getWrapper();
            svn::Revision peg(svn_opt_revision_unspecified);
            svn::Revision rev(svn_opt_revision_unspecified);
            if (svn::Url::isValid(p_Item->m_Stat->path())) {
                /* remote */
                rev = p_Item->m_Stat->entry().revision();
                peg = correctPeg();
            } else {
                /* local */
            }
            if (wrap) {
                SvnItemList lst;
                lst.append(this);
                text = wrap->getInfo(lst, rev, peg, false, false);
                kDebug() << text << endl;
                if (!p_Item->m_fitem.isNull()) {
                    text += p_Item->m_fitem.getToolTipText(0);
                }
            }
        } else if (!p_Item->m_fitem.isNull()) {
            text = p_Item->m_fitem.getToolTipText(6);
        }
        QMutexLocker ml(&(p_Item->_infoTextMutex));
        p_Item->m_infoText = text;
    }
    QMutexLocker ml(&(p_Item->_infoTextMutex));
    return p_Item->m_infoText;
}

void SvnItem::generateToolTip(const svn::InfoEntry &entry)
{
    QString text;
    if (isRealVersioned() &&  !p_Item->m_Stat->entry().url().isEmpty()) {
        SvnActions *wrap = getWrapper();
        if (wrap) {
            svn::InfoEntries e; e.append(entry);
            text = wrap->getInfo(e, fullName(), false);
        }
        if (!p_Item->m_fitem.isNull()) {
            text += p_Item->m_fitem.getToolTipText(0);
        }
    } else if (!p_Item->m_fitem.isNull()) {
        text = p_Item->m_fitem.getToolTipText(6);
    }
    {
        QMutexLocker ml(&(p_Item->_infoTextMutex));
        p_Item->m_infoText = text;
    }
}

KFileItem SvnItem::fileItem()
{
    return p_Item->createItem(correctPeg());
}

const KUrl &SvnItem::kdeName(const svn::Revision &r)
{
    return p_Item->kdeName(r);
}

KMimeType::Ptr SvnItem::mimeType()
{
    return p_Item->mimeType(isDir());
}
