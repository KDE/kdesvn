/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht   *
 *   ral@alwins-world.de   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "checkoutinfo_impl.h"
#include "rangeinput_impl.h"
#include <kurlrequester.h>
#include <qlabel.h>
#include <klineedit.h>
#include <qcheckbox.h>

CheckoutInfo_impl::CheckoutInfo_impl(QWidget *parent, const char *name)
    :CheckoutInfo(parent, name)
{
    m_RangeInput->setStartOnly(true);
    m_RangeInput->setHeadDefault();
}

CheckoutInfo_impl::~CheckoutInfo_impl()
{
}

svn::Revision CheckoutInfo_impl::toRevision()
{
    return m_RangeInput->getRange().first;
}

QString CheckoutInfo_impl::reposURL()
{
    return m_UrlEdit->text();
}

QString CheckoutInfo_impl::targetDir()
{
    return  m_TargetSelector->url();
}

bool CheckoutInfo_impl::forceIt()
{
    return m_forceButton->isChecked();
}

#include "checkoutinfo_impl.moc"
