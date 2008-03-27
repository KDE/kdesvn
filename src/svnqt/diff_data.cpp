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
#include "diff_data.hpp"
#include "svnqt/svnqt_defines.hpp"
#include "svnqt/exception.hpp"

#include <qfile.h>

#include <svn_version.h>
#include <svn_io.h>

namespace svn
{
    DiffOutput::DiffOutput(const Path&aTmpPath)
        :m_Pool(),m_tmpPath(aTmpPath),
        m_outFile(0),m_errFile(0),m_outFileName(0),m_errFileName(0)
    {
        init();
    }

    void DiffOutput::init()
    {
        svn_error_t * error;
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 4)
        error = svn_io_open_unique_file2(&m_outFile, &m_outFileName,
                        m_tmpPath.path().TOUTF8(),
                        ".tmp",
                        svn_io_file_del_on_pool_cleanup, m_Pool);
#else
        error = svn_io_open_unique_file (&m_outFile, &m_outFileName,
                        m_tmpPath.path().TOUTF8(),
                        ".tmp",
                        FALSE, m_Pool);
#endif
        if (error!=0) {
            clean();
            throw ClientException (error);
        }
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 4)
        error = svn_io_open_unique_file2(&m_errFile, &m_errFileName,
                        m_tmpPath.path().TOUTF8(),
                        ".tmp",
                        svn_io_file_del_on_pool_cleanup, m_Pool);
#else
        error = svn_io_open_unique_file (&m_errFile, &m_errFileName,
                        m_tmpPath.path().TOUTF8(),
                        ".tmp",
                        FALSE, m_Pool);
#endif
        if (error!=0) {
            clean();
            throw ClientException (error);
        }
    }

    DiffOutput::~DiffOutput()
    {
        clean();
    }

    void DiffOutput::clean()
    {
        close();
#if !((SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 4))
        if (m_outFileName != 0) {
            svn_esrror_clear (svn_io_remove_file (m_outFileName, m_Pool));
            m_outFileName=0;
        }
        if (m_errFileName != 0) {
            svn_error_clear (svn_io_remove_file (m_errFileName, m_Pool));
            m_errFileName=0;
        }
#endif
    }

    void DiffOutput::close()
    {
        if (m_outFile != 0) {
            svn_io_file_close(m_outFile,m_Pool);
            m_outFile=0;
        }
        if (m_errFile != 0) {
            svn_io_file_close(m_errFile,m_Pool);
            m_errFile=0;
        }
    }

    QByteArray DiffOutput::content()
    {
        if (!m_outFileName) {
            return QByteArray();
        }
        close();
        QFile fi(m_outFileName);
#if QT_VERSION < 0x040000
        if (!fi.open(IO_ReadOnly|IO_Raw)) {
#else
        if (!fi.open(QIODevice::ReadOnly)) {
#endif
            throw ClientException(QString("%1 '%2'").arg(fi.errorString()).arg(m_outFileName));
        }

        QByteArray res = fi.readAll();
        fi.close();
        return res;
    }
}
