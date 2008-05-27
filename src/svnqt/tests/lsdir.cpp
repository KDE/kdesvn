
#include "src/svnqt/client.hpp"
#include "src/svnqt/tests/testconfig.h"
#include "src/svnqt/status.hpp"
#include "src/svnqt/svnqttypes.hpp"
#include <iostream>

int main(int,char**)
{
    svn::ContextP m_CurrentContext;
    svn::Client* m_Svnclient;
    m_Svnclient=svn::Client::getobject(0,0);
    m_CurrentContext = new svn::Context();

    m_Svnclient->setContext(m_CurrentContext);
    svn::DirEntries dlist;

    QString p = QString("file://%1").arg(TESTREPOPATH);
    QString l = QString("%1").arg(TESTCOPATH);

    try {
        dlist = m_Svnclient->list(svn::Path(p),svn::Revision::HEAD,svn::Revision::HEAD,svn::DepthInfinity,true);
    } catch (svn::ClientException e) {
        QString ex = e.msg();
        std::cout << ex.TOUTF8() << std::endl;
        return -1;
    }
    std::cout << "List 1 "<<dlist.size()<<std::endl;
    for (unsigned int i=0; i < dlist.size();++i) {
        QDateTime dt = svn::DateTime(dlist[i]->time());
        std::cout << dlist[i]->name() << " "
                << dlist[i]->lastAuthor() << " "
                << dlist[i]->size() << " "
                << dt.toTime_t() << std::endl;
    }
    try {
        dlist = m_Svnclient->list(svn::Path(p),svn::Revision::HEAD,svn::Revision::HEAD,svn::DepthEmpty,false);
    } catch (svn::ClientException e) {
        QString ex = e.msg();
        std::cout << ex.TOUTF8() << std::endl;
        return -1;
    }
    std::cout << "================"<<std::endl;
    std::cout << "List 2 "<<dlist.size()<<std::endl;
    for (unsigned int i=0; i < dlist.size();++i) {
        QDateTime dt = svn::DateTime(dlist[i]->time());
        std::cout << dlist[i]->name() << " "
                << dlist[i]->lastAuthor() << " "
                << dlist[i]->size() << " "
                << dt.toTime_t() << std::endl;
    }
    std::cout << "================"<<std::endl;
    svn::StatusEntries slist;
    try {
        slist = m_Svnclient->status(svn::Path(p),svn::DepthInfinity,true,true,true,svn::Revision::HEAD,true,false);
    } catch (svn::ClientException e) {
        QString ex = e.msg();
        std::cout << ex.TOUTF8() << std::endl;
        return -1;
    }
    for (unsigned int i=0; i < slist.size();++i) {
        std::cout << slist[i]->path()<< std::endl;
    }
    std::cout << "================"<<std::endl;
    std::cout << "Second status:"<<std::endl;
    try {
        slist = m_Svnclient->status(svn::Path(l),svn::DepthInfinity,true,true,true,svn::Revision::WORKING,true,false);
    } catch (svn::ClientException e) {
        QString ex = e.msg();
        std::cout << ex.TOUTF8() << std::endl;
        return -1;
    }
    for (unsigned int i=0; i < slist.size();++i) {
        std::cout << slist[i]->path()<< std::endl;
    }

    return 0;
}
