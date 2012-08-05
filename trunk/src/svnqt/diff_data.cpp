/***************************************************************************
 *   Copyright (C) 2008-2009 by Rajko Albrecht  ral@alwins-world.de        *
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
#include "diff_data.h"
#include "svnqt/svnqt_defines.h"
#include "svnqt/exception.h"

#include <qfile.h>

#include <svn_version.h>
#include <svn_io.h>
#include <svn_path.h>

namespace svn
{
    DiffData::DiffData(const Path&aTmpPath,const Path&_p1,const Revision&_r1,const Path&_p2,const Revision&_r2)
        :m_Pool(),m_tmpPath(aTmpPath),
        m_outFile(0),m_errFile(0),m_outFileName(0),m_errFileName(0),
        m_p1(_p1),m_p2(_p2),m_r1(_r1),m_r2(_r2),
        m_working_copy_present(false),m_url_is_present(false)
    {
        init();
    }

    void DiffData::init()
    {
        svn_error_t * error;
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 6)
        Pool scratchPool;
        error = svn_io_open_unique_file3(&m_outFile, &m_outFileName,
                        m_tmpPath.path().TOUTF8(),
                        svn_io_file_del_on_pool_cleanup, m_Pool,scratchPool);

#elif (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 4)
        error = svn_io_open_unique_file2(&m_outFile, &m_outFileName,
                        m_tmpPath.path().TOUTF8(),
                        ".tmp",
                        svn_io_file_del_on_pool_cleanup, m_Pool);
#else
        error = svn_io_open_unique_file (&m_outFile, &m_outFileName,
                        m_tmpPath.path().TOUTF8(),
                        ".tmp",
                        false, m_Pool);
#endif
        if (error!=0) {
            clean();
            throw ClientException (error);
        }
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 6)
        error = svn_io_open_unique_file3(&m_errFile, &m_errFileName,
                        m_tmpPath.path().TOUTF8(),
                        svn_io_file_del_on_pool_cleanup, m_Pool,scratchPool);
#elif (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 4)
        error = svn_io_open_unique_file2(&m_errFile, &m_errFileName,
                        m_tmpPath.path().TOUTF8(),
                        ".tmp",
                        svn_io_file_del_on_pool_cleanup, m_Pool);
#else
        error = svn_io_open_unique_file (&m_errFile, &m_errFileName,
                        m_tmpPath.path().TOUTF8(),
                        ".tmp",
                        false, m_Pool);
#endif
        if (error!=0) {
            clean();
            throw ClientException (error);
        }
//        qDebug("Ausgabe: %s",m_outFileName);
//        qDebug("Fehler: %s",m_errFileName);
        if (svn_path_is_url(m_p1.cstr())) {
            m_url_is_present = true;
        } else {
            m_working_copy_present = true;
        }
        if (svn_path_is_url(m_p2.cstr())) {
            m_url_is_present = true;
        } else {
            m_working_copy_present = true;
        }

        if (m_r1.revision()->kind==svn_opt_revision_unspecified && m_working_copy_present) {
            m_r1 = svn_opt_revision_base;
        }
        if (m_r2.revision()->kind==svn_opt_revision_unspecified) {
            m_r2 = m_working_copy_present?svn_opt_revision_working : svn_opt_revision_head;
        }
    }

    DiffData::~DiffData()
    {
        clean();
    }

    void DiffData::clean()
    {
        close();
#if !((SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 4))
        if (m_outFileName != 0) {
            svn_error_clear (svn_io_remove_file (m_outFileName, m_Pool));
            m_outFileName=0;
        }
        if (m_errFileName != 0) {
            svn_error_clear (svn_io_remove_file (m_errFileName, m_Pool));
            m_errFileName=0;
        }
#endif
    }

    void DiffData::close()
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

    QByteArray DiffData::content()
    {
        if (!m_outFileName) {
            return QByteArray();
        }
        close();
        QFile fi(m_outFileName);
        if (!fi.open(QIODevice::ReadOnly)) {
            throw ClientException(QString("%1 '%2'").arg(fi.errorString()).arg(m_outFileName).toLatin1().constData());
        }

        QByteArray res = fi.readAll();
        fi.close();
        return res;
    }
}
