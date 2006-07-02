/***************************************************************************
 *   Copyright (C) 2006 by Rajko Albrecht                                  *
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
#include "svnfrontend/createrepo_impl.h"

#include <kdebug.h>
#include <kurl.h>
#include <kurlrequester.h>
#include <kcombobox.h>

#include <qcheckbox.h>

Createrepo_impl::Createrepo_impl(QWidget *parent, const char *name)
    :CreateRepo_Dlg(parent, name)
{
    m_DisableFsync->setEnabled(false);
    m_LogKeep->setEnabled(false);
}

void Createrepo_impl::fsTypeChanged(int which)
{
    m_DisableFsync->setEnabled(which==1);
    m_LogKeep->setEnabled(which==1);
}

QString Createrepo_impl::targetDir()
{
    KURL u = m_ReposPathinput->url();
    QString res = u.path();
    while (res.endsWith("/")) {
        res.truncate(res.length()-1);
    }
    return res;
}

QString Createrepo_impl::fsType()
{
    return m_FilesystemSelector->currentText();
}

bool Createrepo_impl::disableFsync()
{
    return m_DisableFsync->isChecked();
}

bool Createrepo_impl::keepLogs()
{
    return m_LogKeep->isChecked();
}

bool Createrepo_impl::createMain()
{
    return m_CreateMainDirs->isChecked();
}

#include "createrepo_impl.moc"
