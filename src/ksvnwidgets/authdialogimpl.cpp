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
#include "authdialogimpl.h"
#include "authdialogwidget.h"

#include "src/settings/kdesvnsettings.h"

#include <kpassworddialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <qcheckbox.h>
#include <qlabel.h>

AuthDialogImpl::AuthDialogImpl(const QString & realm,const QString&user,QWidget *parent, const char *name)
    :KDialog(parent)
{
    setObjectName(name);
    m_AuthWidget = new AuthDialogWidget(realm,user,parent);
    setMainWidget(m_AuthWidget);
    setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Help);
    connect(this, SIGNAL(helpClicked()), m_AuthWidget, SLOT(slotHelp()));
}

const QString AuthDialogImpl::Username()const
{
    return m_AuthWidget->Username();
}

const QString AuthDialogImpl::Password()
{
    return m_AuthWidget->Password();
}

bool AuthDialogImpl::maySave()const
{
    return m_AuthWidget->maySave();
}

#include "authdialogimpl.moc"
