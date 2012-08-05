/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
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
#include "dumprepo_impl.h"

#include <kdebug.h>
#include <kurl.h>
#include <kurlrequester.h>
#include <kcombobox.h>
#include <knuminput.h>

#include <qcheckbox.h>

DumpRepo_impl::DumpRepo_impl(QWidget *parent, const char *name)
//     :DumpRepoDlg(parent, name)
    : QWidget(parent)
{
    setupUi(this);
    setObjectName(name);
    m_ReposPath->setMode(KFile::Directory|KFile::LocalOnly);
    m_OutputFile->setMode(KFile::File|KFile::LocalOnly);
}

void DumpRepo_impl::slotDumpRange(bool how)
{
    m_StartNumber->setEnabled(how);
    m_EndNumber->setEnabled(how);
}


/*!
    \fn DumpRepo_impl::reposPath()
 */
QString DumpRepo_impl::reposPath()
{
    KUrl u = m_ReposPath->url();
    QString res = u.path();
    while (res.endsWith('/')) {
        res.truncate(res.length()-1);
    }
    return res;
}


/*!
    \fn DumpRepo_impl::targetFile()
 */
QString DumpRepo_impl::targetFile()
{
    KUrl u = m_OutputFile->url();
    QString res = u.path();
    while (res.endsWith('/')) {
        res.truncate(res.length()-1);
    }
    return res;
}


/*!
    \fn DumpRepo_impl::incremental()
 */
bool DumpRepo_impl::incremental()
{
    return m_incrementalDump->isChecked();
}


/*!
    \fn DumpRepo_impl::use_dumps()
 */
bool DumpRepo_impl::use_deltas()
{
    return m_UseDeltas->isChecked();
}


/*!
    \fn DumpRepo_impl::useNumbers()
 */
bool DumpRepo_impl::useNumbers()
{
    return m_Rangeonly->isChecked();
}

/*!
    \fn DumpRepo_impl::startNumber()
 */
int DumpRepo_impl::startNumber()
{
    return useNumbers()?m_StartNumber->value():-1;
}

/*!
    \fn DumpRepo_impl::endNumber()
 */
int DumpRepo_impl::endNumber()
{
    return useNumbers()?m_EndNumber->value():-1;
}

#include "dumprepo_impl.moc"
