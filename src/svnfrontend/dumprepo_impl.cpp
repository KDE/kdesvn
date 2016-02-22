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

DumpRepo_impl::DumpRepo_impl(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    slotDumpRange(m_Rangeonly->isChecked());
}

void DumpRepo_impl::slotDumpRange(bool how)
{
    m_StartNumber->setEnabled(how);
    m_EndNumber->setEnabled(how);
    m_lblStart->setEnabled(how);
    m_lblEnd->setEnabled(how);
}

/*!
    \fn DumpRepo_impl::reposPath()
 */
QString DumpRepo_impl::reposPath() const
{
    return m_ReposPath->url().toLocalFile();
}

/*!
    \fn DumpRepo_impl::targetFile()
 */
QString DumpRepo_impl::targetFile() const
{
    return m_OutputFile->url().toLocalFile();
}

/*!
    \fn DumpRepo_impl::incremental()
 */
bool DumpRepo_impl::incremental() const
{
    return m_incrementalDump->isChecked();
}

/*!
    \fn DumpRepo_impl::use_dumps()
 */
bool DumpRepo_impl::use_deltas() const
{
    return m_UseDeltas->isChecked();
}

/*!
    \fn DumpRepo_impl::useNumbers()
 */
bool DumpRepo_impl::useNumbers() const
{
    return m_Rangeonly->isChecked();
}

/*!
    \fn DumpRepo_impl::startNumber() const
 */
int DumpRepo_impl::startNumber() const
{
    return useNumbers() ? m_StartNumber->value() : -1;
}

/*!
    \fn DumpRepo_impl::endNumber()
 */
int DumpRepo_impl::endNumber() const
{
    return useNumbers() ? m_EndNumber->value() : -1;
}
