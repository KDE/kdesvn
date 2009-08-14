/***************************************************************************
 *   Copyright (C) 2007 by Rajko Albrecht  ral@alwins-world.de             *
 *   http://kdesvn.alwins-world.de/                                        *
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
#include "encodingselector_impl.h"
#include <kdebug.h>
#include <kcharsets.h>
#include <kglobal.h>

#include <qcombobox.h>

EncodingSelector_impl::EncodingSelector_impl(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    m_encodingList->addItems( KGlobal::charsets()->availableEncodingNames());
}

EncodingSelector_impl::EncodingSelector_impl(const QString&cur,QWidget *parent, const char *name)
    : QWidget(parent)
{
    setupUi(this);
    if (name) {
        setObjectName(name);
    }

    m_encodingList->addItems( KGlobal::charsets()->availableEncodingNames());
    setCurrentEncoding(cur);
}

void EncodingSelector_impl::setCurrentEncoding(const QString&cur)
{
    for (int j = 1;j<m_encodingList->count();++j ) {
        if(m_encodingList->itemText(j)==cur) {
            m_encodingList->setCurrentIndex(j);
            break;
        }
    }
}

void EncodingSelector_impl::itemActivated(int which)
{
    if (which == 0) {
        emit TextCodecChanged(QString(""));
    } else {
        emit TextCodecChanged(m_encodingList->currentText());
    }
}

#include "encodingselector_impl.moc"

