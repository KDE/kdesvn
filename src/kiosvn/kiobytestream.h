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
#ifndef KIOBYTESTREAM_H
#define KIOBYTESTREAM_H

#include "svnqt/svnstream.h"

#include <kio/global.h>
#include <QMimeType>
#include <QElapsedTimer>

class StreamWrittenCb
{
public:
    StreamWrittenCb() = default;
    virtual ~StreamWrittenCb() = default;
    virtual void streamWritten(const KIO::filesize_t current) = 0;
    virtual void streamPushData(const QByteArray &streamData) = 0;
    virtual void streamSendMime(const QMimeType &mt) = 0;
};

/**
    @author Rajko Albrecht
*/
class KioByteStream : public svn::stream::SvnStream
{
public:
    KioByteStream(StreamWrittenCb *, const QString &filename);

    ~KioByteStream();

    bool isOk() const override;
    long write(const char *data, const unsigned long max) override;

    KIO::filesize_t written()
    {
        return m_Written;
    }

protected:
    StreamWrittenCb *m_Cb;
    KIO::filesize_t m_Written;
    bool m_mimeSend;
    QString m_Filename;
    QByteArray array;
    QElapsedTimer m_MessageTick;
};

#endif
