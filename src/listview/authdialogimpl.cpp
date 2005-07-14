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
#include "authdialogimpl.h"
#include <kpassdlg.h>
#include <klineedit.h>
#include <qcheckbox.h>
#include <qlabel.h>

AuthDialogImpl::AuthDialogImpl(const QString & realm,QWidget *parent, const char *name)
    :AuthDialogData(parent, name),curPass("")
{
    if (!realm.isEmpty()) {
        m_RealmLabel->setText(m_RealmLabel->text()+" "+realm);
        resize( QSize(334, 158).expandedTo(minimumSizeHint()) );
    }
}

void AuthDialogImpl::slotHelp()
{
}

const QString AuthDialogImpl::Username()const
{
    return m_UsernameEdit->text();
}

const QString AuthDialogImpl::Password()
{
    /* as described in interface description wie must make a copy of string */
    curPass.setAscii(m_PasswordEdit->password());
    return curPass;
}

bool AuthDialogImpl::maySave()const
{
    return m_StorePasswordButton->isChecked();
}

#include "authdialogimpl.moc"
