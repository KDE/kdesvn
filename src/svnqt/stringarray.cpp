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
 * history and logs, available at https://commits.kde.org/kdesvn.          *
 ***************************************************************************/

#include "stringarray.h"
#include "pool.h"

#include <svn_types.h>
// apr api
#include <apr_pools.h>
#include <apr_strings.h>

/*!
    \fn svn::StringArray::StringArray()
 */
svn::StringArray::StringArray()
    : m_content()
{
    setNull(true);
}

/*!
    \fn svn::StringArray::StringArray(const QStringList&)
 */
svn::StringArray::StringArray(const QStringList &aList)
    : m_content(aList)
{
    setNull(m_content.isEmpty());
}

/*!
    \fn svn::StringArray::StringArray(const apr_array_header_t * apr_targets)
 */
svn::StringArray::StringArray(const apr_array_header_t *apr_targets)
    : m_content()
{
    for (int i = 0; i < apr_targets->nelts; i++) {
        const char **target =
            &APR_ARRAY_IDX(apr_targets, i, const char *);

        m_content.push_back(QString::fromUtf8(*target));
    }
    setNull(m_content.isEmpty());
}

/*!
    \fn svn::StringArray::size()const
 */
QStringList::size_type svn::StringArray::size()const
{
    if (isNull()) {
        return 0;
    }
    return m_content.size();
}

/*!
    \fn svn::StringArray::array (const Pool & pool) const
 */
apr_array_header_t *svn::StringArray::array(const Pool &pool) const
{
    if (isNull()) {
        return nullptr;
    }
    apr_pool_t *apr_pool = pool.pool();
    apr_array_header_t *apr_targets =
        apr_array_make(apr_pool, m_content.size(), sizeof(const char *));

    for (const QString &content : m_content) {
        const QByteArray s = content.toUtf8();
        char *t2 = apr_pstrndup(apr_pool, s.data(), s.size());
        (*((const char **) apr_array_push(apr_targets))) = t2;
    }
    return apr_targets;
}

bool svn::StringArray::isNull()const
{
    return m_isNull;
}

void svn::StringArray::setNull(bool _n)
{
    if (_n) {
        m_content.clear();
    }
    m_isNull = _n;
}
