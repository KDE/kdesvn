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
#include "editproperty_impl.h"
#include <ktextedit.h>
#include <klineedit.h>

EditProperty_impl::EditProperty_impl(QWidget *parent, const char *name)
    :EditPropsDlgData(parent, name)
{
}

void EditProperty_impl::slotHelp()
{
}


QString EditProperty_impl::PropName()const
{
    return m_NameEdit->text();
}

QString EditProperty_impl::PropValue()const
{
    return m_ValueEdit->text();
}

void EditProperty_impl::setPropName(const QString&n)
{
    m_NameEdit->setText(n);
}

void EditProperty_impl::setPropValue(const QString&v)
{
    m_ValueEdit->setText(v);
}

#include "editproperty_impl.moc"
