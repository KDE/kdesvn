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
#include "kiobytestream.h"

KioByteStream::KioByteStream(StreamWrittenCb*aCb,const QString&filename)
    : svn::stream::SvnStream(false,true,0L),
    m_Cb(aCb),m_Written(0),
    m_mimeSend(false),m_Filename(filename)
{
    m_MessageTick.start();
}

KioByteStream::~KioByteStream()
{
}

bool KioByteStream::isOk() const
{
    return m_Cb != 0;
}

long KioByteStream::write(const char* data, const unsigned long max)
{
    bool forceInfo = !m_mimeSend;
    if (m_Cb) {
        if (!m_mimeSend) {
            m_mimeSend = true;
            array = QByteArray::fromRawData(data, max);
            KMimeType::Ptr result = KMimeType::findByNameAndContent(m_Filename,array);
            m_Cb->streamSendMime(result);
            array.clear();
        }
        array = QByteArray::fromRawData(data, max);
        m_Cb->streamPushData(array);
        array.clear();
        m_Written+=max;
        if (m_MessageTick.elapsed() >=100 || forceInfo) {
            m_Cb->streamWritten(m_Written);
            m_MessageTick.restart();
        }
        return max;
    }
    return -1;
}
