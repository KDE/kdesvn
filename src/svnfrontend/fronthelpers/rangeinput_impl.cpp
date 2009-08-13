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
#include "rangeinput_impl.h"
#include "helpers/sub2qt.h"

#include <qpushbutton.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <knuminput.h>
#include <kdatetimewidget.h>
#include <klocale.h>
#include <kdebug.h>

Rangeinput_impl::Rangeinput_impl(QWidget *parent, const char *name)
//     :RangeInputDlg(parent, name)
    : QWidget(parent)
{
    setupUi(this);
    setObjectName(name);

    m_startRevInput->setRange(0,INT_MAX,1,false);
    m_endRevInput->setRange(0,INT_MAX,1,false);
    m_startRevInput->setValue(1);
    m_endRevInput->setValue(1);
    m_startDateInput->setDateTime(QDateTime::currentDateTime ());
    m_stopDateInput->setDateTime(QDateTime::currentDateTime ());
    m_stopDateInput->setEnabled(false);
    m_startDateInput->setEnabled(false);
    m_stopHeadButton->setChecked(true);
}

Rangeinput_impl::~Rangeinput_impl()
{
}

void Rangeinput_impl::startNumberToggled(bool how)
{
    m_startRevInput->setEnabled(how);
    if (how) {
        m_startDateInput->setEnabled(!how);
    }
}

void Rangeinput_impl::startBaseToggled(bool how)
{
    if (how) {
        m_startRevInput->setEnabled(!how);
        m_startDateInput->setEnabled(!how);
    }
}

void Rangeinput_impl::startHeadToggled(bool how)
{
    if (how) {
        m_startRevInput->setEnabled(!how);
        m_startDateInput->setEnabled(!how);
    }
}

void Rangeinput_impl::setNoWorking(bool aValue)
{
    if (!aValue) {
        if (m_startWorkingButton->isChecked()) {
            m_startHeadButton->setChecked(false);
        }
        if (m_stopWorkingButton->isChecked()) {
            m_stopHeadButton->setChecked(false);
        }
    }
    m_startWorkingButton->setEnabled(!aValue);
    m_stopWorkingButton->setEnabled(!aValue);
}

void Rangeinput_impl::onHelp()
{
}


void Rangeinput_impl::stopHeadToggled(bool how)
{
    if (how) {
        m_endRevInput->setEnabled(!how);
        m_stopDateInput->setEnabled(!how);
    }
}


void Rangeinput_impl::stopBaseToggled(bool how)
{
    if (how) {
        m_endRevInput->setEnabled(!how);
        m_stopDateInput->setEnabled(!how);
    }
}


void Rangeinput_impl::stopNumberToggled(bool how)
{
    m_endRevInput->setEnabled(how);
    if (how) {
        m_stopDateInput->setEnabled(!how);
    }
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
    } else if (m_startDateButton->isChecked()) {
        ret.first=m_startDateInput->dateTime();
    } else if (m_startWorkingButton->isChecked()) {
        ret.first = svn::Revision::WORKING;
    }
    if (m_stopStartButton->isChecked()) {
        ret.second = svn::Revision::START;
    } else if (m_stopHeadButton->isChecked()) {
        ret.second = svn::Revision::HEAD;
    } else if (m_stopNumberButton->isChecked()) {
        ret.second = m_endRevInput->value();
    } else if (m_stopDateButton->isChecked()) {
        ret.second=m_stopDateInput->dateTime();
    } else if (m_stopWorkingButton->isChecked()) {
        ret.second = svn::Revision::WORKING;
    }
    return ret;
}

void Rangeinput_impl::stopDateToggled(bool how)
{
    m_stopDateInput->setEnabled(how);
    if (how) {
        m_endRevInput->setEnabled(!how);
    }
}


void Rangeinput_impl::startDateToggled(bool how)
{
    m_startDateInput->setEnabled(how);
    if (how) {
        m_startRevInput->setEnabled(!how);
    }
}


bool Rangeinput_impl::StartOnly() const
{
    return m_StartOnly;
}

void Rangeinput_impl::setHeadDefault()
{
    m_stopHeadButton->setChecked(true);
    m_startHeadButton->setChecked(true);
}

void Rangeinput_impl::setStartOnly(bool theValue)
{
    m_StartOnly = theValue;
    if (m_StartOnly) {
        layout()->removeWidget(m_stopRevBox);
        m_stopRevBox->hide();
        m_startRevBox->setTitle(i18n("Select revision"));
    } else {
        layout()->addWidget(m_stopRevBox);
        m_stopRevBox->show();
        m_startRevBox->setTitle(i18n( "Start with revision" ));
    }
    updateGeometry();
    setMinimumSize(minimumSizeHint());
    resize( QSize(397, 272).expandedTo(minimumSizeHint()) );
}

#include "rangeinput_impl.moc"
