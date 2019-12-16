/***************************************************************************
 *   Copyright (C) 2007 by Rajko Albrecht  ral@alwins-world.de             *
 *   https://kde.org/applications/development/org.kde.kdesvn               *
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
#include "encodingselector_impl.h"
#include <kcharsets.h>


EncodingSelector_impl::EncodingSelector_impl(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    m_encodingList->addItems(KCharsets::charsets()->availableEncodingNames());
}

void EncodingSelector_impl::setCurrentEncoding(const QString &cur)
{
    for (int j = 1; j < m_encodingList->count(); ++j) {
        if (m_encodingList->itemText(j) == cur) {
            m_encodingList->setCurrentIndex(j);
            break;
        }
    }
}

void EncodingSelector_impl::itemActivated(int which)
{
    if (which == 0) {
        emit TextCodecChanged(QString());
    } else {
        emit TextCodecChanged(m_encodingList->currentText());
    }
}
