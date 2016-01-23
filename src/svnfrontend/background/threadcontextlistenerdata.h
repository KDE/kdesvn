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
#ifndef THREADCONTEXTLISTENERDATA_H
#define THREADCONTEXTLISTENERDATA_H

#include "svnqt/context_listener.h"

#include <QString>
#include <QMutex>

/**
@author Rajko Albrecht
*/
struct ThreadContextListenerData
{
    ThreadContextListenerData()
        : noProgress(true)
        , bReturnValue(false)
    {}

    /* sometimes suppress progress messages */
    bool noProgress;

    static QMutex *callbackMutex(); // only one callback at a time

    /* safed due condition above */
    /* this variables are for the event handling across threads */
    bool bReturnValue;
    struct strust_answer {
        svn::ContextListener::SslServerTrustAnswer sslTrustAnswer;
        svn::ContextListener::SslServerTrustData trustdata;
        strust_answer()
            : sslTrustAnswer(svn::ContextListener::DONT_ACCEPT)
        {}
    } m_strust_answer;

    /* login into server */
    struct slogin_data {
        QString user, password, realm;
        bool maysave;
        slogin_data()
            : maysave(false)
        {}
    } m_slogin_data;

    struct slog_message {
        QString msg;
        svn::CommitItemList items;
    } m_slog_message;

    struct scert_pw {
        QString password, realm;
        bool maysave;
        scert_pw()
            : maysave(false)
        {}
    } m_scert_pw;

    struct scert_file {
        QString certfile;
    } m_scert_file;
};

#endif
