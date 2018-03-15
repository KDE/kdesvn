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

#include "diffoptions.h"
#include "svnqt_defines.h"
#include "stringarray.h"
#include "pool.h"

#include <svn_diff.h>
#include <svn_version.h>

namespace svn
{
void DiffOptions::init(const svn_diff_file_options_t *_diffopts)
{
    _ignoreeol = _diffopts->ignore_eol_style;
    _showc = _diffopts->show_c_function;
    switch (_diffopts->ignore_space) {
    case svn_diff_file_ignore_space_change:
        _ignorespace = IgnoreSpaceChange;
        break;
    case svn_diff_file_ignore_space_all:
        _ignorespace = IgnoreSpaceAll;
        break;
    case svn_diff_file_ignore_space_none:
    default:
        _ignorespace = IgnoreSpaceNone;
        break;
    }
}

DiffOptions::DiffOptions(const QStringList &options)
{
    Pool pool;
    StringArray _ar(options);
    svn_diff_file_options_t *_diffopts =  svn_diff_file_options_create(pool);
    if (_diffopts) {
        svn_error_t *error = svn_diff_file_options_parse(_diffopts, _ar.array(pool), pool);
        if (error == nullptr) {
            init(_diffopts);
        }
    }
}

DiffOptions::DiffOptions(const svn_diff_file_options_t *options)
{
    if (options) {
        init(options);
    }
}

svn_diff_file_options_t *DiffOptions::options(const Pool &pool)const
{
    svn_diff_file_options_t *_diffopts =  svn_diff_file_options_create(pool);
    _diffopts->ignore_eol_style = _ignoreeol;
    _diffopts->show_c_function = _showc;
    switch (_ignorespace) {
    case IgnoreSpaceChange:
        _diffopts->ignore_space = svn_diff_file_ignore_space_change;
        break;
    case IgnoreSpaceAll:
        _diffopts->ignore_space = svn_diff_file_ignore_space_all;
        break;
    case IgnoreSpaceNone:
    default:
        _diffopts->ignore_space = svn_diff_file_ignore_space_none;
        break;
    }
    return _diffopts;
}

}
