/***************************************************************************
 *   Copyright (C) 2006-2007 by Rajko Albrecht                             *
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

class RecurseCheck
{
    bool&value;
public:
    RecurseCheck(bool&aValue):value(aValue){ value=true;}
    ~RecurseCheck(){value = false;}
};

Createrepo_impl::Createrepo_impl(bool enable_compat13, bool enable_compat14, QWidget *parent, const char *name)
    :CreateRepo_Dlg(parent, name)
{
    inChangeCompat=true;
    m_DisableFsync->setEnabled(false);
    m_LogKeep->setEnabled(false);
    if (!enable_compat13){
        m_svn13compat->setEnabled(false);
        m_svn13compat->hide();
    } else {
        m_svn13compat->setEnabled(true);
    }
    if (!enable_compat14){
        m_svn14compat->setEnabled(false);
        m_svn14compat->hide();
    } else {
        m_svn14compat->setEnabled(true);
    }
    inChangeCompat=false;
}

void Createrepo_impl::fsTypeChanged(int which)
{
    m_DisableFsync->setEnabled(which==1);
    m_LogKeep->setEnabled(which==1);
}

QString Createrepo_impl::targetDir()
{
    KUrl u = m_ReposPathinput->url();
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

bool Createrepo_impl::compat13()const
{
    return (m_svn13compat->isChecked()||!m_svn13compat->isEnabled());
}

bool Createrepo_impl::compat14()const
{
    return (m_svn14compat->isChecked()||!m_svn14compat->isEnabled());
}

void Createrepo_impl::compatChanged14(bool)
{
    if (inChangeCompat) {
        return;
    }
    RecurseCheck rc(inChangeCompat);
    if (m_svn14compat->isChecked()) {
        m_svn13compat->setChecked(false);
    }
}

void Createrepo_impl::compatChanged13(bool)
{
    if (inChangeCompat) {
        return;
    }
    RecurseCheck rc(inChangeCompat);
    if (m_svn13compat->isChecked() && m_svn14compat->isEnabled()) {
        m_svn14compat->setChecked(false);
    }
}

#include "createrepo_impl.moc"
