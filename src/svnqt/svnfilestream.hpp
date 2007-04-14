/***************************************************************************
 *   Copyright (C) 2006-2007 by Rajko Albrecht                             *
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
#ifndef SVN_STREAMSVNFILESTREAM_HPP
#define SVN_STREAMSVNFILESTREAM_HPP

#include "svnstream.hpp"

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
    SvnFileOStream(const QString&fn,svn_client_ctx_t*ctx=0);

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
    SvnFileIStream(const QString&fn,svn_client_ctx_t*ctx=0);

    virtual ~SvnFileIStream();
    virtual bool isOk() const;
    virtual long read(char* data, const unsigned long max);

private:
    SvnFileStream_private*m_FileData;
};

}

}

#endif
