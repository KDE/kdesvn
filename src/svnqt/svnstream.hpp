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

    virtual bool isOk()const = 0;

protected:
    virtual void setError(const QString&)const;
    /** set the internal error
     * @param ioError error code from QIODevide::status
     */
    virtual void setError(int ioError)const;

private:
    SvnStream_private*m_Data;
};

class SvnByteStream_private;

class SvnByteStream:public SvnStream
{
public:
    SvnByteStream();
    virtual ~SvnByteStream();

    virtual long write(const char*,const unsigned long);

    QByteArray content()const;

    virtual bool isOk()const;

private:
    SvnByteStream_private*m_ByteData;
};

} // namespace stream

} // namespace svn

#endif
