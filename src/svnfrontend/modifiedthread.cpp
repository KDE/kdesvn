#include "modifiedthread.h"

#include <kdebug.h>

CheckModifiedThread::CheckModifiedThread(SvnActions*_parent,const QString&what)
    : QThread(),mutex()
{
    m_Parent = _parent;
    m_CurrentContext = new svn::Context();
    m_SvnContext = new CContextListener(0);
    m_CurrentContext->setListener(m_SvnContext);
    m_what = what;
    m_Svnclient.setContext(m_CurrentContext);
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

void CheckModifiedThread::run()
{
    // what must be cleaned!
    svn::Revision where = svn::Revision::HEAD;
    QString ex;
    try {
        m_Cache = m_Svnclient.status(m_what,true,false,false,false,where);
    } catch (svn::ClientException e) {
        kdDebug()<<"Exception in thread"<<endl;
    }
    kdDebug()<<"Thread finished"<<endl;
}

ThreadEndEvent::ThreadEndEvent()
    : QEvent(QEvent::User)
{
}
