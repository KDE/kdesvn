/***************************************************************************
 *   Copyright (C) 2006 by Rajko Albrecht                                  *
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
#include "svnstream.hpp"
#include "pool.hpp"
#include "apr.hpp"

namespace svn {

namespace stream {
class SvnStream_private
{
public:
    SvnStream_private(){m_Stream=0;m_LastError="";}
    ~SvnStream_private(){}

    static svn_error_t * stream_write(void*baton,const char*data,apr_size_t*len);
    static svn_error_t * stream_read(void*baton,char*data,apr_size_t*len);

    Pool m_Pool;
    svn_stream_t * m_Stream;
    QString m_LastError;
};

svn_error_t * SvnStream_private::stream_read(void*baton,char*data,apr_size_t*len)
{
    SvnStream*b = (SvnStream*)baton;
    *len = b->read(data,*len);
    return SVN_NO_ERROR;
}

svn_error_t * SvnStream_private::stream_write(void*baton,const char*data,apr_size_t*len)
{
    SvnStream*b = (SvnStream*)baton;
    *len = b->write(data,*len);
    return SVN_NO_ERROR;
}

SvnStream::SvnStream()
{
    m_Data = new SvnStream_private;
    m_Data->m_Stream = svn_stream_create(this,m_Data->m_Pool);
    svn_stream_set_read(m_Data->m_Stream,SvnStream_private::stream_read);
    svn_stream_set_write(m_Data->m_Stream,SvnStream_private::stream_write);
}


SvnStream::~SvnStream()
{
    delete m_Data;
}

SvnStream::operator svn_stream_t* ()const
{
    return m_Data->m_Stream;
}

long SvnStream::write(const char*,const unsigned long)
{
    m_Data->m_LastError = "Write not supported with that stream";
    return 0;
}

long SvnStream::read(char*,const unsigned long )
{
    m_Data->m_LastError = "Read not supported with that stream";
    return 0;
}

const QString&SvnStream::lastError()const
{
    return m_Data->m_LastError;
}

}

}
