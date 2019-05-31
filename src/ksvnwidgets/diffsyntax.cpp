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

#include <QFontDatabase>
#include <QRegularExpression>

void DiffSyntax::highlightBlock(const QString &aText)
{
    static const QRegularExpression a(QLatin1String("^\\w+:\\s.*$")); // filename (Index: foo/bar.txt)
    QTextCharFormat format;
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    bool bIsModifiedLine = false;

    if (previousBlockState() == 1) {
        setCurrentBlockState(2);
    } else if (previousBlockState() == 2) {
        if (a.match(aText).capturedStart() != 0) {
            setCurrentBlockState(2);
        }
    }

    if (a.match(aText).hasMatch()) {  // filename (Index: foo/bar.txt)
        format.setForeground(QColor(0x66, 0x00, 0x33));
        if (previousBlockState() == 1 || previousBlockState() == 2) {
            format.setFontWeight(QFont::Bold);
        } else {
            format.setFontItalic(true);
        }
    } else if (aText.startsWith(QLatin1String("_____"))) {
        setCurrentBlockState(1);
        format.setForeground(QColor(0x1D, 0x1D, 0x8F));
    } else if (aText.startsWith(QLatin1Char('+'))) {  // added line in new file
        format.setForeground(QColor(0x00, 0x8B, 0x00));
        if (aText.startsWith(QLatin1String("+++"))) { // new file name
            format.setFontWeight(QFont::Bold);
        } else {
            bIsModifiedLine = true;
        }
    } else if (aText.startsWith(QLatin1Char('-'))) {  // removed line in old file
        format.setForeground(QColor(0xCD, 0x33, 0x33));
        if (aText.startsWith(QLatin1String("---"))) { // old file name
            format.setFontWeight(QFont::Bold);
        } else {
            bIsModifiedLine = true;
        }
    } else if (aText.startsWith(QLatin1String("@@"))) { // line numbers
        format.setForeground(QColor(0x1D, 0x1D, 0x8F));
    }
    if (previousBlockState() == 2 && currentBlockState() == 2) {
        if (aText.startsWith(QLatin1String("   +"))) {
            format.setForeground(QColor(0x00, 0x8B, 0x00));
        } else if (aText.startsWith(QLatin1String("   -"))) {
            format.setForeground(QColor(0xCD, 0x33, 0x33));
        }
    }
    setFormat(0, aText.length(), format);
    // highlight trailing spaces
    if (bIsModifiedLine && aText.endsWith(QLatin1Char(' '))) {
        static const QRegularExpression hlTrailingSpaceRx(QLatin1String("[^\\s]")); // search the last non-space
        const int idx = aText.lastIndexOf(hlTrailingSpaceRx);
        format.setBackground(format.foreground());
        setFormat(idx + 1, aText.length() - idx - 1, format); // only spaces in this range
    }
}
