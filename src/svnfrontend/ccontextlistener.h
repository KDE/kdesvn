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
#ifndef CCONTEXTLISTENER_H
#define CCONTEXTLISTENER_H

#include "svncpp/context_listener.hpp"

#include <qobject.h>
#include <qstring.h>

/**
@author Rajko Albrecht
*/
class CContextListener : public QObject, public svn::ContextListener
{
    Q_OBJECT
public:
    CContextListener(QObject *parent = 0, const char *name = 0);
    ~CContextListener();

    /* context-listener methods */
    virtual bool contextGetLogin (const std::string & realm,
                     std::string & username,
                     std::string & password,
                     bool & maySave);
    virtual void contextNotify (const char *path,
                   svn_wc_notify_action_t action,
                   svn_node_kind_t kind,
                   const char *mime_type,
                   svn_wc_notify_state_t content_state,
                   svn_wc_notify_state_t prop_state,
                   svn_revnum_t revision);

    virtual bool contextCancel();
    /*!
     * Get logmessage for checkin and so on...
     */
    virtual bool contextGetLogMessage (std::string & msg);
    virtual SslServerTrustAnswer contextSslServerTrustPrompt (const SslServerTrustData & data,
                                 apr_uint32_t & acceptedFailures);
    virtual bool contextSslClientCertPrompt (std::string & certFile);
    virtual bool contextSslClientCertPwPrompt (std::string & password,
                                   const std::string & realm, bool & maySave);
    static QString NotifyAction(svn_wc_notify_action_t action);

    void setCancelled(bool how){m_cancelMe = how;}
signals:
    void sendNotify(const QString&);
    void tickProgress();

protected:
    static const int smax_actionstring;
    static const QString action_strings[];
    bool m_cancelMe;
};

#endif
