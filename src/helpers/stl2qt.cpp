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

#include "stl2qt.h"

namespace helpers {
stl2qt::stl2qt()
{
}

stl2qt::~stl2qt()
{
}

QString stl2qt::stl2qtstring_(const std::string&what)
{
    return QString::fromUtf8(what.c_str());
}

QString stl2qt::stl2qtstring_(const char*what)
{
    return what?QString::fromUtf8(what):QString("");
}

std::string stl2qt::qt2stlstring_(const QString&what)
{
    return std::string(what.isEmpty()?"":what.utf8());
}

}
