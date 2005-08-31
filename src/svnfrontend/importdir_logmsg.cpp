/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht                                  *
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
#include <klocale.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qvbox.h>

Importdir_logmsg::Importdir_logmsg(QWidget *parent, const char *name)
 : Logmsg_impl(parent, name)
{
    m_createDirBox = new QCheckBox("",this,"create_dir_checkbox");
    createDirboxDir();
    m_ItemsLayout->addWidget(m_createDirBox);
    m_createDirBox->setChecked(true);
}

Importdir_logmsg::~Importdir_logmsg()
{
}

bool Importdir_logmsg::createDir()
{
    return m_createDirBox->isChecked();
}

void Importdir_logmsg::createDirboxDir(const QString & which)
{
    m_createDirBox->setText(i18n("Create subdir %1 on import").arg(which.isEmpty()?"(Last part)":which));
}

#include "importdir_logmsg.moc"
