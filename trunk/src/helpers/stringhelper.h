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
#ifndef STRINGHELPER_H
#define STRINGHELPER_H

#include <qstring.h>

#include <kglobal.h>
#include <klocale.h>

namespace helpers
{

class ByteToString
{
protected:

    qulonglong m_value;
    QString m_text;

public:
    explicit ByteToString(qulonglong value)
    :m_value (value),m_text(KGlobal::locale()->formatByteSize(value))
    {
    };

    operator const QString&()const
    {
        return m_text;
    }
};

}

#endif
