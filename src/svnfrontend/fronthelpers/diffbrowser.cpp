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

#include "diffbrowser.h"
#include "diffsyntax.h"

#include <kglobalsettings.h>
#include <kglobal.h>

#include <qfont.h>
/*!
    \fn DiffBrowser::DiffBrowser(QWidget*parent=0,const char*name=0)
 */
DiffBrowser::DiffBrowser(QWidget*parent,const char*name)
    : QTextBrowser( parent, name)
{
    setTextFormat(Qt::PlainText);
    setFont(KGlobalSettings::fixedFont());
    //setTabStopWidth(4);
    setWordWrap(QTextEdit::NoWrap);
    m_Syntax = new DiffSyntax(this);
}

/*!
    \fn DiffBrowser::~DiffBrowser()
 */
 DiffBrowser::~DiffBrowser()
{
    delete m_Syntax;
}

void DiffBrowser::setText(const QString&aText)
{
#if 0
    if (aText.find("\t")!=-1) {
        QString b=aText;
        b.replace("\t","    ");
        QTextBrowser::setText(b);
    } else {
#endif
        QTextBrowser::setText(aText);
#if 0
    }
#endif
}

#include "diffbrowser.h.moc"
