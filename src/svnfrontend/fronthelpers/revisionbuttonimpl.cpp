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
#include "revisionbuttonimpl.h"
#include "svnfrontend/fronthelpers/rangeinput_impl.h"
#include "settings/kdesvnsettings.h"

RevisionButtonImpl::RevisionButtonImpl(QWidget *parent)
    : QWidget(parent),
      m_Rev(svn::Revision::UNDEFINED), m_noWorking(false)
{
    setupUi(this);
}

void RevisionButtonImpl::setRevision(const svn::Revision &aRev)
{
    m_Rev = aRev;
    m_RevisionButton->setText(m_Rev.toString());
    emit revisionChanged();
}

void RevisionButtonImpl::askRevision()
{
    Rangeinput_impl::revision_range range;
    if (Rangeinput_impl::getRevisionRange(range, !m_noWorking, true, m_Rev)) {
        setRevision(range.first);
    }
}

void RevisionButtonImpl::setNoWorking(bool how)
{
    m_noWorking = how;
}
