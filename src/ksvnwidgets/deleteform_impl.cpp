/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
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

#include "deleteform_impl.h"

DeleteForm_impl::DeleteForm_impl(QWidget*parent)
    :QWidget(parent),DeleteForm()
{
    setupUi(this);
}

DeleteForm_impl::~DeleteForm_impl()
{
}

void DeleteForm_impl::showExtraButtons(bool show)
{
    m_keepLocal->setVisible(show);
    m_forceDelete->setVisible(show);
}

void DeleteForm_impl::setStringList(const QStringList&aList)
{
    m_ItemsList->clear();
    m_ItemsList->insertItems(0,aList);
}

bool DeleteForm_impl::keep_local()const
{
    return m_keepLocal->isChecked();
}

bool DeleteForm_impl::force_delete()const
{
    return m_forceDelete->isChecked();
}

#include "deleteform_impl.moc"
