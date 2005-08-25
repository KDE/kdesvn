#ifndef _RUNTEMPFILE_H
#define _RUNTEMPFILE_H

#include <krun.h>
#include <ktempfile.h>

#include <qstring.h>
#include <qobject.h>

class RunTempfile:public QObject
{
    Q_OBJECT
public:
    RunTempfile(const QByteArray&);
    virtual ~RunTempfile();

    bool status()const;
protected:
    KRun*_job;
    KTempFile*_file;
    bool _stat;
    QTimer m_timer;
    bool initdone;

protected slots:
    void finished();
    void timeout();
};

#endif
