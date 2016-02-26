/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
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
#include "propertyitem.h"
#include <kiconloader.h>

PropertyListViewItem::PropertyListViewItem(QTreeWidget *parent, const QString &aName, const QString &aValue)
    : QTreeWidgetItem(parent, _RTTI_), m_currentName(aName), m_startName(aName), m_currentValue(aValue), m_startValue(aValue), m_deleted(false)
{
    setText(0, startName());
    setText(1, startValue());
}

PropertyListViewItem::PropertyListViewItem(QTreeWidget *parent)
    : QTreeWidgetItem(parent, _RTTI_), m_currentName(), m_startName(), m_currentValue(), m_startValue(), m_deleted(false)
{
}

PropertyListViewItem::~PropertyListViewItem()
{
}

void PropertyListViewItem::checkValue()
{
    m_currentValue = text(1);
}

void PropertyListViewItem::checkName()
{
    m_currentName = text(0);
}

bool PropertyListViewItem::different()const
{
    return m_currentName != m_startName || m_currentValue != m_startValue || deleted();
}

void PropertyListViewItem::deleteIt()
{
    m_deleted = true;
    setIcon(0, KIconLoader::global()->loadIcon("dialog-cancel", KIconLoader::Desktop, 16));
}

void PropertyListViewItem::unDeleteIt()
{
    m_deleted = false;
    setIcon(0, QIcon());
}

bool PropertyListViewItem::protected_Property(const QString &what)
{
    if (
        what.compare("svn:mergeinfo") == 0 ||
        what.compare("svn:special") == 0
    ) {
        return true;
    }
    return false;
}
