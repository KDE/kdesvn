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
#include "authdialogimpl.h"
#include "authdialogwidget.h"

#include "settings/kdesvnsettings.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

AuthDialogImpl::AuthDialogImpl(const QString &realm, const QString &user, QWidget *parent)
    : QDialog(parent)
    , m_AuthWidget(new AuthDialogWidget(realm, user, parent))
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(m_AuthWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::Help);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}

AuthDialogImpl::~AuthDialogImpl()
{
    delete m_AuthWidget;
}

QString AuthDialogImpl::Username() const
{
    return m_AuthWidget->Username();
}

QString AuthDialogImpl::Password() const
{
    return m_AuthWidget->Password();
}

bool AuthDialogImpl::maySave()const
{
    return m_AuthWidget->maySave();
}
