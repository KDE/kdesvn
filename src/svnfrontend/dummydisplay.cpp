/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
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
#include "dummydisplay.h"
#include "svnqt/revision.hpp"
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
