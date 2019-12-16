/***************************************************************************
 *   Copyright (C) 2008-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   https://kde.org/applications/development/org.kde.kdesvn               *
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
#include "diff_data.h"
#include "exception.h"
#include "helper.h"

#include <QFile>

#include <svn_io.h>
#include <svn_path.h>

namespace svn
{
DiffData::DiffData(const Path &aTmpPath, const Path &_p1, const Revision &_r1, const Path &_p2, const Revision &_r2)
    : m_Pool()
#if SVN_API_VERSION >= SVN_VERSION_CHECK(1,8,0)
    , m_outStream(new stream::SvnByteStream)
    , m_errStream(new stream::SvnByteStream)
#else
    , m_tmpPath(aTmpPath)
    , m_outFile(nullptr)
    , m_errFile(nullptr)
    , m_outFileName(nullptr)
    , m_errFileName(nullptr)
#endif
    , m_p1(_p1)
    , m_p2(_p2)
    , m_r1(_r1)
    , m_r2(_r2)
    , m_working_copy_present(false)
    , m_url_is_present(false)
{
#if SVN_API_VERSION >= SVN_VERSION_CHECK(1,8,0)
    Q_UNUSED(aTmpPath)
#endif
    init();
}

void DiffData::init()
{
#if SVN_API_VERSION < SVN_VERSION_CHECK(1,8,0)
    svn_error_t *error;
    Pool scratchPool;
    error = svn_io_open_unique_file3(&m_outFile, &m_outFileName,
                                     m_tmpPath.path().toUtf8(),
                                     svn_io_file_del_on_pool_cleanup, m_Pool, scratchPool);

    if (error != 0) {
        clean();
        throw ClientException(error);
    }
    error = svn_io_open_unique_file3(&m_errFile, &m_errFileName,
                                     m_tmpPath.path().toUtf8(),
                                     svn_io_file_del_on_pool_cleanup, m_Pool, scratchPool);
    if (error != 0) {
        clean();
        throw ClientException(error);
    }
#endif
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

    if (m_r1.revision()->kind == svn_opt_revision_unspecified && m_working_copy_present) {
        m_r1 = svn_opt_revision_base;
    }
    if (m_r2.revision()->kind == svn_opt_revision_unspecified) {
        m_r2 = m_working_copy_present ? svn_opt_revision_working : svn_opt_revision_head;
    }
}

DiffData::~DiffData()
{
    close();
}

void DiffData::close()
{
#if SVN_API_VERSION >= SVN_VERSION_CHECK(1,8,0)
    delete m_outStream;
    m_outStream = nullptr;
    delete m_errStream;
    m_errStream = nullptr;
#else
    if (m_outFile) {
        svn_io_file_close(m_outFile, m_Pool);
        m_outFile = nullptr;
    }
    if (m_errFile) {
        svn_io_file_close(m_errFile, m_Pool);
        m_errFile = nullptr;
    }
#endif
}

QByteArray DiffData::content()
{
#if SVN_API_VERSION >= SVN_VERSION_CHECK(1,8,0)
    return m_outStream->content();
#else
    if (!m_outFileName) {
        return QByteArray();
    }
    close();
    QFile fi(QString::fromUtf8(m_outFileName));
    if (!fi.open(QIODevice::ReadOnly)) {
        throw ClientException(QStringLiteral("%1 '%2'").arg(fi.errorString(), fi.fileName()));
    }

    QByteArray res = fi.readAll();
    fi.close();
    return res;
#endif
}
}
