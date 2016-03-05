/***************************************************************************
 *   Copyright (C) 2016 Christian Ehrlicher <ch.ehrlicher@gmx.de>          *
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

#include "ksvndialog.h"

#include <QPushButton>
#include "helpers/windowgeometryhelper.h"

KSvnDialog::KSvnDialog(const QString &configGroupName, QWidget *parent)
    : QDialog(parent)
    , m_configGroupName(configGroupName)
{}

KSvnDialog::~KSvnDialog()
{
    WindowGeometryHelper::save(this, m_configGroupName);
}

void KSvnDialog::setDefaultButton(QPushButton *defaultButton)
{
    if (defaultButton) {
        defaultButton->setDefault(true);
        defaultButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    }
}

void KSvnDialog::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);
    WindowGeometryHelper::restore(this, m_configGroupName);
}
