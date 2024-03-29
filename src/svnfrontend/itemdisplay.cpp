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

#include "itemdisplay.h"
#include "settings/kdesvnsettings.h"
#include "svnitem.h"
#include "svnqt/status.h"

ItemDisplay::ItemDisplay()
    : m_isWorkingCopy(false)
    , m_isNetworked(false)
{
}

bool ItemDisplay::isWorkingCopy() const
{
    return m_isWorkingCopy;
}

QString ItemDisplay::baseUri() const
{
    return m_baseUri;
}

QUrl ItemDisplay::baseUriAsUrl() const
{
    return m_isWorkingCopy ? QUrl::fromLocalFile(m_baseUri) : QUrl(m_baseUri);
}

/*!
    \fn ItemDisplay::isNetworked()const
 */
bool ItemDisplay::isNetworked() const
{
    return m_isNetworked;
}

void ItemDisplay::setWorkingCopy(bool how)
{
    m_isWorkingCopy = how;
}

void ItemDisplay::setNetworked(bool how)
{
    m_isNetworked = how;
}

void ItemDisplay::setBaseUri(const QString &uri)
{
    m_baseUri = uri;
    /* otherwise subversion lib asserts! */
    while (m_baseUri.endsWith(QLatin1Char('/'))) {
        m_baseUri.chop(1);
    }
}

QString ItemDisplay::lastError() const
{
    return m_LastException;
}

/*!
    \fn ItemDisplay::filterOut(const SvnItem*)
 */
bool ItemDisplay::filterOut(const SvnItem *item)
{
    if (!item->stat()->validReposStatus()) {
        if ((item->isIgnored() && !Kdesvnsettings::display_ignored_files())
            || (Kdesvnsettings::hide_unchanged_files() && item->isRealVersioned() && !item->isModified() && !item->isChildModified())
            || (!Kdesvnsettings::display_unknown_files() && !item->stat()->isVersioned())) {
            return true;
        }
    }
    return false;
}

/*!
    \fn ItemDisplay::relativePath(const SvnItem*item)
 */
QString ItemDisplay::relativePath(const SvnItem *item) const
{
    if (!isWorkingCopy() || !item->fullName().startsWith(baseUri())) {
        return item->fullName();
    }
    QString name = item->fullName();
    if (name == baseUri()) {
        name = QLatin1Char('.');
    } else {
        name = name.right(name.length() - baseUri().length() - 1);
    }
    if (name.isEmpty()) {
        name = QLatin1Char('.');
    }
    return name;
}
