/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht   *
 *   ral@alwins-world.de   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "rangeinput_impl.h"

#include <qpushbutton.h>
#include <qradiobutton.h>
#include <knuminput.h>

Rangeinput_impl::Rangeinput_impl(QWidget *parent, const char *name)
    :RangeInputDlg(parent, name)
{
    m_startRevInput->setRange(0,INT_MAX,1,false);
    m_endRevInput->setRange(0,INT_MAX,1,false);
}

Rangeinput_impl::~Rangeinput_impl()
{
}

void Rangeinput_impl::startNumberToggled(bool how)
{
    m_startRevInput->setEnabled(how);
}

void Rangeinput_impl::startBaseToggled(bool how)
{
    if (how) m_startRevInput->setEnabled(!how);
}

void Rangeinput_impl::startHeadToggled(bool how)
{
    if (how) m_startRevInput->setEnabled(!how);
}

void Rangeinput_impl::onHelp()
{
}


void Rangeinput_impl::stopHeadToggled(bool how)
{
    if (how) m_endRevInput->setEnabled(!how);
}


void Rangeinput_impl::stopBaseToggled(bool how)
{
    if (how) m_endRevInput->setEnabled(!how);
}


void Rangeinput_impl::stopNumberToggled(bool how)
{
    m_endRevInput->setEnabled(how);
}

Rangeinput_impl::revision_range Rangeinput_impl::getRange()
{
    revision_range ret;
    if (m_startStartButton->isChecked()) {
        ret.first = svn::Revision::START;
    } else if (m_startHeadButton->isChecked()) {
        ret.first = svn::Revision::HEAD;
    } else if (m_startNumberButton->isChecked()) {
        ret.first = m_startRevInput->value();
    }
    if (m_stopStartButton->isChecked()) {
        ret.second = svn::Revision::START;
    } else if (m_stopHeadButton->isChecked()) {
        ret.second = svn::Revision::HEAD;
    } else if (m_stopNumberButton->isChecked()) {
        ret.second = m_endRevInput->value();
    }
    return ret;
}

#include "rangeinput_impl.moc"
