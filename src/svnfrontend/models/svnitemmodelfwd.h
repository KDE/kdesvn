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

#ifndef SVNITEMMODELFWD_H
#define SVNITEMMODELFWD_H

#include <QFlags>

class SvnItemModelNode;
class SvnItemModelNodeDir;
class SvnItemModel;

namespace svnmodel
{

enum ItemType { None = 0x0, Dir = 1, File = 2, All = Dir | File };

Q_DECLARE_FLAGS(ItemTypeFlag, ItemType)
Q_DECLARE_OPERATORS_FOR_FLAGS(svnmodel::ItemTypeFlag)
}

#endif
