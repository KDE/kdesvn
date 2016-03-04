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

#include <QPushButton>
#include "helpers/windowgeometryhelper.h"

DeleteForm::DeleteForm(const QStringList &files, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::DeleteForm)
{
    m_ui->setupUi(this);
    m_ui->m_ItemsList->addItems(files);
    QPushButton *okButton = m_ui->buttonBox->button(QDialogButtonBox::Yes);
    if (okButton) {
        okButton->setDefault(true);
        okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    }
    connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

DeleteForm::~DeleteForm()
{
    WindowGeometryHelper wgh(this, QLatin1String("delete_items_dialog"), false);
    wgh.save();
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

void DeleteForm::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);
    WindowGeometryHelper wgh(this, QLatin1String("delete_items_dialog"));
}
