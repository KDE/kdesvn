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

#include "deleteform.h"
#include "ui_deleteform.h"

DeleteForm::DeleteForm(const QStringList &files, QWidget *parent)
    : KSvnDialog(QLatin1String("delete_items_dialog"), parent)
    , m_ui(new Ui::DeleteForm)
{
    m_ui->setupUi(this);
    m_ui->m_ItemsList->addItems(files);
    setDefaultButton(m_ui->buttonBox->button(QDialogButtonBox::Yes));
    connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

DeleteForm::~DeleteForm()
{
    delete m_ui;
}

void DeleteForm::showExtraButtons(bool show)
{
    m_ui->m_keepLocal->setVisible(show);
    m_ui->m_forceDelete->setVisible(show);
}

bool DeleteForm::keep_local() const
{
    return m_ui->m_keepLocal->isChecked();
}

bool DeleteForm::force_delete() const
{
    return m_ui->m_forceDelete->isChecked();
}
