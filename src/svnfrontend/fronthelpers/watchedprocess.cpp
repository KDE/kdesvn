#include "watchedprocess.h"

#include <kio/netaccess.h>

#include <QList>
#include <QString>

class ProcessData
{
public:
    ProcessData()
        :_autoDelete(false)
    {}
    ~ProcessData()
    {
        QStringList::iterator it2;
        for (it2 = _tempFiles.begin();it2 != _tempFiles.end();++it2) {
            KIO::NetAccess::del((*it2),0);
        }
        for (it2 = _tempDirs.begin();it2 != _tempDirs.end();++it2) {
            KIO::NetAccess::del((*it2),0);
        }
    }

    QStringList _tempFiles;
    QStringList _tempDirs;
    bool _autoDelete;
};

WatchedProcess::WatchedProcess(QObject*parent)
    :KProcess(parent)
{
    m_Data = new ProcessData;
    connect(this,SIGNAL(error(QProcess::ProcessError)),SLOT(slotError(QProcess::ProcessError)));
    connect(this,SIGNAL(finished(int,QProcess::ExitStatus)),SLOT(slotFinished(int,QProcess::ExitStatus)));
    connect(this,SIGNAL(readyReadStandardError()),SLOT(slotReadyReadStandardError()));
    connect(this,SIGNAL(readyReadStandardOutput()),SLOT(slotReadyReadStandardOutput()));
    connect(this,SIGNAL(started()),SLOT(slotStarted()));
    connect(this,SIGNAL(stateChanged(QProcess::ProcessState)),SLOT(slotStateChanged(QProcess::ProcessState)));
}

WatchedProcess::~WatchedProcess()
{
    if (state()==QProcess::NotRunning) {
        terminate();
        if (!waitForFinished(1000)) {
            kill();
        }
    }
    delete m_Data;
}

void WatchedProcess::setAutoDelete(bool autodel)
{
    m_Data->_autoDelete=autodel;
}

bool WatchedProcess::autoDelete()const
{
    return m_Data->_autoDelete;
}

void WatchedProcess::slotError(QProcess::ProcessError error_code)
{
    emit error(error_code,this);
}

void WatchedProcess::slotFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    emit finished(exitCode,exitStatus,this);
    if (m_Data->_autoDelete) {
        m_Data->_autoDelete=false;
        deleteLater();
    }
}

void WatchedProcess::WatchedProcess::slotReadyReadStandardError()
{
    emit dataStderrRead(readAllStandardError(),this);
}

void WatchedProcess::slotReadyReadStandardOutput()
{
    emit dataStdoutRead(readAllStandardOutput(),this);
}

void WatchedProcess::slotStarted()
{
    emit started(this);
}

void WatchedProcess::slotStateChanged(QProcess::ProcessState state)
{
    emit stateChanged(state,this);
}

void WatchedProcess::appendTempFile(const QString&aFile)
{
    m_Data->_tempFiles.append(aFile);
}

void WatchedProcess::appendTempDir(const QString&aDir)
{
    m_Data->_tempDirs.append(aDir);
}

#include "watchedprocess.moc"
