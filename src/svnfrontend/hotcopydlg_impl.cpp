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
#include "hotcopydlg_impl.h"

#include <kurl.h>

HotcopyDlg_impl::HotcopyDlg_impl(QWidget *parent)
    : QWidget(parent), Ui::HotcopyDlg()
{
    setupUi(this);
    m_SrcpathEditor->setMode(KFile::Directory | KFile::LocalOnly);
    m_DestpathEditor->setMode(KFile::Directory | KFile::LocalOnly);
}

HotcopyDlg_impl::~HotcopyDlg_impl()
{
}

QString HotcopyDlg_impl::srcPath()const
{
    return m_SrcpathEditor->url().path(KUrl::RemoveTrailingSlash);
}

QString HotcopyDlg_impl::destPath()const
{
    return m_DestpathEditor->url().path(KUrl::RemoveTrailingSlash);
}

bool HotcopyDlg_impl::cleanLogs()const
{
    return m_Cleanlogs->isChecked();
}
