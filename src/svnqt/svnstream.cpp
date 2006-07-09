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
#include "svncpp_defines.hpp"
#include "pool.hpp"
#include "apr.hpp"

#include <qbuffer.h>

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
    long res = b->isOk()?b->read(data,*len):-1;

    if (res<0) {
        *len = 0;
        return svn_error_create(SVN_ERR_MALFORMED_FILE,0L,b->lastError().TOUTF8());
    }
    *len = res;
    return SVN_NO_ERROR;
}

svn_error_t * SvnStream_private::stream_write(void*baton,const char*data,apr_size_t*len)
{
    SvnStream*b = (SvnStream*)baton;
    long res = b->isOk()?b->write(data,*len):-1;
    if (res<0) {
        *len = 0;
        return svn_error_create(SVN_ERR_MALFORMED_FILE,0L,b->lastError().utf8());
    }
    *len = res;
    return SVN_NO_ERROR;
}

SvnStream::SvnStream(bool read, bool write)
{
    m_Data = new SvnStream_private;
    m_Data->m_Stream = svn_stream_create(this,m_Data->m_Pool);
    if (read) {
        svn_stream_set_read(m_Data->m_Stream,SvnStream_private::stream_read);
    }
    if (write) {
        svn_stream_set_write(m_Data->m_Stream,SvnStream_private::stream_write);
    }
}

SvnStream::SvnStream()
{
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
    return -1;
}

long SvnStream::read(char*,const unsigned long )
{
    m_Data->m_LastError = "Read not supported with that stream";
    return -1;
}

const QString&SvnStream::lastError()const
{
    return m_Data->m_LastError;
}

void SvnStream::setError(const QString&aError)const
{
    m_Data->m_LastError = aError;
}

void SvnStream::setError(int ioError)const
{
    switch (ioError) {
    case IO_Ok:
        setError("Operation was successfull.");
        break;
    case IO_ReadError:
        setError("Could not read from device");
        break;
    case IO_WriteError:
        setError("Could not write to device");
        break;
    case IO_FatalError:
        setError("A fatal unrecoverable error occurred.");
        break;
    case IO_OpenError:
        setError("Could not open device or stream.");
        break;
    case IO_AbortError:
        setError("The operation was unexpectedly aborted.");
        break;
    case IO_TimeOutError:
        setError("The operation timed out.");
        break;
    case IO_UnspecifiedError:
        setError("An unspecified error happened on close.");
        break;
    default:
        setError("Unknown error happend.");
        break;
    }
}

class SvnByteStream_private {
public:
    SvnByteStream_private();
    virtual ~SvnByteStream_private(){}

    QByteArray m_Content;
    QBuffer mBuf;
};

SvnByteStream_private::SvnByteStream_private()
    :mBuf(m_Content)
{
    mBuf.open(IO_WriteOnly);
}

/* ByteStream implementation start */
SvnByteStream::SvnByteStream()
    : SvnStream(false,true)
{
    m_ByteData = new SvnByteStream_private;
    if (!m_ByteData->mBuf.isOpen()) {
        setError(m_ByteData->mBuf.status());
    }
}

SvnByteStream::~SvnByteStream()
{
    delete m_ByteData;
}

long SvnByteStream::write(const char*aData,const unsigned long max)
{
    long i = m_ByteData->mBuf.writeBlock(aData,max);
    if (i<0) {
        setError(m_ByteData->mBuf.status());
    }
    return i;
}

QByteArray SvnByteStream::content()const
{
    return m_ByteData->mBuf.buffer();
}

bool SvnByteStream::isOk()const
{
    return m_ByteData->mBuf.isOpen();
}

/* ByteStream implementation end */

} // namespace stream

} // namespace svn
