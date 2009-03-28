/***************************************************************************
 *   Copyright (C) 2005-2007 by Rajko Albrecht                             *
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
#include "authdialogimpl.h"
#include "src/settings/kdesvnsettings.h"

#include <kpassworddialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <qcheckbox.h>
#include <qlabel.h>

AuthDialogImpl::AuthDialogImpl(const QString & realm,const QString&user,QWidget *parent, const char *name)
    :KDialog(parent),Ui::AuthDialog(),curPass("")
{
    setupUi(this);
    setObjectName(name);

    m_UsernameEdit->setText(user);
    m_PasswordEdit->setText("");
    m_StorePasswordButton->setChecked(Kdesvnsettings::store_passwords());
    QString text = m_StorePasswordButton->text();
    m_StorePasswordButton->setText(
            m_StorePasswordButton->text()+QString(" (%1)")
            .arg((Kdesvnsettings::passwords_in_wallet()?i18n("into KDE Wallet"):i18n("into subversions simple storage"))));
    if (!realm.isEmpty()) {
        m_RealmLabel->setText(m_RealmLabel->text()+' '+realm);
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
#if 0
    /* as described in interface description wie must make a copy of string */
    curPass = QString::fromUtf8(m_PasswordEdit->password());
    return curPass;
#endif
    return m_PasswordEdit->text();
}

bool AuthDialogImpl::maySave()const
{
    return m_StorePasswordButton->isChecked();
}

#include "authdialogimpl.moc"
