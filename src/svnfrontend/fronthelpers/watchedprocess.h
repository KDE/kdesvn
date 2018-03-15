/***************************************************************************
 *   Copyright (C) 2008 by Rajko Albrecht                                  *
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
#include <kprocess.h>

#ifndef WATCHEDPROCESS_H
#define WATCHEDPROCESS_H

class ProcessData;

class WatchedProcess: public KProcess
{
    Q_OBJECT
public:
    explicit WatchedProcess(QObject *parent = nullptr);
    ~WatchedProcess();

    void appendTempFile(const QString &);
    void appendTempDir(const QString &);
    void setAutoDelete(bool);
    bool autoDelete()const;

private:
    ProcessData *m_Data;

protected Q_SLOTS:
    void slotError(QProcess::ProcessError);
    void slotFinished(int, QProcess::ExitStatus);
    void slotReadyReadStandardError();
    void slotReadyReadStandardOutput();
    void slotStarted();
    void slotStateChanged(QProcess::ProcessState);

Q_SIGNALS:
    void dataStderrRead(const QByteArray &, WatchedProcess *);
    void dataStdoutRead(const QByteArray &, WatchedProcess *);

    void error(QProcess::ProcessError, WatchedProcess *);
    void finished(int, QProcess::ExitStatus, WatchedProcess *);
    void started(WatchedProcess *);
    void stateChanged(QProcess::ProcessState newState, WatchedProcess *);
};

#endif
