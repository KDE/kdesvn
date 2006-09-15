#ifndef SVNSVNSTREAM_HPP
#define SVNSVNSTREAM_HPP

#include "svnqt/svnqt_defines.hpp"

#include <qstring.h>

#include <svn_io.h>
struct svn_client_ctx_t;

namespace svn {

namespace stream {
class SvnStream_private;

/**
	@author Rajko Albrecht <ral@alwins-world.de>
    @short wrapper class around the svn_stream_t structure
*/
class SVNQT_EXPORT SvnStream{
    friend class SvnStream_private;
public:
    //! Constructor
    /*!
     * Setup a svn_stream_t and holds a required pool. The stream will freed
     * when deleting this object.
     * \param readit set readable
     * \param writeit set writable
     * \param ctx a client context for calls to cancel_func inside. you should this only set with functions not using it itself
     * like svn_client_cat2:
     */
    SvnStream(bool readit, bool writeit, svn_client_ctx_t * ctx = 0);
    //! frees all structures and releases memory pool.
    virtual ~SvnStream();

    //! operator returning transparent a svn_stream_t structure
    /*!
        \return a svn_stream_t structure for use with subversion api.
     */
    operator svn_stream_t* ()const;

    //! write operation
    /*!
        Write data FROM subversion to the class behind. Eg., data comes from
        subversion-api and this method has to do something with it (printing on a window, writing to a file)
        This implementation always returns -1 (eg, error), must reimplemented for real usage.
        \param data the data to written
        \param max maximum data to write
        \return should return the amount of data real written, in case of error must return -1
        \sa setError(int ioError), setError(const QString&error), read(char*data,const unsigned long max)
     */
    virtual long write(const char*data,const unsigned long max);
    //! read operation
    /*! implements the wrapper for svn_stream_read, eg. data are read FROM class (eg, file, string or whatever)
        into subversion-api. This implementation always returns -1 (eg, error), must reimplemented for real usage.
        \param data target array where to store the read
        \param max maximum byte count to read
        \return amount of data read or -1 in case of error
        \sa setError(int ioError), setError(const QString&error), write(const char*data,const unsigned long max)
    */
    virtual long read(char*data,const unsigned long max);

    //! returns the error set
    /*!
        \return a human readable message about the reason the last operation failed.
     */
    virtual const QString& lastError()const;
    //! is that stream usable
    /*!
        Gives information about if the stream object is usable. May if the file is real open or such.
        \return true if stream is usable, false if not.
     */
    virtual bool isOk()const = 0;

    svn_client_ctx_t * context();

protected:
    //! set a human readable errormessage
    /*!
        This message may printed to the user and will checked if one of the stream-operations failed. So should set from
        write and/or read if them will return -1 (for error)
        \param error the errormessage assigned.
     */
    virtual void setError(const QString&error)const;
    //! set the internal error
    /*! \param ioError error code from QIODevide::status
     */
#if QT_VERSION < 0x040000
    virtual void setError(int ioError)const;
#endif

protected:
    int cancelElapsed()const;
    void cancelTimeReset();

private:
    SvnStream_private*m_Data;
    /* disable default contructor */
    SvnStream();
};

class SvnByteStream_private;

//! a class let subversion print into a QByteArray
class SVNQT_EXPORT SvnByteStream:public SvnStream
{
public:
    //! constructor
    /*!
        creates internal buffer
     * \param ctx a client context for calls to cancel_func inside. you should this only set with functions not using it itself
     * like svn_client_cat2:
     */
    SvnByteStream(svn_client_ctx_t * ctx = 0);
    //! release internal buffer
    virtual ~SvnByteStream();
    //! fill internal buffer with data
    /*!
        stores the data written into the internal buffer.
        \param data data to store
        \param max length of data to store
        \return data real stored or -1 if error.
    */
    virtual long write(const char*data,const unsigned long max);

    //! return the data stored
    /*!
        \return the internal stored data
    */
    QByteArray content()const;
    //! checks if the buffer is usable.
    /*!
     * \return true if data may written, false if not, in that case a errormessage will set.
     */
    virtual bool isOk()const;

private:
    SvnByteStream_private*m_ByteData;
};

} // namespace stream

} // namespace svn

#endif
