/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   http://kdesvn.alwins-world.de/                                        *
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
#ifndef TESTLISTENER_H
#define TESTLISTENER_H

#include "src/svnqt/context_listener.hpp"

class TestListener:public svn::ContextListener
{
    public:
        TestListener(){}
        virtual ~TestListener(){}

        virtual void contextProgress(long long int current, long long int max){};
        virtual bool contextSslClientCertPwPrompt (QString &,const QString &, bool &){return false;}
        virtual bool contextLoadSslClientCertPw(QString&,const QString&){return false;}
        virtual bool contextSslClientCertPrompt (QString &){return false;}
        virtual svn::ContextListener::SslServerTrustAnswer
                contextSslServerTrustPrompt (const SslServerTrustData &,
                                             apr_uint32_t & ){return svn::ContextListener::SslServerTrustAnswer();}
        virtual bool contextGetLogMessage (QString &,const svn::CommitItemList&){return false;}
        virtual bool contextCancel(){return false;}
        virtual void contextNotify (const svn_wc_notify_t *){}
        virtual void contextNotify (const char *,svn_wc_notify_action_t,
                                    svn_node_kind_t,
                                    const char *,
                                    svn_wc_notify_state_t,
                                    svn_wc_notify_state_t,
                                    svn_revnum_t){}
        virtual bool contextGetSavedLogin (const QString & realm,QString & username,QString & password){return false;}
        virtual bool contextGetCachedLogin (const QString & realm,QString & username,QString & password){return false;} 
        virtual bool contextGetLogin (const QString & realm,
                                      QString & username,
                                      QString & password,
                                      bool & maySave){maySave=false;return false;}

};

#endif
