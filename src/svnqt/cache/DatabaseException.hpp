#ifndef _DATABASE_EXCEPTION_HPP
#define _DATABASE_EXCEPTION_HPP

#include "svnqt/exception.hpp"

namespace svn
{
namespace cache
{

class DatabaseException:public svn::Exception
{
    private:
        DatabaseException()throw();
    public:
        DatabaseException(const QString&msg)throw()
            : Exception(msg)
        {}
        DatabaseException(const DatabaseException&src)throw()
            : Exception(src.msg())
        {}
};

}
}
#endif
