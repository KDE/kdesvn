/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht                                  *
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
#ifndef CCONTEXTLISTENER_H
#define CCONTEXTLISTENER_H

#include "svncpp/context_listener.hpp"
#include "helpers/smart_pointer.h"

#include <qobject.h>
#include <qstring.h>

/**
@author Rajko Albrecht
*/
class CContextListener : public QObject, public svn::ContextListener,public ref_count
{
    Q_OBJECT
public:
    CContextListener(QObject *parent = 0, const char *name = 0);
    ~CContextListener();

    /* context-listener methods */
    virtual bool contextGetLogin (const QString & realm,
                     QString & username,
                     QString & password,
                     bool & maySave);
    virtual void contextNotify (const char *path,
                   svn_wc_notify_action_t action,
                   svn_node_kind_t kind,
                   const char *mime_type,
                   svn_wc_notify_state_t content_state,
                   svn_wc_notify_state_t prop_state,
                   svn_revnum_t revision);

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
    virtual void contextNotify (const svn_wc_notify_t *action);
#endif

    virtual bool contextCancel();
    /*!
     * Get logmessage for checkin and so on...
     */
    virtual bool contextGetLogMessage (QString & msg);
    virtual SslServerTrustAnswer contextSslServerTrustPrompt (const SslServerTrustData & data,
                                 apr_uint32_t & acceptedFailures);
    virtual bool contextSslClientCertPrompt (QString & certFile);
    virtual bool contextSslClientCertPwPrompt (QString & password,
                                   const QString & realm, bool & maySave);
    static QString NotifyAction(svn_wc_notify_action_t action);
    static QString NotifyState(svn_wc_notify_state_t);

    void setCancelled(bool how){m_cancelMe = how;}
signals:
    void sendNotify(const QString&);
    void tickProgress();
    void waitShow(bool);

protected:
    static const int smax_actionstring;
    static const QString action_strings[];
    static const QString notify_state_strings[];
    bool m_cancelMe;
};

#endif
