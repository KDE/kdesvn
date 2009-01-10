/***************************************************************************
 *   Copyright (C) 2006-2008 by Rajko Albrecht                             *
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

#include "svnqt/stringarray.hpp"
#include "svnqt/pool.hpp"

#include <svn_types.h>
// apr api
#include <apr_pools.h>
#include <apr_strings.h>

/*!
    \fn svn::StringArray::StringArray()
 */
 svn::StringArray::StringArray()
    :m_content()
{
    setNull(true);
}


/*!
    \fn svn::StringArray::StringArray(const QStringList&)
 */
svn::StringArray::StringArray(const QStringList&aList)
    :m_content(aList)
{
    setNull(false);
}

/*!
    \fn svn::StringArray::StringArray(const apr_array_header_t * apr_targets)
 */
svn::StringArray::StringArray(const apr_array_header_t * apr_targets)
    :m_content()
{
    int i;
    for (i = 0; i < apr_targets->nelts; i++)
    {
        const char ** target =
                &APR_ARRAY_IDX (apr_targets, i, const char *);

        m_content.push_back (QString::FROMUTF8(*target));
    }
}


/*!
    \fn svn::StringArray::size()const
 */
size_t svn::StringArray::size()const
{
    if (isNull()) {
        return 0;
    }
    return m_content.size ();
}


/*!
    \fn svn::StringArray::operator[](size_t which)
 */
const QString& svn::StringArray::operator[](size_t which)
{
    return m_content[which];
}


/*!
    \fn svn::StringArray::array (const Pool & pool) const
 */
apr_array_header_t * svn::StringArray::array (const Pool & pool) const
{
    apr_pool_t *apr_pool = pool.pool();
    apr_array_header_t *apr_targets;
    if (isNull()) {
        apr_targets = apr_array_make(apr_pool, 0, 0);
    } else {
        QStringList::const_iterator it;

        apr_targets =
                apr_array_make (apr_pool,m_content.size(),sizeof (const char *));

        for (it = m_content.begin (); it != m_content.end (); it++)
        {
            QByteArray s = (*it).TOUTF8();
            char * t2 = apr_pstrndup (apr_pool,s,s.size());

            (*((const char **) apr_array_push (apr_targets))) = t2;
        }
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
