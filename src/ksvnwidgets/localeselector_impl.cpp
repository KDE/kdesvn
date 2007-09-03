/***************************************************************************
 *   Copyright (C) 2007 by Rajko Albrecht  ral@alwins-world.de             *
 *   http://kdesvn.alwins-world.de/                                        *
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
#include "localeselector_impl.h"
#include <kdebug.h>
#include <kcharsets.h>
#include <kglobal.h>

#include <qcombobox.h>

LocaleSelector_impl::LocaleSelector_impl(const QString&cur,QWidget *parent, const char *name)
    :LocaleSelector(parent, name)
{
    m_localeList->insertStringList( KGlobal::charsets()->availableEncodingNames());

    for (int j = 1;j<m_localeList->count();++j ) {
        if(m_localeList->text(j)==cur) {
            m_localeList->setCurrentItem(j);
            break;
        }
    }
}

void LocaleSelector_impl::itemActivated(int which)
{
    if (which == 0) {
        emit TextCodecChanged(QString(""));
    } else {
        emit TextCodecChanged(m_localeList->currentText());
    }
}

#include "localeselector_impl.moc"
