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
#ifndef THREADCONTEXTLISTENERDATA_H
#define THREADCONTEXTLISTENERDATA_H

#include "svncpp/context_listener.hpp"

#include <qthread.h>
#include <qstring.h>

/**
@author Rajko Albrecht
*/
class ThreadContextListenerData{
public:
    ThreadContextListenerData();

    virtual ~ThreadContextListenerData();

    /* only one callback at time */
    QMutex m_CallbackMutex;
    QWaitCondition m_trustpromptWait;

    /* safed due condition above */
    /* this variables are for the event handling across threads */
    /* Trust ssl realm* */
    struct strust_answer {
        svn::ContextListener::SslServerTrustAnswer m_SslTrustAnswer;
        const svn::ContextListener::SslServerTrustData*m_Trustdata;
    };


    /* login into server */
    struct slogin_data
    {
        QString user,password,realm;
        bool maysave,ok;
    };

    struct slog_message
    {
        QString msg;
        bool ok;
    };

    struct scert_pw
    {
        QString password,realm;
        bool ok,maysave;
    };

    struct scert_file
    {
        QString certfile;
        bool ok;
    };

    struct snotify
    {
        QString msg;
    };
};

#endif
