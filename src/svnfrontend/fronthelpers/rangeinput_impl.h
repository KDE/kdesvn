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
#ifndef RANGEINPUT_IMPL_H
#define RANGEINPUT_IMPL_H

#include "svnqt/revision.h"
#include "ui_rangeinput.h"
#include <QPair>

class Rangeinput_impl final : public QWidget, public Ui::RangeInput
{
    Q_OBJECT
public:
    explicit Rangeinput_impl(QWidget *parent = nullptr);

    typedef QPair<svn::Revision, svn::Revision> revision_range;

    static bool getRevisionRange(revision_range &range,
                                 bool bWithWorking = true,
                                 bool bStartOnly = false,
                                 const svn::Revision &preset = svn::Revision(),
                                 QWidget *parent = nullptr);

    revision_range getRange() const;

    void setStartOnly(bool theValue);
    void setNoWorking(bool aValue);

    bool StartOnly() const;
    void setHeadDefault();

protected slots:
    void onHelp();
    void stopHeadToggled(bool);
    void stopBaseToggled(bool);
    void stopNumberToggled(bool);
    void startHeadToggled(bool);
    void startBaseToggled(bool);
    void startNumberToggled(bool);
    void stopDateToggled(bool);
    void startDateToggled(bool);

protected:
    bool m_StartOnly;
};

#endif
