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
 * history and logs, available at https://commits.kde.org/kdesvn.          *
 ***************************************************************************/
#include "svnstream.h"

#include "pool.h"
#include "apr.h"

// Subversion api
#include <svn_client.h>

#include <QBuffer>
#include <QElapsedTimer>

namespace svn
{

namespace stream
{
class SVNQT_NOEXPORT SvnStream_private
{
public:
    SvnStream_private()
    {
        m_Stream = nullptr;
        _context = nullptr;/*cancel_timeout.start();*/
    }
    ~SvnStream_private()
    {
        /*qDebug("Time elapsed: %i ",cancel_timeout.elapsed());*/
    }

    static svn_error_t *stream_write(void *baton, const char *data, apr_size_t *len);
    static svn_error_t *stream_read(void *baton, char *data, apr_size_t *len);

    Pool m_Pool;
    svn_stream_t *m_Stream;
    QString m_LastError;

    svn_client_ctx_t *_context;
    QElapsedTimer cancel_timeout;
};

svn_error_t *SvnStream_private::stream_read(void *baton, char *data, apr_size_t *len)
{
    SvnStream *b = (SvnStream *)baton;
    svn_client_ctx_t *ctx = b->context();

    if (ctx && ctx->cancel_func) {
        SVN_ERR(ctx->cancel_func(ctx->cancel_baton));
    }

    long res = b->isOk() ? b->read(data, *len) : -1;

    if (res < 0) {
        *len = 0;
        return svn_error_create(SVN_ERR_MALFORMED_FILE, nullptr, b->lastError().toUtf8());
    }
    *len = res;
    return SVN_NO_ERROR;
}

svn_error_t *SvnStream_private::stream_write(void *baton, const char *data, apr_size_t *len)
{
    SvnStream *b = (SvnStream *)baton;
    svn_client_ctx_t *ctx = b->context();

    if (ctx && ctx->cancel_func && b->cancelElapsed() > 50) {
        //qDebug("Check cancel");
        SVN_ERR(ctx->cancel_func(ctx->cancel_baton));
        b->cancelTimeReset();
    }

    long res = b->isOk() ? b->write(data, *len) : -1;
    if (res < 0) {
        *len = 0;
        return svn_error_create(SVN_ERR_MALFORMED_FILE, nullptr, b->lastError().toUtf8());
    }
    *len = res;
    return SVN_NO_ERROR;
}

SvnStream::SvnStream(bool read, bool write, svn_client_ctx_t *ctx)
{
    m_Data = new SvnStream_private;
    m_Data->m_Stream = svn_stream_create(this, m_Data->m_Pool);
    m_Data->_context = ctx;
    if (read) {
        svn_stream_set_read(m_Data->m_Stream, SvnStream_private::stream_read);
    }
    if (write) {
        svn_stream_set_write(m_Data->m_Stream, SvnStream_private::stream_write);
    }
}

SvnStream::~SvnStream()
{
    delete m_Data;
}

int SvnStream::cancelElapsed()const
{
    return m_Data->cancel_timeout.elapsed();
}

void SvnStream::cancelTimeReset()
{
    m_Data->cancel_timeout.restart();
}

SvnStream::operator svn_stream_t *()const
{
    return m_Data->m_Stream;
}

svn_client_ctx_t *SvnStream::context()
{
    return m_Data->_context;
}

long SvnStream::write(const char *, const unsigned long)
{
    m_Data->m_LastError = QLatin1String("Write not supported with that stream");
    return -1;
}

long SvnStream::read(char *, const unsigned long)
{
    m_Data->m_LastError = QLatin1String("Read not supported with that stream");
    return -1;
}

const QString &SvnStream::lastError()const
{
    return m_Data->m_LastError;
}

void SvnStream::setError(const QString &aError)const
{
    m_Data->m_LastError = aError;
}

/* ByteStream implementation start */
SvnByteStream::SvnByteStream(svn_client_ctx_t *ctx)
    : SvnStream(false, true, ctx)
    , m_ByteData(new QBuffer)
{
    m_ByteData->open(QIODevice::ReadWrite);
    if (!m_ByteData->isOpen()) {
        setError(m_ByteData->errorString());
    }
}

SvnByteStream::~SvnByteStream()
{
    delete m_ByteData;
}

long SvnByteStream::write(const char *aData, const unsigned long max)
{
    qint64 i = m_ByteData->write(aData, max);
    if (i < 0) {
        setError(m_ByteData->errorString());
    }
    return i;
}

QByteArray SvnByteStream::content()const
{
    return m_ByteData->buffer();
}

bool SvnByteStream::isOk()const
{
    return m_ByteData->isOpen();
}

/* ByteStream implementation end */

} // namespace stream

} // namespace svn
