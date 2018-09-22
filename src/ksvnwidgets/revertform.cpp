/***************************************************************************
 *   Copyright (C) 2106 by Christian Ehrlicher <ch.ehrlicher@gmx.de>       *
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

#include "revertform.h"
#include "depthselector.h"
#include "ui_revertform.h"

RevertForm::RevertForm(const QStringList &files, QWidget *parent)
    : KSvnDialog(QLatin1String("revert_items_dialog"), parent)
    , m_ui(new Ui::RevertForm)
{
    m_ui->setupUi(this);
    m_ui->m_ItemsList->addItems(files);
    setDefaultButton(m_ui->buttonBox->button(QDialogButtonBox::Ok));
    connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

RevertForm::~RevertForm()
{
    delete m_ui;
}

svn::Depth RevertForm::getDepth() const
{
    return m_ui->m_DepthSelect->getDepth();
}
