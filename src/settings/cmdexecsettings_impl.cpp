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

#include "cmdexecsettings_impl.h"

CmdExecSettings_impl::CmdExecSettings_impl(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    kcfg_cmdline_log_minline->setSuffix(i18n(" line(s)"));
    kcfg_cmdline_log_minline->setEnabled(kcfg_cmdline_show_logwindow->isChecked());
    kcfg_kio_standard_logmsg->setEnabled(kcfg_kio_use_standard_logmsg->isChecked());
    kcfg_no_konqueror_toplevelmenu->setDisabled(kcfg_no_konqueror_contextmenu->isChecked());
}

CmdExecSettings_impl::~CmdExecSettings_impl()
{
}
