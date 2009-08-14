/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this program (in the file LGPL.txt); if not,         *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at http://kdesvn.alwins-world.de.           *
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
