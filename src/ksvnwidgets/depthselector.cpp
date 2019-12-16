/***************************************************************************
 *   Copyright (C) 2007 by Rajko Albrecht  ral@alwins-world.de             *
 *   https://kde.org/applications/development/org.kde.kdesvn               *
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
#include "depthselector.h"
#include "ui_depthselector.h"

DepthSelector::DepthSelector(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::DepthSelector)
{
    m_ui->setupUi(this);
    m_ui->m_DepthCombo->setCurrentIndex(3);
}

DepthSelector::~DepthSelector()
{
    delete m_ui;
}

void DepthSelector::addItemWidget(QWidget *aWidget)
{
    m_ui->hboxLayout->removeItem(m_ui->spacerItem);
    aWidget->setParent(this);
    m_ui->hboxLayout->addWidget(aWidget);
    aWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_ui->hboxLayout->addItem(m_ui->spacerItem);
}

svn::Depth DepthSelector::getDepth() const
{
    switch (m_ui->m_DepthCombo->currentIndex()) {
    case 0:
        return svn::DepthEmpty;
    case 1:
        return svn::DepthFiles;
    case 2:
        return svn::DepthImmediates;
    case 3:
    default:
          break;
    }
    return svn::DepthInfinity;
}

void DepthSelector::hideDepth(bool hide)
{
    m_ui->m_DepthCombo->setHidden(hide);
}
