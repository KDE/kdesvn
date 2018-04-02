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
#ifndef KIOLISTENER_H
#define KIOLISTENER_H

#include "svnqt/context_listener.h"
#include "ksvnwidgets/pwstorage.h"

namespace KIO
{
class SlaveBase;
class kio_svnProtocol;

/**
@author Rajko Albrecht
*/
class KioListener : public svn::ContextListener
{
public:
    explicit KioListener(KIO::kio_svnProtocol *_par);
    virtual ~KioListener();

    /* context-listener methods */
    bool contextGetLogin(const QString &realm,
                                 QString &username,
                                 QString &password,
                                 bool &maySave) override;
    bool contextGetSavedLogin(const QString &realm, QString &username, QString &password) override;
    bool contextGetCachedLogin(const QString &realm, QString &username, QString &password) override;

    void contextNotify(const char *path,
                               svn_wc_notify_action_t action,
                               svn_node_kind_t kind,
                               const char *mime_type,
                               svn_wc_notify_state_t content_state,
                               svn_wc_notify_state_t prop_state,
                               svn_revnum_t revision) override;
    void contextNotify(const svn_wc_notify_t *action) override;

    bool contextCancel() override;
    bool contextGetLogMessage(QString &msg, const svn::CommitItemList &) override;
    SslServerTrustAnswer contextSslServerTrustPrompt(const SslServerTrustData &data,
            apr_uint32_t &acceptedFailures) override;
    bool contextSslClientCertPrompt(QString &certFile) override;
    bool contextSslClientCertPwPrompt(QString &password,
            const QString &realm, bool &maySave) override;
    bool contextLoadSslClientCertPw(QString &password, const QString &realm) override;
    /* context listener virtuals end */
    unsigned int counter()const
    {
        return m_notifyCounter;
    }
    void incCounter()
    {
        ++m_notifyCounter;
    }
    void contextProgress(long long int current, long long int max) override;

    void setCancel(bool value)
    {
        m_Canceld = value;
    }

    /** Callback for generating list entries
     * This implementation sends items to the protocol, @a entries will ignored.
     * @param entries default target list - ignored
     * @param dirent entry to add (send by subversion)
     * @param lock accociated lock (may be null!)
     * @param path the path of the item
     * @return true if inserted/displayd, false if dirent or entries aren't valid.
     */
    bool contextAddListItem(svn::DirEntries *entries, const svn_dirent_t *dirent, const svn_lock_t *lock, const QString &path) override;

    void uncancel()
    {
        m_Canceld = false;
    }

private:
    KIO::kio_svnProtocol *par;

protected:
    unsigned int  m_notifyCounter;
    bool m_External;
    bool m_HasChanges;
    bool m_FirstTxDelta;
    bool m_Canceld;
};
}

#endif
