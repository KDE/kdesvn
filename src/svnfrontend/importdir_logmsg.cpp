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
#include "importdir_logmsg.h"

#include "svnqt/version_check.h"

#include <KLocalizedString>
#include <QCheckBox>


Importdir_logmsg::Importdir_logmsg(QWidget *parent)
    : Commitmsg_impl(parent)
{
    m_createDirBox = new QCheckBox(this);
    hideKeepsLock(true);
    createDirboxDir();
    addItemWidget(m_createDirBox);
    m_createDirBox->setChecked(true);
    QHBoxLayout *tmpLayout = new QHBoxLayout();
    m_noIgnore = new QCheckBox(this);
    m_noIgnore->setText(i18n("No ignore"));
    m_noIgnore->setToolTip(i18n("If set, add files or directories that match ignore patterns."));
    tmpLayout->addWidget(m_noIgnore);
    //LogmessageDataLayout->addWidget(m_createDirBox);
    if (svn::Version::version_major() > 1 || svn::Version::version_minor() > 4) {
        m_ignoreUnknownNodes = new QCheckBox(this);
        m_ignoreUnknownNodes->setText(i18n("Ignore unknown node types"));
        m_ignoreUnknownNodes->setToolTip(i18n("Should files with unknown node types be ignored"));
        m_ignoreUnknownNodes->setWhatsThis(i18n("Ignore files of which the node type is unknown, such as device files and pipes."));
        tmpLayout->addWidget(m_ignoreUnknownNodes);
        //addItemWidget(m_ignoreUnknownNodes);
    } else {
        m_ignoreUnknownNodes = nullptr;
    }
    QSpacerItem *m_leftspacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    tmpLayout->addItem(m_leftspacer);
    if (layout()) {
        layout()->addItem(tmpLayout);
    }
}

Importdir_logmsg::~Importdir_logmsg()
{
}

bool Importdir_logmsg::noIgnore()
{
    return m_noIgnore->isChecked();
}

bool Importdir_logmsg::ignoreUnknownNodes()
{
    return m_ignoreUnknownNodes ? m_ignoreUnknownNodes->isChecked() : false;
}

bool Importdir_logmsg::createDir()
{
    return m_createDirBox->isChecked();
}

void Importdir_logmsg::createDirboxDir(const QString &which)
{
    m_createDirBox->setText(i18n("Create subdirectory %1 on import", (which.isEmpty() ? i18n("(Last part)") : which)));
}
