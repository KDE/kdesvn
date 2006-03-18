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
#ifndef KIOLISTENER_H
#define KIOLISTENER_H

#include "svnqt/context_listener.hpp"

namespace KIO {
    class SlaveBase;
}

class kio_svnProtocol;

/**
@author Rajko Albrecht
*/
class KioListener : public svn::ContextListener
{
public:
    KioListener(kio_svnProtocol*_par);
    virtual ~KioListener();

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
    virtual void contextNotify (const svn_wc_notify_t *action);

    virtual bool contextCancel();
    virtual bool contextGetLogMessage (QString & msg,const svn::CommitItemList&);
    virtual SslServerTrustAnswer contextSslServerTrustPrompt (const SslServerTrustData & data,
                                 apr_uint32_t & acceptedFailures);
    virtual bool contextSslClientCertPrompt (QString & certFile);
    virtual bool contextSslClientCertPwPrompt (QString & password,
                                   const QString & realm, bool & maySave);
    /* context listener virtuals end */
    unsigned int counter()const{return m_notifyCounter;}
    void incCounter(){++m_notifyCounter;}
private:
    kio_svnProtocol *par;
protected:
    unsigned int  m_notifyCounter;
    bool m_External;
    bool m_HasChanges;
    bool m_FirstTxDelta;
};

#endif
