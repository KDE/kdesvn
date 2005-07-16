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
#ifndef RANGEINPUT_IMPL_H
#define RANGEINPUT_IMPL_H

#include "rangeinput.h"
#include "svncpp/revision.hpp"
#include <qpair.h>

class Rangeinput_impl: public RangeInputDlg {
Q_OBJECT
public:
    Rangeinput_impl(QWidget *parent = 0, const char *name = 0);
    virtual ~Rangeinput_impl();

    typedef QPair<svn::Revision,svn::Revision> revision_range;

    revision_range getRange();

protected slots:
    virtual void onHelp();
    virtual void stopHeadToggled(bool);
    virtual void stopBaseToggled(bool);
    virtual void stopNumberToggled(bool);
    virtual void startHeadToggled(bool);
    virtual void startBaseToggled(bool);
    virtual void startNumberToggled(bool);
};

#endif
