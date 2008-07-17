#ifndef _TESTLISTENER_
#define _TESTLISTENER_

#include "src/svnqt/context_listener.hpp"

class TestListener:public svn::ContextListener
{
    public:
        TestListener(){}
        virtual ~TestListener(){}

        virtual void contextProgress(long long int current, long long int max){};
        virtual bool contextSslClientCertPwPrompt (QString &,const QString &, bool &){return false;}
        virtual bool contextLoadSslClientCertPw(QString&,const QString&){return false;}
        virtual bool contextSslClientCertPrompt (QString &){return false;}
        virtual svn::ContextListener::SslServerTrustAnswer
                contextSslServerTrustPrompt (const SslServerTrustData &,
                                             apr_uint32_t & ){return svn::ContextListener::SslServerTrustAnswer();}
        virtual bool contextGetLogMessage (QString &,const svn::CommitItemList&){return false;}
        virtual bool contextCancel(){return false;}
        virtual void contextNotify (const svn_wc_notify_t *){}
        virtual void contextNotify (const char *,svn_wc_notify_action_t,
                                    svn_node_kind_t,
                                    const char *,
                                    svn_wc_notify_state_t,
                                    svn_wc_notify_state_t,
                                    svn_revnum_t){}
        virtual bool contextGetSavedLogin (const QString & realm,QString & username,QString & password){return false;}
        virtual bool contextGetCachedLogin (const QString & realm,QString & username,QString & password){return false;} 
        virtual bool contextGetLogin (const QString & realm,
                                      QString & username,
                                      QString & password,
                                      bool & maySave){maySave=false;return false;}

};

#endif
