#ifndef SVNSVNSTREAM_HPP
#define SVNSVNSTREAM_HPP

#include <qstring.h>

#include <svn_io.h>

namespace svn {

namespace stream {
class SvnStream_private;
/**
	@author Rajko Albrecht <ral@alwins-world.de>
*/
class SvnStream{
public:
    SvnStream();
    virtual ~SvnStream();

    operator svn_stream_t* ()const;

    virtual long write(const char*data,const unsigned long max);
    virtual long read(char*data,const unsigned long max);

    virtual const QString& lastError()const;

private:
    SvnStream_private*m_Data;
};

}

}

#endif
