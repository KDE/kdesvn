/***************************************************************************
 *   Copyright (C) 2007 by Rajko Albrecht                                  *
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
#ifndef DIFFBROWSERDATA_H
#define DIFFBROWSERDATA_H

#include "diffsyntax.h"

// #include <keditcl.h>
#include <KTextEditor/Editor>
#include <KFindDialog>

/* FIXME TODO fix the search */
#include <qstring.h>

class DiffBrowserData
{
public:
    DiffBrowserData();
    virtual ~DiffBrowserData();

//     enum {NONE, FORWARD, BACKWARD};

    DiffSyntax*m_Syntax;
    QByteArray m_content;
    KFindDialog *srchdialog;

    int last_search,last_finished_search;
    QString pattern;
};

#endif

