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
#include "dispcolorsettings_impl.h"
#include <kcolorbutton.h>
#include <qcheckbox.h>

DispColorSettings_impl::DispColorSettings_impl(QWidget *parent, const char *name)
    :QWidget(parent)
{
    setupUi(this);
    setObjectName(name);
    coloredStateToggled(kcfg_colored_state->isChecked());
}

DispColorSettings_impl::~DispColorSettings_impl()
{
}

void DispColorSettings_impl::coloredStateToggled(bool how)
{
    kcfg_color_locked_item->setEnabled(how);
    kcfg_color_changed_item->setEnabled(how);
    kcfg_color_item_deleted->setEnabled(how);
    kcfg_color_item_added->setEnabled(how);
    kcfg_color_need_update->setEnabled(how);
    kcfg_color_missed_item->setEnabled(how);
    kcfg_color_notversioned_item->setEnabled(how);
    kcfg_color_conflicted_item->setEnabled(how);
}


#include "dispcolorsettings_impl.moc"
