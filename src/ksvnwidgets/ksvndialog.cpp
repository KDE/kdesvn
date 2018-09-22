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

#include <QApplication>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <KHelpClient>
#include "helpers/windowgeometryhelper.h"

KSvnDialog::KSvnDialog(const QString &configGroupName, QWidget *parent)
    : QDialog(parent ? parent : QApplication::activeModalWidget())
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

// ----------------------------------------------------------------
KSvnSimpleOkDialog::KSvnSimpleOkDialog(const QString &configGroupName, QWidget *parent)
    : KSvnDialog(configGroupName, parent)
    , m_layout(new QVBoxLayout(this))
    , m_bBox(new QDialogButtonBox(QDialogButtonBox::Ok, this))
    , m_bBoxAdded(false)
{
    connect(m_bBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_bBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_bBox, &QDialogButtonBox::helpRequested, this, &KSvnSimpleOkDialog::onHelpRequested);
    setDefaultButton(m_bBox->button(QDialogButtonBox::Ok));
}

void KSvnSimpleOkDialog::setWithCancelButton()
{
    m_bBox->setStandardButtons(m_bBox->standardButtons() | QDialogButtonBox::Cancel);
}

void KSvnSimpleOkDialog::addWidget(QWidget *widget)
{
     m_layout->addWidget(widget);
}

void KSvnSimpleOkDialog::addButtonBox()
{
    if (!m_bBoxAdded) {
        m_bBoxAdded = true;
        m_layout->addWidget(m_bBox);
    }
}

void KSvnSimpleOkDialog::setHelp(const QString &context)
{
    m_helpContext = context;
    m_bBox->setStandardButtons(m_bBox->standardButtons() | QDialogButtonBox::Help);
}

int KSvnSimpleOkDialog::exec()
{
    addButtonBox();
    return KSvnDialog::exec();
}

void KSvnSimpleOkDialog::onHelpRequested()
{
    KHelpClient::invokeHelp(m_helpContext, QLatin1String("kdesvn"));
}
