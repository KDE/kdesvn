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
#include "depthselector.h"
#include "src/svnqt/version_check.hpp"

#include <kdebug.h>
#include <klocale.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlayout.h>

DepthSelector::DepthSelector(QWidget *parent, const char *name)
    :DepthSettings(parent, name)
{
    if (svn::Version::version_major()>1|| svn::Version::version_minor()>5 ) {
        m_recurse = 0L;
        kdDebug()<<minimumSizeHint()<<endl;
    } else {
        delete depthButtonGroup;
        depthButtonGroup=0;
        m_recurse = new QCheckBox( this, "m_RecursiveButton" );
        m_recurse->setChecked( TRUE );
        m_recurse->setText(i18n( "Recursive" ));
        DepthFormLayout->addWidget( m_recurse );
        DepthFormLayout->setMargin(0);
        kdDebug()<<minimumSizeHint()<<endl;
    }
    setMinimumHeight(minimumSizeHint().height());
}

DepthSelector::~DepthSelector()
{
}

/*!
    \fn DepthSelector::getDepth()const
 */
svn::Depth DepthSelector::getDepth()const
{
    if (depthButtonGroup) {
        switch (depthButtonGroup->selectedId()){
            case 0:
                return svn::DepthEmpty;
                break;
            case 1:
                return svn::DepthFiles;
                break;
            case 2:
                return svn::DepthImmediates;
                break;
            case 3:
            default:
                return svn::DepthInfinity;
        }
    } else {
        return (m_recurse->isChecked()?svn::DepthInfinity:svn::DepthEmpty);
    }
}

#include "depthselector.moc"
