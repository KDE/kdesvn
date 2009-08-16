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

#include "diffsyntax.h"
#include <kglobalsettings.h>
#include <kglobal.h>
#include <kdebug.h>

#include <qregexp.h>

/*!
    \fn DiffSyntax::DiffSyntax(QTextEdit*)
 */
 DiffSyntax::DiffSyntax(QTextEdit*aTextEdit)
    : QSyntaxHighlighter(aTextEdit)
{
}


/*!
    \fn DiffSyntax::highlightParagraph ( const QString & text, int endStateOfLastPara )
 */
// int DiffSyntax::highlightParagraph ( const QString & aText, int endStateOfLastPara)
// {
//     static QRegExp a("^\\w+:\\s.*$");
//     static QRegExp b("^\\W+$");
//     QColor c(0,0,0);
//     QFont f(KGlobalSettings::fixedFont());
//     int ret = 0;
//     if (endStateOfLastPara == 1) {
//         ret = 2;
//     } else if (endStateOfLastPara == 2) {
//         if (b.indexIn(aText)!=0) {
//             ret = 2;
//         }
//     }
// 
//     if (a.indexIn(aText)>-1) {
//         c = QColor("#660033");
//         if (endStateOfLastPara==1||endStateOfLastPara==2) {
//             f.setBold(true);
//         } else {
//             f.setItalic(true);
//         }
//     } else if (aText.startsWith("_____" )) {
//         ret = 1;
//         c = QColor("#1D1D8F");
//     } else if (aText.startsWith("+")) {
//         c = QColor("#008B00");
//         if (aText.startsWith("+++")) {
//             f.setBold(true);
//         }
//     } else if (aText.startsWith("-")) {
//         c = QColor("#CD3333");
//         if (aText.startsWith("---")) {
//             f.setBold(true);
//         }
//     } else if (aText.startsWith("@@")) {
//         c = QColor("#1D1D8F");
//     }
//     if (endStateOfLastPara==2 && ret==2) {
//         if (aText.startsWith("   +")) {
//             c = QColor("#008B00");
//         } else if (aText.startsWith("   -")) {
//             c = QColor("#CD3333");
//         }
//     }
//     setFormat(0,(int)aText.length(),f,c);
//     return ret;
// }

void DiffSyntax::highlightBlock(const QString &aText)
{
    static QRegExp a("^\\w+:\\s.*$");
    static QRegExp b("^\\W+$");
    QTextCharFormat format;
//     QColor c(0,0,0);
//     QFont f(KGlobalSettings::fixedFont());
    format.setFont(KGlobalSettings::fixedFont());

    if (previousBlockState() == 1) {
        setCurrentBlockState(2);
    } else if (previousBlockState() == 2) {
        if (b.indexIn(aText)!=0) {
            setCurrentBlockState(2);
        }
    }

    if (a.indexIn(aText)>-1) {
        format.setForeground(QColor("#660033"));
        if (previousBlockState()==1 || previousBlockState()==2) {
            format.setFontWeight(QFont::Bold);
        } else {
            format.setFontItalic(true);
        }
    } else if (aText.startsWith("_____" )) {
        setCurrentBlockState(1);
        format.setForeground(QColor("#1D1D8F"));
    } else if (aText.startsWith('+')) {
        format.setForeground(QColor("#008B00"));
        if (aText.startsWith("+++")) {
            format.setFontWeight(QFont::Bold);
        }
    } else if (aText.startsWith('-')) {
        format.setForeground(QColor("#CD3333"));
        if (aText.startsWith("---")) {
            format.setFontWeight(QFont::Bold);
        }
    } else if (aText.startsWith("@@")) {
        format.setForeground(QColor("#1D1D8F"));
    }
    if (previousBlockState()==2 && currentBlockState()==2) {
        if (aText.startsWith("   +")) {
            format.setForeground(QColor("#008B00"));
        } else if (aText.startsWith("   -")) {
            format.setForeground(QColor("#CD3333"));
        }
    }
    setFormat(0, aText.length(), format);
}


/*!
    \fn DiffSyntax::~DiffSyntax()
 */
 DiffSyntax::~DiffSyntax()
{
}
