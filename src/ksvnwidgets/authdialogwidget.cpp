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
#include "authdialogwidget.h"
#include "src/settings/kdesvnsettings.h"

#include <kpassworddialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <qcheckbox.h>
#include <qlabel.h>

AuthDialogWidget::AuthDialogWidget(const QString &realm, const QString &user, QWidget *parent)
    : QWidget(parent), Ui::AuthDialogWidget(), curPass()
{
    setupUi(this);

    m_UsernameEdit->setText(user);
    m_PasswordEdit->clear();
    m_StorePasswordButton->setChecked(Kdesvnsettings::store_passwords());
    m_StorePasswordButton->setText(
        Kdesvnsettings::passwords_in_wallet()
        ? i18n("Store password (into KDE Wallet)")
        : i18n("Store password (into Subversion' simple storage)"));
    if (!realm.isEmpty()) {
        m_RealmLabel->setText(i18n("Enter authentication info for %1", realm));
        resize(QSize(334, 158).expandedTo(minimumSizeHint()));
    }
}

void AuthDialogWidget::slotHelp()
{
}

const QString AuthDialogWidget::Username()const
{
    return m_UsernameEdit->text();
}

const QString AuthDialogWidget::Password() const
{
    return m_PasswordEdit->text();
}

bool AuthDialogWidget::maySave()const
{
    return m_StorePasswordButton->isChecked();
}
