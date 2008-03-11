#ifndef _DATABASE_EXCEPTION_HPP
#define _DATABASE_EXCEPTION_HPP

#include "svnqt/exception.hpp"

namespace svn
{
namespace cache
{

class SVNQT_EXPORT DatabaseException:public svn::Exception
{
    private:
        DatabaseException()throw();
        int m_number;

    public:
        DatabaseException(const QString&msg)throw()
            : Exception(msg),m_number(-1)
        {}

        DatabaseException(const DatabaseException&src)throw()
            : Exception(src.msg()),m_number(src.number())
        {}
        DatabaseException(const QString&msg,int aNumber)throw();
        virtual ~DatabaseException()throw(){}
        int number() const
        {
            return m_number;
        }
};

}
}
#endif
