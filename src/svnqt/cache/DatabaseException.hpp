/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   http://kdesvn.alwins-world.de/                                        *
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
#ifndef _DATABASE_EXCEPTION_HPP
#define _DATABASE_EXCEPTION_HPP

#include "svnqt/exception.hpp"

namespace svn
{
namespace cache
{

class SVNQT_EXPORT DatabaseException:public svn::Exception
{
    private:
        DatabaseException()throw();
        int m_number;

    public:
        DatabaseException(const QString&msg)throw()
            : Exception(msg),m_number(-1)
        {}

        DatabaseException(const DatabaseException&src)throw()
            : Exception(src.msg()),m_number(src.number())
        {}
        DatabaseException(const QString&msg,int aNumber)throw();
        virtual ~DatabaseException()throw(){}
        int number() const
        {
            return m_number;
        }
};

}
}
#endif
