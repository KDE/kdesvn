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
#ifndef SVN_STREAMSVNFILESTREAM_HPP
#define SVN_STREAMSVNFILESTREAM_HPP

#include "svnstream.h"

namespace svn {

namespace stream {

class SvnFileStream_private;

/**
	@author Rajko Albrecht <ral@alwins-world.de>
    @short Writeonly filestream
*/
class SVNQT_EXPORT SvnFileOStream : public SvnStream
{
public:
    explicit SvnFileOStream(const QString&fn,svn_client_ctx_t*ctx=0);

    virtual ~SvnFileOStream();

    virtual bool isOk() const;
    virtual long write(const char* data, const unsigned long max);
private:
    SvnFileStream_private*m_FileData;
};

/**
    @author Rajko Albrecht <ral@alwins-world.de>
    @short Readonly filestream
*/
class SVNQT_EXPORT SvnFileIStream : public SvnStream
{
public:
    explicit SvnFileIStream(const QString&fn,svn_client_ctx_t*ctx=0);

    virtual ~SvnFileIStream();
    virtual bool isOk() const;
    virtual long read(char* data, const unsigned long max);

private:
    SvnFileStream_private*m_FileData;
};

}

}

#endif
