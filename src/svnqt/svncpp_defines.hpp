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

#ifndef _SVNCPP_DEFINES_H
#define _SVNCPP_DEFINES_H

// config
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __KDE_HAVE_GCC_VISIBILITY_DISABLED_MACRO
#define SVNQT_EXPORT __attribute__ ((visibility("visible")))
#define SVNQT_NOEXPORT __attribute__ ((visibility("hidden")))
#else
#define SVNQT_EXPORT
#define SVNQT_NOEXPORT
#endif
#endif

// qt
#include <qglobal.h>

#if QT_VERSION < 0x040000
#define TOUTF8 local8Bit
#define FROMUTF8 fromLocal8Bit
#else
#define TOUTF8 toUtf8
#define FROMUTF8 fromUtf8
#endif
