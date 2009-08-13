/***************************************************************************
 *   Copyright (C) 2008-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   http://kdesvn.alwins-world.de/                                        *
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

#ifndef DIFF_DATA_HPP
#define DIFF_DATA_HPP

#include "svnqt/svnqt_defines.hpp"
#include "svnqt/pool.hpp"
#include "path.hpp"
#include "revision.hpp"

#include <qglobal.h>

#include <QtCore>

struct apr_file_t;

namespace svn
{
    class Path;

    class SVNQT_NOEXPORT DiffData
    {
        protected:
            Pool m_Pool;
            Path m_tmpPath;
            apr_file_t*m_outFile;
            apr_file_t*m_errFile;
            const char*m_outFileName;
            const char*m_errFileName;

            Path m_p1,m_p2;
            Revision m_r1,m_r2;

            bool m_working_copy_present,m_url_is_present;

            void init();
            void clean();
            void close();

        public:
            DiffData(const Path&aTmpPath,const Path&,const Revision&,const Path&,const Revision&);
            virtual ~DiffData();

            apr_file_t*outFile(){return m_outFile;}
            apr_file_t*errFile(){return m_errFile;}
            const Revision& r1()const{return m_r1;}
            const Revision& r2()const{return m_r2;}

            QByteArray content();
    };
}

#endif
