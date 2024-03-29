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
#include "loaddmpdlg_impl.h"

LoadDmpDlg_impl::LoadDmpDlg_impl(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    m_Dumpfile->setMode(KFile::File);
    m_Repository->setMode(KFile::Directory | KFile::LocalOnly);
}

LoadDmpDlg_impl::~LoadDmpDlg_impl()
{
}

/*!
    \fn LoadDmpDlg_impl::usePost()const
 */
bool LoadDmpDlg_impl::usePost() const
{
    return m_UsePost->isChecked();
}

/*!
    \fn LoadDmpDlg_impl::usePre()const
 */
bool LoadDmpDlg_impl::usePre() const
{
    return m_UsePre->isChecked();
}

bool LoadDmpDlg_impl::validateProps() const
{
    return m_validateProps->isChecked();
}

/*!
    \fn LoadDmpDlg_impl::uuidAction()const
 */
int LoadDmpDlg_impl::uuidAction() const
{
    if (m_UUidForce->isChecked()) {
        return 2;
    } else if (m_UUidIgnore->isChecked()) {
        return 1;
    }
    return 0;
}

/*!
    \fn LoadDmpDlg_impl::dumpFile()const
 */
QUrl LoadDmpDlg_impl::dumpFile() const
{
    return m_Dumpfile->url();
}

/*!
    \fn LoadDmpDlg_impl::repository()const
 */
QString LoadDmpDlg_impl::repository() const
{
    return m_Repository->url().toLocalFile();
}

/*!
    \fn LoadDmpDlg_impl::parentPath()const
 */
QString LoadDmpDlg_impl::parentPath() const
{
    QString res = m_Rootfolder->text();
    while (res.endsWith(QLatin1Char('/'))) {
        res.chop(1);
    }
    return res;
}

#include "moc_loaddmpdlg_impl.cpp"
