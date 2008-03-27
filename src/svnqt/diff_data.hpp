/***************************************************************************
 *   Copyright (C) 2008 by Rajko Albrecht  ral@alwins-world.de             *
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

#ifndef __DIFF_DATA
#define __DIFF_DATA

#include "svnqt/svnqt_defines.hpp"
#include "svnqt/pool.hpp"
#include "path.hpp"

#include <qglobal.h>

#if QT_VERSION < 0x040000
    #include <qstring.h>
#else
    #include <QtCore>
#endif

struct apr_file_t;

namespace svn
{
    class Path;

    class SVNQT_NOEXPORT DiffOutput
    {
        protected:
            Pool m_Pool;
            Path m_tmpPath;
            apr_file_t*m_outFile;
            apr_file_t*m_errFile;
            const char*m_outFileName;
            const char*m_errFileName;

            void init();
            void clean();
            void close();

        public:
            DiffOutput(const Path&aTmpPath);
            virtual ~DiffOutput();

            apr_file_t*outFile(){return m_outFile;}
            apr_file_t*errFile(){return m_errFile;}

            QByteArray content();
    };
}

#endif
