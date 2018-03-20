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

PropertyListViewItem::PropertyListViewItem(QTreeWidget *parent, const QString &aStartName, const QString &aStartValue)
    : QTreeWidgetItem(parent, _RTTI_)
    , m_currentName(aStartName)
    , m_startName(aStartName)
    , m_currentValue(aStartValue)
    , m_startValue(aStartValue)
    , m_deleted(false)
{
    setText(0, startName());
    setText(1, startValue());
}

void PropertyListViewItem::setName(const QString &name)
{
    m_currentName = name;
    setText(0, name);
}

void PropertyListViewItem::setValue(const QString &value)
{
    m_currentValue = value;
    setText(1, value);
}

bool PropertyListViewItem::different()const
{
    return m_currentName != m_startName || m_currentValue != m_startValue || deleted();
}

void PropertyListViewItem::deleteIt()
{
    m_deleted = true;
    setIcon(0, KIconLoader::global()->loadIcon(QStringLiteral("dialog-cancel"), KIconLoader::Desktop, 16));
}

void PropertyListViewItem::unDeleteIt()
{
    m_deleted = false;
    setIcon(0, QIcon());
}

bool PropertyListViewItem::protected_Property(const QString &what)
{
    return (what.compare(QLatin1String("svn:mergeinfo")) == 0 ||
            what.compare(QLatin1String("svn:special")) == 0);
}
