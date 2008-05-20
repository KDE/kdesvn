#include "DatabaseException.hpp"

/*!
    \fn svn::cache::DatabaseException::DatabaseException(const QString&msg,int aNumber)throw()
 */
svn::cache::DatabaseException::DatabaseException(const QString&msg,int aNumber)throw()
    : Exception(msg),m_number(aNumber)
{
    if (aNumber>-1) {
        setMessage(QString("(Code %1) %2").arg(aNumber).arg(msg));
    }
}
