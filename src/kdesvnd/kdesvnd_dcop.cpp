/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht                                  *
 *   rajko.albrecht@tecways.com                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "config.h"
#include "kdesvnd_dcop.h"
#include "authdialogimpl.h"
#include "ssltrustprompt_impl.h"
#include "logmsg_impl.h"
#include "svncpp/client.hpp"
#include "svncpp/revision.hpp"
#include "svncpp/status.hpp"
#include "svncpp/context_listener.hpp"
#include "svncpp/url.hpp"
#include "helpers/ktranslateurl.h"

#include <kdebug.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kfiledialog.h>

#include <qdir.h>

extern "C" {
// avoid problems with elder kde installations
#ifdef __KDE_HAVE_GCC_VISIBILITY
    KDE_EXPORT KDEDModule *create_kdesvnd(const QCString &name)
#else
    KDEDModule *create_kdesvnd(const QCString &name)
#endif
    {
       return new kdesvnd_dcop(name);
    }
}

class IListener:public svn::ContextListener
{
    friend class kdesvnd_dcop;

    kdesvnd_dcop*m_back;
public:
    IListener(kdesvnd_dcop*p);
    virtual ~IListener();
    /* context-listener methods */
    virtual bool contextGetLogin (const QString & realm,
                                  QString & username,
                                  QString & password,
                                  bool & maySave);
    virtual void contextNotify (const char *path,
                                svn_wc_notify_action_t action,
                                svn_node_kind_t kind,
                                const char *mime_type,
                                svn_wc_notify_state_t content_state,
                                svn_wc_notify_state_t prop_state,
                                svn_revnum_t revision);
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
    virtual void contextNotify (const svn_wc_notify_t *action);
#endif

    virtual bool contextCancel();
    virtual bool contextGetLogMessage (QString & msg);
    virtual svn::ContextListener::SslServerTrustAnswer
            contextSslServerTrustPrompt (const SslServerTrustData & data,
            apr_uint32_t & acceptedFailures);
    virtual bool contextSslClientCertPrompt (QString & certFile);
    virtual bool contextSslClientCertPwPrompt (QString & password,
                                               const QString & realm, bool & maySave);
    /* context listener virtuals end */

protected:
    svn::Client m_Svnclient;
    svn::Context* m_CurrentContext;
};

IListener::IListener(kdesvnd_dcop*p)
    :svn::ContextListener()
{
    m_back=p;
    m_CurrentContext = new svn::Context();
    m_CurrentContext->setListener(this);
    m_Svnclient.setContext(m_CurrentContext);
}

IListener::~IListener()
{
    delete m_CurrentContext;
}

kdesvnd_dcop::kdesvnd_dcop(const QCString&name) : KDEDModule(name)
{
    kdDebug() << "Starting new service... " << endl;
    m_Listener=new IListener(this);
}

kdesvnd_dcop::~kdesvnd_dcop()
{
    kdDebug() << "Going away... " << endl;
    delete m_Listener;
}

QStringList kdesvnd_dcop::getTopLevelActionMenu (const KURL::List list)
{
    QStringList result;
    if (list.count()==0) {
        return result;
    }
    QString base;
    if (!isWorkingCopy(list[0],base)) {
        if (isRepository(list[0])) {
            result << "Export"
                   << "Checkout";
        }
    } else {
        result << "Update"
            << "Commit";
    }
    return result;
}

QStringList kdesvnd_dcop::getActionMenu (const KURL::List list)
{
    QStringList result;
    if (list.count()==0) {
        return result;
    }
    QString base;

    bool parentIsWc = false;
    bool itemIsWc = isWorkingCopy(list[0],base);

    QString _par = list[0].directory(true,true);
    parentIsWc = isWorkingCopy(_par,base);

    if (!parentIsWc && !itemIsWc) {
        if (isRepository(list[0])) {
            result << "Log"
                    << "Info";
            if (isRepository(list[0].upURL())) {
                result << "Blame"
                        << "Rename";
            }
        }
        return result;
    }

    if (!itemIsWc) {
        result << "Add";
        return result;
    }

    result << "Log"
        << "Info"
        << "Diff"
        << "Rename"
        << "Revert";

    KURL url = helpers::KTranslateUrl::translateSystemUrl(list[0]);
    
    QFileInfo f(url.path());
    if (f.isFile()) {
        result << "Blame";
    }

    if (f.isDir()) {
        result << "Addnew";
        result << "Switch";
    }

    return result;
}

QStringList kdesvnd_dcop::getSingleActionMenu(QCString what)
{
    KURL::List l; l.append(KURL(what));
    return getActionMenu(l);
}

QStringList kdesvnd_dcop::get_login(QString realm)
{
    AuthDialogImpl auth(realm);
    QStringList res;
    if (auth.exec()==QDialog::Accepted) {
        res.append(auth.Username());
        res.append(auth.Password());
        if (auth.maySave()) {
            res.append("true");
        } else {
            res.append("false");
        }
    }
    return res;
}

int kdesvnd_dcop::get_sslaccept(QString hostname,QString fingerprint,QString validFrom,QString validUntil,QString issuerDName,QString realm)
{
    bool ok,saveit;
    if (!SslTrustPrompt_impl::sslTrust(
        hostname,
        fingerprint,
        validFrom,
        validUntil,
        issuerDName,
        realm,
        QStringList(),
        &ok,&saveit)) {
        return -1;
    }
    if (!saveit) {
        return 0;
    }
    return 1;
}

QString kdesvnd_dcop::get_sslclientcertfile()
{
    QString afile = KFileDialog::getOpenFileName(QString::null,
        QString::null,
        0,
        i18n("Open a file with a #PKCS12 certificate"));
    return afile;
}

QStringList kdesvnd_dcop::get_logmsg()
{
    QStringList res;
    bool ok;
    QString logMessage = Logmsg_impl::getLogmessage(&ok,0,0,"logmsg_impl");
    if (!ok) {
        return res;
    }
    res.append(logMessage);
    return res;
}

QString kdesvnd_dcop::cleanUrl(const KURL&url)
{
    QString cleanpath = url.path();
    while (cleanpath.endsWith("/")) {
        cleanpath.truncate(cleanpath.length()-1);
    }
    return cleanpath;
}

/* just simple name check of course - no network acess! */
bool kdesvnd_dcop::isRepository(const KURL&url)
{
    kdDebug()<<"kdesvnd_dcop::isRepository Url zum repo check: "<<url<<endl;
    QString proto = svn::Url::transformProtokoll(url.protocol());
    kdDebug()<<"kdesvnd_dcop::isRepository Protokoll: " << proto << endl;
    if (proto=="file") {
        // local access - may a repository
        svn::Revision where = svn::Revision::HEAD;
        svn::StatusEntries dlist;
        try {
            m_Listener->m_Svnclient.status("file://"+cleanUrl(url),false,false,false,false,where);
        } catch (svn::ClientException e) {
            kdDebug()<< e.message()<<endl;
            return false;
        }
        return true;
    } else {
        return svn::Url::isValid(proto);
    }
}

bool kdesvnd_dcop::isWorkingCopy(const KURL&_url,QString&base)
{
    base = "";
    KURL url = _url;
    url = helpers::KTranslateUrl::translateSystemUrl(url);

    if (url.isEmpty()||!url.isLocalFile()||url.protocol()!="file") return false;
    svn::Revision peg(svn_opt_revision_unspecified);
    svn::Revision rev(svn_opt_revision_unspecified);
    svn::InfoEntries e;
    try {
        e = m_Listener->m_Svnclient.info2(cleanUrl(url),false,rev,peg);
    } catch (svn::ClientException e) {
        kdDebug()<< e.message()<<endl;
        return false;
    }
    base=e[0].url();
    return true;
}

bool IListener::contextGetLogin (const QString & realm,
                                  QString & username,
                                  QString & password,
                                  bool & maySave)
{
    QStringList res = m_back->get_login(realm);
    if (res.count()!=3) {
        return false;
    }
    username = res[0];
    password = res[1];
    maySave = (res[2]=="true");
    return true;
}

void IListener::contextNotify(const char * /*path*/,
                                 svn_wc_notify_action_t /*action*/,
                                 svn_node_kind_t /*kind*/,
                                 const char */*mime_type*/,
                                 svn_wc_notify_state_t /*content_state*/,
                                 svn_wc_notify_state_t /*prop_state*/,
                                 svn_revnum_t /*revision*/)
{
}

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
void IListener::contextNotify(const svn_wc_notify_t * /*action*/)
{
}
#endif

bool IListener::contextCancel()
{
    return false;
}

bool IListener::contextGetLogMessage (QString & msg)
{
    QStringList res = m_back->get_logmsg();
    if (res.count()==0) {
        return false;
    }
    msg = res[1];
    return true;
}

svn::ContextListener::SslServerTrustAnswer IListener::contextSslServerTrustPrompt (const SslServerTrustData & data,
        apr_uint32_t & /*acceptedFailures*/)
{
    int res = m_back->get_sslaccept(data.hostname,
            data.fingerprint,
            data.validFrom,
            data.validUntil,
            data.issuerDName,
            data.realm);
    switch (res) {
        case -1:
            return DONT_ACCEPT;
            break;
        case 1:
            return ACCEPT_PERMANENTLY;
            break;
        default:
        case 0:
            return ACCEPT_TEMPORARILY;
            break;
    }
    /* avoid compiler warnings */
    return ACCEPT_TEMPORARILY;
}

bool IListener::contextSslClientCertPrompt (QString & certFile)
{
    certFile = m_back->get_sslclientcertfile();
    if (certFile.isEmpty()) {
        return false;
    }
    return true;
}

bool IListener::contextSslClientCertPwPrompt (QString & password,
                                   const QString & realm, bool & maySave)
{
    return false;
}
