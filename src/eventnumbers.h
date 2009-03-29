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
#ifndef EVENTNUMBERS_H
#define EVENTNUMBERS_H

#include <QEvent>

#define EVENT_CACHE_THREAD_FINISHED QEvent::Type(QEvent::User)
#define EVENT_THREAD_SSL_TRUST_PROMPT QEvent::Type(QEvent::User+1)
#define EVENT_THREAD_LOGIN_PROMPT QEvent::Type(QEvent::User+2)
#define EVENT_THREAD_LOGMSG_PROMPT QEvent::Type(QEvent::User+3)
#define EVENT_THREAD_CERT_PW_PROMPT QEvent::Type(QEvent::User+4)
#define EVENT_THREAD_CERT_SELECT_PROMPT QEvent::Type(QEvent::User+5)
#define EVENT_THREAD_NOTIFY QEvent::Type(QEvent::User+6)
#define EVENT_LOGCACHE_FINISHED QEvent::Type(QEvent::User+7)
#define EVENT_LOGCACHE_STATUS QEvent::Type(QEvent::User+8)
#define EVENT_THREAD_LOGIN_SAVED QEvent::Type(QEvent::User+9)
#define EVENT_UPDATE_CACHE_FINISHED QEvent::Type(QEvent::User+10)

#endif
