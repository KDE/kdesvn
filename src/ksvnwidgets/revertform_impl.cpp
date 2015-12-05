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
#include "revertform_impl.h"
#include "depthselector.h"

#include <qstringlist.h>

/*!
    \fn RevertFormImpl::RevertFormImpl(QWidget*parent,const char*name)
 */
RevertFormImpl::RevertFormImpl(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    setMinimumSize(minimumSizeHint());
}

void RevertFormImpl::setRecursive(bool rec)
{
    m_DepthSelect->setRecursive(rec);
}

/*!
    \fn RevertFormImpl::~RevertFormImpl()
 */
RevertFormImpl::~RevertFormImpl()
{
}

svn::Depth RevertFormImpl::getDepth()const
{
    return m_DepthSelect->getDepth();
}

/*!
    \fn RevertFormImpl::setDispList(const QStringList&_list)
 */
void RevertFormImpl::setDispList(const QStringList &_list)
{
    m_ItemsList->addItems(_list);
}
