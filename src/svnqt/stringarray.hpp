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

#ifndef _STRING_ARRAY_HPP
#define _STRING_ARRAY_HPP

#include "svnqt/svnqt_defines.hpp"
#include "svnqt/svnqttypes.hpp"

#include <qglobal.h>
#include <QtCore>
#include <QStringList>

// apr api
#include "apr_tables.h"

namespace svn
{
    // forward declarations
    class Pool;

    /** Handle array of const char * in a c++ like way */
    class SVNQT_EXPORT StringArray
    {
        protected:
            QStringList m_content;
            bool m_isNull;

        public:
            StringArray();
            StringArray(const QStringList&);
            StringArray(const apr_array_header_t * apr_targets);
            QStringList::size_type size()const;
            const QString& operator[](QStringList::size_type which)const;
            QString& operator[](QStringList::size_type which);
            /**
             * Returns an apr array containing char*.
             *
             * @param pool Pool used for conversion
             */
            apr_array_header_t * array (const Pool & pool) const;
            /** content of array
             * @return const reference to data, may used for searches.
             */
            const QStringList& data() const {return m_content;}

            /** if array should return 0 instead of empty array */
            bool isNull()const;
            void setNull(bool _n);
    };
}

#endif
