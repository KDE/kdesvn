#ifndef _MODIFIED_THREAD_H
#define _MODIFIED_THREAD_H

#include "svncpp/client.hpp"
#include "svncpp/revision.hpp"
#include "svncpp/status.hpp"
#include "ccontextlistener.h"
#include "eventnumbers.h"

#include <qthread.h>
#include <qevent.h>

class QObject;

class CheckModifiedThread:public QThread
{
public:
    CheckModifiedThread(QObject*,const QString&what,bool _updates=false);
    virtual ~CheckModifiedThread();
    virtual void run();
    virtual void cancelMe();
    virtual const svn::StatusEntries&getList()const;

protected:
    QMutex mutex;
    svn::Client m_Svnclient;
    svn::Context* m_CurrentContext;
    smart_pointer<CContextListener> m_SvnContext;
    QObject*m_Parent;
    QString m_what;
    bool m_updates;
    svn::StatusEntries m_Cache;
};

class ThreadEndEvent:public QEvent
{
public:
    ThreadEndEvent(QThread*);
    virtual ~ThreadEndEvent();

    QThread* threadObject();

protected:
    QThread* m_threadObject;
};

#endif

