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
class SvnFileStream : public SvnStream
{
public:
    SvnFileStream(const QString&fn);

    virtual ~SvnFileStream();

    virtual bool isOk() const;
    virtual long write(const char* data, const unsigned long max);
private:
    SvnFileStream_private*m_FileData;
};

}

}

#endif
