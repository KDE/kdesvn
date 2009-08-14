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
#include "authdialogwidget.h"
#include "src/settings/kdesvnsettings.h"

#include <kpassworddialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <qcheckbox.h>
#include <qlabel.h>

AuthDialogWidget::AuthDialogWidget(const QString & realm,const QString&user,QWidget *parent, const char *name)
    :QWidget(parent),Ui::AuthDialogWidget(),curPass("")
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

void AuthDialogWidget::slotHelp()
{
}

const QString AuthDialogWidget::Username()const
{
    return m_UsernameEdit->text();
}

const QString AuthDialogWidget::Password()
{
    return m_PasswordEdit->text();
}

bool AuthDialogWidget::maySave()const
{
    return m_StorePasswordButton->isChecked();
}

#include "authdialogwidget.moc"
