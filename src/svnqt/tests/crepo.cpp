#include "src/svnqt/client.hpp"
#include "src/svnqt/tests/testconfig.h"
#include "src/svnqt/repository.hpp"
#include "src/svnqt/repositorylistener.hpp"
#include "src/svnqt/targets.hpp"

#include "testlistener.h"

#include <iostream>
#include <unistd.h>
#include <qstringlist.h>

class Listener:public svn::repository::RepositoryListener
{
    public:
        Listener(){}
        virtual ~Listener(){}
        virtual void sendWarning(const QString&msg)
        {
            std::cout << msg << std::endl;
        }
        virtual void sendError(const QString&msg)
        {
            std::cout << msg << std::endl;
        }
        virtual bool isCanceld(){return false;}
};

int main(int,char**)
{
    QString p = TESTREPOPATH;
    Listener ls;
    svn::repository::Repository rp(&ls);
    try {
        rp.CreateOpen(p,"fsfs");
    } catch (svn::ClientException e) {
        QString ex = e.msg();
        std::cout << ex.TOUTF8() << std::endl;
        return -1;
    }

    svn::ContextP m_CurrentContext;
    svn::Client* m_Svnclient;
    m_Svnclient=svn::Client::getobject(0,0);
    TestListener tl;
    m_CurrentContext = new svn::Context();
    m_CurrentContext->setListener(&tl);
    p = "file://"+p;

    m_Svnclient->setContext(m_CurrentContext);
    QStringList s; s.append(p+"/trunk"); s.append(p+"/branches"); s.append(p+"/tags");

    try {
        m_Svnclient->mkdir(svn::Targets(s),"Test mkdir");
        m_Svnclient->checkout(p,TESTCOPATH,svn::Revision::HEAD,svn::Revision::HEAD,true,false);
    } catch (svn::ClientException e) {
        QString ex = e.msg();
        std::cout << ex.TOUTF8() << std::endl;
        return -1;
    }

    return 0;
}
