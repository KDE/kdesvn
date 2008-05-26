/***************************************************************************
 *   Copyright (C) 2005-2007 by Rajko Albrecht                             *
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

#include "src/svnqt/version_check.hpp"

#include <klocale.h>

#include <qcheckbox.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qwhatsthis.h>
#include <qtooltip.h>


Importdir_logmsg::Importdir_logmsg(QWidget *parent, const char *name)
 : Logmsg_impl(parent, name)
{
    m_createDirBox = new QCheckBox("",this,"create_dir_checkbox");
    m_keepLocksButton->hide();
    //delete m_keepLocksButton;
    createDirboxDir();
    addItemWidget(m_createDirBox);
    m_createDirBox->setChecked(true);
    QHBoxLayout* tmpLayout = new QHBoxLayout( this, 11, 6, "ExtraLayout");
    m_noIgnore = new QCheckBox("",this,"no_ignore_pattern");
    m_noIgnore->setText(i18n("No ignore"));
    QToolTip::add(m_noIgnore,i18n("If set, add files or directories that match ignore patterns."));
    tmpLayout->addWidget(m_noIgnore);
    //LogmessageDataLayout->addWidget(m_createDirBox);
    if (svn::Version::version_major()>1|| svn::Version::version_minor()>4 ) {
        m_ignoreUnknownNodes = new QCheckBox("",this,"ignore_unknown_nodes_box");
        m_ignoreUnknownNodes->setText(i18n("Ignore unknown node types"));
        QToolTip::add(m_ignoreUnknownNodes,i18n("Should files with unknown node types be ignored"));
        QWhatsThis::add(m_ignoreUnknownNodes,i18n("Ignore files of which the node type is unknown, such as device files and pipes."));
        tmpLayout->addWidget(m_ignoreUnknownNodes);
        //addItemWidget(m_ignoreUnknownNodes);
    } else {
        m_ignoreUnknownNodes=0;
    }
    QSpacerItem* m_leftspacer = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    tmpLayout->addItem(m_leftspacer);
    LogmessageDataLayout->addItem(tmpLayout);
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
    return m_ignoreUnknownNodes?m_ignoreUnknownNodes->isChecked():false;
}

bool Importdir_logmsg::createDir()
{
    return m_createDirBox->isChecked();
}

void Importdir_logmsg::createDirboxDir(const QString & which)
{
    m_createDirBox->setText(i18n("Create subdir %1 on import").arg(which.isEmpty()?i18n("(Last part)"):which));
}

#include "importdir_logmsg.moc"
