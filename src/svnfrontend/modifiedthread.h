#ifndef _MODIFIED_THREAD_H
#define _MODIFIED_THREAD_H

#include "svncpp/client.hpp"
#include "svncpp/revision.hpp"
#include "svncpp/status.hpp"
#include "ccontextlistener.h"

#include <qthread.h>
#include <qevent.h>

class SvnActions;

class CheckModifiedThread:public QThread
{
public:
    CheckModifiedThread(SvnActions*,const QString&what);
    virtual ~CheckModifiedThread();
    virtual void run();
    virtual void cancelMe();

    svn::StatusEntries m_Cache;
protected:
    QMutex mutex;
    svn::Client m_Svnclient;
    svn::Context* m_CurrentContext;
    smart_pointer<CContextListener> m_SvnContext;
    SvnActions*m_Parent;
    QString m_what;
};

class ThreadEndEvent:public QEvent
{
public:
    ThreadEndEvent();
};

#endif

