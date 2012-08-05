/***************************************************************************
 *   Copyright (C) 2007-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   http://kdesvn.alwins-world.de/                                        *
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
 * history and logs, available at http://kdesvn.alwins-world.de.           *
 ***************************************************************************/

#ifndef DIFF_OPTIONS_HPP
#define DIFF_OPTIONS_HPP

#include "svnqt/svnqt_defines.h"

struct svn_diff_file_options_t;
class QStringList;

namespace svn
{
    class Pool;
    struct DiffOptionsData;
    /** c++ wrapper for svn_diffoptions_t
     *
     * This is needed until svnqt stops support for subversion prior 1.4
     */
    class SVNQT_EXPORT DiffOptions
    {
        public:
            enum IgnoreSpace {
                IgnoreSpaceNone,
                IgnoreSpaceChange,
                IgnoreSpaceAll
            };
        protected:
            DiffOptionsData* m_data;
            void init(const svn_diff_file_options_t*options);

        public:
            DiffOptions();
            /** Initialize options with values depending on options.
             * Supported types are:
             * - --ignore-space-change, -b
             * - --ignore-all-space, -w
             * - --ignore-eol-style
             * - --unified, -u (for compatibility, does nothing).
             * @sa svn_diff_file_options_parse
             */
            DiffOptions(const QStringList&options);

            /** Initialize options with values depending on options.
             * Only if build against subversion 1.4 or newer.
             */
            DiffOptions(const svn_diff_file_options_t*options);

            /** copy operator
             */
            DiffOptions(const DiffOptions&old);

            ~DiffOptions();

            svn_diff_file_options_t*options(const Pool&pool)const;
    };
}

#endif
