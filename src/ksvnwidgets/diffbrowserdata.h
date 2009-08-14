/***************************************************************************
 *   Copyright (C) 2007 by Rajko Albrecht                                  *
 *   ral@alwins-world.de                                                   *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this program (in the file LGPL.txt); if not,         *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at http://kdesvn.alwins-world.de.           *
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

