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
#include "dummydisplay.h"
#include "svnqt/revision.h"
#include <QList>

DummyDisplay::DummyDisplay()
 : ItemDisplay()
{
}


DummyDisplay::~DummyDisplay()
{
}

QWidget*DummyDisplay::realWidget()
{
    return 0L;
}

SvnItem*DummyDisplay::Selected()const
{
    return 0L;
}

void DummyDisplay::SelectionList(QList<SvnItem*>&)const
{
}

bool DummyDisplay::openUrl( const KUrl &,bool)
{
    return false;
}

SvnItem*DummyDisplay::SelectedOrMain()const
{
    return 0;
}

const svn::Revision&DummyDisplay::baseRevision()const
{
    static svn::Revision fake(svn::Revision::UNDEFINED);
    return fake;
}
