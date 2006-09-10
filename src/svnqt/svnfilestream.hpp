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
