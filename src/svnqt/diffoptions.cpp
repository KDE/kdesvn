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
#include "stringarray.h"
#include "pool.h"

#include <svn_diff.h>
#ifdef HAS_SVN_VERSION_H
#include <svn_version.h>
#endif

namespace svn
{
    struct DiffOptionsData
    {
        DiffOptions::IgnoreSpace _ignorespace;
        bool _ignoreeol;
        bool _showc;

        DiffOptionsData()
        {
            _ignorespace=DiffOptions::IgnoreSpaceNone;
            _ignoreeol = _showc = false;
        }
    };

    DiffOptions::DiffOptions()
    {
        m_data = new DiffOptionsData;
    }

    void DiffOptions::init(const svn_diff_file_options_t*_diffopts)
    {
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 4)) || (SVN_VER_MAJOR > 1)
        m_data->_ignoreeol = _diffopts->ignore_eol_style;

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
        m_data->_showc = _diffopts->show_c_function;
#else
        m_data->_showc = false;
#endif
        switch (_diffopts->ignore_space) {
            case svn_diff_file_ignore_space_change:
                m_data->_ignorespace = IgnoreSpaceChange;
                break;
            case svn_diff_file_ignore_space_all:
                m_data->_ignorespace = IgnoreSpaceAll;
                break;
            case svn_diff_file_ignore_space_none:
            default:
                m_data->_ignorespace = IgnoreSpaceNone;
                break;
        }
#else
        Q_UNUSED(_diffopts);
#endif
    }

    DiffOptions::DiffOptions(const QStringList&options)
    {
        m_data = new DiffOptionsData;
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 4)) || (SVN_VER_MAJOR > 1)
        Pool pool;
        StringArray _ar(options);
        svn_diff_file_options_t * _diffopts =  svn_diff_file_options_create(pool);
        if (_diffopts) {
            svn_error_t*error = svn_diff_file_options_parse(_diffopts,_ar.array(pool),pool);
            if (error==0) {
                init(_diffopts);
            }
        }
#else
        Q_UNUSED(options)
#endif
    }

    DiffOptions::DiffOptions(const svn_diff_file_options_t*options)
    {
        m_data = new DiffOptionsData;
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 4)) || (SVN_VER_MAJOR > 1)
        if (options) {
            init(options);
        }
#else
        Q_UNUSED(options)
#endif
    }

    svn_diff_file_options_t*DiffOptions::options(const Pool&pool)const
    {
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 4)) || (SVN_VER_MAJOR > 1)
        svn_diff_file_options_t * _diffopts =  svn_diff_file_options_create(pool);
        _diffopts->ignore_eol_style = m_data->_ignoreeol;
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
        _diffopts->show_c_function = m_data->_showc;
#endif
        switch (m_data->_ignorespace) {
            case IgnoreSpaceChange:
                _diffopts->ignore_space=svn_diff_file_ignore_space_change;
                break;
            case IgnoreSpaceAll:
                _diffopts->ignore_space=svn_diff_file_ignore_space_all;
                break;
            case IgnoreSpaceNone:
            default:
                _diffopts->ignore_space=svn_diff_file_ignore_space_none;
                break;
        }
        return _diffopts;
#else
        Q_UNUSED(pool);
        return 0;
#endif
    }

    DiffOptions::DiffOptions(const DiffOptions&old)
    {
        m_data = new DiffOptionsData;
        m_data->_showc = old.m_data->_showc;
        m_data->_ignorespace=old.m_data->_ignorespace;
        m_data->_ignoreeol=old.m_data->_ignoreeol;
    }

    DiffOptions::~DiffOptions()
    {
        delete m_data;
        m_data=0;
    }
}
