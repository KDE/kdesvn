#include "client.hpp"
#include "repository.hpp"
#include "context.hpp"
#include "datetime.hpp"

#include <qdatastream.h>

int main(int,char**)
{
    svn::Client::getobject(0,0);
    svn::repository::Repository rep(0L);
    svn::ContextP myContext = new svn::Context();

    QByteArray tout;
    QDataStream out(tout,IO_WriteOnly);
    svn::Client*m_Svnclient = svn::Client::getobject(0,0);
    svn::ContextP m_CurrentContext = new svn::Context();
    m_Svnclient->setContext(m_CurrentContext);
    bool gotit = true;
    svn::LogEntriesMap m_OldHistory;

    try {
        m_Svnclient->log("http://www.alwins-world.de/repos/kdesvn/trunk",svn::Revision::HEAD,20,m_OldHistory,true,false,0);
    } catch (svn::ClientException ce) {
        gotit = false;
    }
    if (gotit) {
        out << m_OldHistory;
        svn::LogEntriesMap m_NewHistory;
        QDataStream inp(tout,IO_ReadOnly);
        inp >> m_NewHistory;

        svn::LogEntriesMap::Iterator it;
        for (it=m_NewHistory.begin();it!=m_NewHistory.end();++it) {
            qDebug("%lu %s %s",it.key(),it.data().author.ascii(),it.data().message.ascii());
        }
    }

  return 1;
}
