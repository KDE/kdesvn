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
#ifndef STRINGHELPER_H
#define STRINGHELPER_H

#include <qstring.h>

namespace helpers
{

class ByteToString
{
protected:

public:
    ByteToString(){};

    QString operator()(long value)
    {
        char pre = 0;
        double v = (double)value;
        if (v<0) v=0;
        while (v>=1024.0 && pre != 'T')
        {
            switch (pre)
            {
            case 'k':
                pre = 'M';
                break;
            case 'M':
                pre = 'G';
                break;
            case 'G':
                pre = 'T';
                break;
            default:
                pre = 'k';
                break;
            }
            v /= 1024.0;
        }
        return QString("%1 %2Byte").arg(v,0,'f',pre?2:0).arg(pre?QString(QChar(pre)):QString(""));
    }
};

}

#endif
