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
#include "svnfilestream.hpp"

#include <qfile.h>

namespace svn {

namespace stream {

class SvnFileStream_private
{
public:
    SvnFileStream_private(const QString&fn);
    virtual ~SvnFileStream_private();

    QString m_FileName;
    QFile m_File;
};

SvnFileStream_private::SvnFileStream_private(const QString&fn)
    : m_FileName(fn),m_File(fn)
{
    m_File.open(IO_WriteOnly);
}

SvnFileStream_private::~SvnFileStream_private()
{
}

SvnFileStream::SvnFileStream(const QString&fn)
    :SvnStream()
{
    m_FileData = new SvnFileStream_private(fn);
    if (!m_FileData->m_File.isOpen()) {
        setError(m_FileData->m_File.errorString());
    }
}


SvnFileStream::~SvnFileStream()
{
    delete m_FileData;
}


bool SvnFileStream::isOk() const
{
    return m_FileData->m_File.isOpen();
}

long SvnFileStream::write(const char* data, const unsigned long max)
{
    if (!m_FileData->m_File.isOpen()) {
        return -1;
    }
    long res = m_FileData->m_File.writeBlock(data,max);
    if (res<0) {
        setError(m_FileData->m_File.errorString());
    }
    return res;
}

}

}
