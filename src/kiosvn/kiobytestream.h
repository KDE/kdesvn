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
#ifndef KIOBYTESTREAM_H
#define KIOBYTESTREAM_H

#include "src/svnqt/svnstream.hpp"

#include <kio/global.h>
#include <kmimetype.h>
#include <qbuffer.h>
#include <qdatetime.h>

class StreamWrittenCb
{
public:
    StreamWrittenCb(){}
    virtual ~StreamWrittenCb(){}
    virtual void streamWritten(const KIO::filesize_t current) = 0;
    virtual void streamPushData(QByteArray)=0;
    virtual void streamSendMime(KMimeType::Ptr)=0;
};

/**
	@author Rajko Albrecht
*/
class KioByteStream : public svn::stream::SvnStream
{
public:
    KioByteStream(StreamWrittenCb*,const QString&filename);

    ~KioByteStream();

    virtual bool isOk() const;
    virtual long write(const char* data, const unsigned long max);

    KIO::filesize_t written(){return m_Written;}

protected:
    StreamWrittenCb*m_Cb;
    KIO::filesize_t m_Written;
    bool m_mimeSend;
    QString m_Filename;
    QByteArray array;
    QTime m_MessageTick;
};

#endif
