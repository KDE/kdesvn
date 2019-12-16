/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   https://kde.org/applications/development/org.kde.kdesvn               *
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
#include "watchedprocess.h"

#include <QDir>
#include <QStringList>

class ProcessData
{
public:
    ProcessData() = default;
    ~ProcessData()
    {
        QStringList::iterator it2;
        for (const QString &fn : qAsConst(_tempFiles)) {
            QFile::remove(fn);
        }
        for (const QString &dir : qAsConst(_tempDirs)) {
            QDir(dir).removeRecursively();
        }
    }

    QStringList _tempFiles;
    QStringList _tempDirs;
    bool _autoDelete = false;
};

WatchedProcess::WatchedProcess(QObject *parent)
    : KProcess(parent)
    , m_Data(new ProcessData)
{
    connect(this, &QProcess::errorOccurred,
            this, &WatchedProcess::slotError);
    connect(this, QOverload<int, QProcess::ExitStatus>::of(&KProcess::finished),
            this, &WatchedProcess::slotFinished);
    connect(this, &QProcess::readyReadStandardError,
            this, &WatchedProcess::slotReadyReadStandardError);
    connect(this, &QProcess::readyReadStandardOutput,
            this, &WatchedProcess::slotReadyReadStandardOutput);
    connect(this, &KProcess::started,
            this, &WatchedProcess::slotStarted);
    connect(this, &KProcess::stateChanged,
            this, &WatchedProcess::slotStateChanged);
}

WatchedProcess::~WatchedProcess()
{
    if (state() == QProcess::NotRunning) {
        terminate();
        if (!waitForFinished(1000)) {
            kill();
        }
    }
    delete m_Data;
}

void WatchedProcess::setAutoDelete(bool autodel)
{
    m_Data->_autoDelete = autodel;
}

bool WatchedProcess::autoDelete()const
{
    return m_Data->_autoDelete;
}

void WatchedProcess::slotError(QProcess::ProcessError error_code)
{
    emit error(error_code, this);
}

void WatchedProcess::slotFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    emit finished(exitCode, exitStatus, this);
    if (m_Data->_autoDelete) {
        m_Data->_autoDelete = false;
        deleteLater();
    }
}

void WatchedProcess::WatchedProcess::slotReadyReadStandardError()
{
    emit dataStderrRead(readAllStandardError(), this);
}

void WatchedProcess::slotReadyReadStandardOutput()
{
    emit dataStdoutRead(readAllStandardOutput(), this);
}

void WatchedProcess::slotStarted()
{
    emit started(this);
}

void WatchedProcess::slotStateChanged(QProcess::ProcessState state)
{
    emit stateChanged(state, this);
}

void WatchedProcess::appendTempFile(const QString &aFile)
{
    m_Data->_tempFiles.append(aFile);
}

void WatchedProcess::appendTempDir(const QString &aDir)
{
    m_Data->_tempDirs.append(aDir);
}
