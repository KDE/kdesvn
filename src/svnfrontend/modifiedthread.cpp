#include "modifiedthread.h"
#include "tcontextlistener.h"

#include <qobject.h>
#include <kdebug.h>
#include <kapplication.h>

CheckModifiedThread::CheckModifiedThread(QObject*_parent,const QString&what,bool _updates)
    : QThread(),mutex()
{
    m_Parent = _parent;
    m_CurrentContext = new svn::Context();
    m_SvnContext = new ThreadContextListener(m_Parent,0);
    if (m_Parent) {
        QObject::connect(m_SvnContext,SIGNAL(sendNotify(const QString&)),m_Parent,SLOT(slotNotifyMessage(const QString&)));
    }

    m_CurrentContext->setListener(m_SvnContext);
    m_what = what;
    m_Svnclient.setContext(m_CurrentContext);
    m_updates = _updates;
}

CheckModifiedThread::~CheckModifiedThread()
{
    delete m_CurrentContext;
}

void CheckModifiedThread::cancelMe()
{
    // method is threadsafe!
    m_SvnContext->setCanceled(true);
}

const svn::StatusEntries&CheckModifiedThread::getList()const
{
    return m_Cache;
}

void CheckModifiedThread::run()
{
    // what must be cleaned!
    svn::Revision where = svn::Revision::HEAD;
    QString ex;
    try {
        //                                  rec  all   up        noign
        m_Cache = m_Svnclient.status(m_what,true,false,m_updates,false,where);
    } catch (svn::ClientException e) {
        m_SvnContext->contextNotify(e.msg());
    }
    KApplication*k = KApplication::kApplication();
    if (k) {
        QCustomEvent*ev = new QCustomEvent(EVENT_THREAD_FINISHED);
        ev->setData((void*)this);
        k->postEvent(m_Parent,ev);
    }
}
