/***************************************************************************
 *   Copyright (C) 2005-2007 by Rajko Albrecht                             *
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

#include "kdesvn-config.h"
#include "kdesvnd.h"
#include "src/ksvnwidgets/authdialogimpl.h"
#include "src/ksvnwidgets/ssltrustprompt_impl.h"
#include "src/ksvnwidgets/commitmsg_impl.h"
#include "src/settings/kdesvnsettings.h"
#include "src/ksvnwidgets/pwstorage.h"
#include "src/svnqt/client.hpp"
#include "src/svnqt/revision.hpp"
#include "src/svnqt/status.hpp"
#include "src/svnqt/context_listener.hpp"
#include "src/svnqt/url.hpp"
#include "src/svnqt/svnqttypes.hpp"
#include "helpers/ktranslateurl.h"
#include "kdesvndadaptor.h"

#include <kdebug.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>
#include <kfiledialog.h>
#include <kpassworddialog.h>

#include <kpluginfactory.h>
#include <kpluginloader.h>

#include <qdir.h>
#include <qvariant.h>

K_PLUGIN_FACTORY(KdeSvndFactory,
                 registerPlugin<kdesvnd>();
                 )
K_EXPORT_PLUGIN(KdeSvndFactory("kio_kdesvn"))

class IListener:public svn::ContextListener
{
    friend class kdesvnd;

    kdesvnd*m_back;
public:
    IListener(kdesvnd*p);
    virtual ~IListener();
    /* context-listener methods */
    virtual bool contextGetLogin (const QString & realm,
                                  QString & username,
                                  QString & password,
                                  bool & maySave);
    virtual bool contextGetSavedLogin (const QString & realm,QString & username,QString & password);
    virtual bool contextGetCachedLogin (const QString & realm,QString & username,QString & password);

    virtual void contextNotify (const char *path,
                                svn_wc_notify_action_t action,
                                svn_node_kind_t kind,
                                const char *mime_type,
                                svn_wc_notify_state_t content_state,
                                svn_wc_notify_state_t prop_state,
                                svn_revnum_t revision);
    virtual void contextNotify (const svn_wc_notify_t *action);

    virtual bool contextCancel();
    virtual bool contextGetLogMessage (QString & msg,const svn::CommitItemList&);
    virtual svn::ContextListener::SslServerTrustAnswer
            contextSslServerTrustPrompt (const SslServerTrustData & data,
            apr_uint32_t & acceptedFailures);
    virtual bool contextSslClientCertPrompt (QString & certFile);
    virtual bool contextLoadSslClientCertPw(QString&password,const QString&realm);
    virtual bool contextSslClientCertPwPrompt (QString & password,
                                               const QString & realm, bool & maySave);
    virtual void contextProgress(long long int current, long long int max);

    /* context listener virtuals end */

protected:
    svn::Client* m_Svnclient;
    svn::ContextP m_CurrentContext;
};

IListener::IListener(kdesvnd*p)
    :svn::ContextListener()
{
    m_Svnclient = svn::Client::getobject(0,0);
    m_back=p;
    m_CurrentContext = new svn::Context();
    m_CurrentContext->setListener(this);
    m_Svnclient->setContext(m_CurrentContext);
}

IListener::~IListener()
{
}

kdesvnd::kdesvnd(QObject* parent, const QList<QVariant>&) : KDEDModule(parent)
{
    KGlobal::locale()->insertCatalog("kdesvn");
    m_Listener=new IListener(this);
    new KdesvndAdaptor(this);
}

kdesvnd::~kdesvnd()
{
    delete m_Listener;
}

QStringList kdesvnd::getActionMenu (const KUrl::List& list)
{
    QStringList result;
    Kdesvnsettings::self()->readConfig();
    if (Kdesvnsettings::no_konqueror_contextmenu()||list.count()==0) {
        return result;
    }
    QString base;


    bool parentIsWc = false;
    bool itemIsWc = isWorkingCopy(list[0],base);
    bool itemIsRepository = false;

    QString _par = list[0].directory(KUrl::IgnoreTrailingSlash);
    parentIsWc = isWorkingCopy(_par,base);

    if (!parentIsWc && !itemIsWc) {
        itemIsRepository = isRepository(list[0]);
    }

    if (!itemIsWc) {
        if (itemIsRepository) {
            result << "Export"
                   << "Checkout";
        } else {
            result << "Exportto"
                   << "Checkoutto";
        }
    } else {
        result << "Update"
               << "Commit";
    }

    if (!parentIsWc && !itemIsWc) {
        if (itemIsRepository) {
            result << "Log"
                    << "Info";
            if (isRepository(list[0].upUrl())) {
                result << "Blame"
                        << "Rename";
            }
            result << "Tree";
        }
        return result;
    }

    if (!itemIsWc) {
        result << "Add";
        return result;
    }

    result << "Log"
        << "Tree"
        << "Info"
        << "Diff"
        << "Rename"
        << "Revert";

    KUrl url = helpers::KTranslateUrl::translateSystemUrl(list[0]);

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

QStringList kdesvnd::getSingleActionMenu(const QString& what)
{
    KUrl::List l; l.append(KUrl(what));
    return getActionMenu(l);
}

QStringList kdesvnd::get_login(const QString&realm,const QString&user)
{
    AuthDialogImpl auth(realm,user);
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

int kdesvnd::get_sslaccept(const QString& hostname,const QString& fingerprint,const QString& validFrom,const QString& validUntil,const QString& issuerDName,const QString& realm)
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

QStringList kdesvnd::get_sslclientcertpw(const QString& realm)
{
    QStringList resList;
    KPasswordDialog dlg(0,KPasswordDialog::DomainReadOnly|KPasswordDialog::ShowKeepPassword);
    dlg.setDomain(realm);
    dlg.setCaption(i18n("Enter password for realm %1").arg(realm));
    dlg.setKeepPassword(true);
    if (dlg.exec()!=KPasswordDialog::Accepted) {
        return resList;
    }
    resList.append(dlg.password());
    if (dlg.keepPassword()) {
        resList.append("true");
    } else {
        resList.append("false");
    }
    return resList;
}

QString kdesvnd::get_sslclientcertfile()
{
    QString afile = KFileDialog::getOpenFileName(KUrl(),
        QString::null,
        0,
        i18n("Open a file with a #PKCS12 certificate"));
    return afile;
}

QStringList kdesvnd::get_logmsg()
{
    QStringList res;
    bool ok;
    QString logMessage = Commitmsg_impl::getLogmessage(&ok,0,0,0);
    if (!ok) {
        return res;
    }
    res.append(logMessage);
    return res;
}

QStringList kdesvnd::get_logmsg(const QDBusVariant&_in)
{
    QStringList res;
    if (_in.variant().type()!=QVariant::Map) {
        return res;
    }
    QMap<QString,QVariant> v = _in.variant().toMap();
    QMap<QString,QString> list;
    QMap<QString,QVariant>::const_iterator _it=v.begin();
    for(;_it!=v.end();++_it) {
        list[_it.key()]=_it.value().toString();
    }

    bool ok;
    QString logMessage = Commitmsg_impl::getLogmessage(list,&ok,0,0,0);
    if (!ok) {
        return res;
    }
    res.append(logMessage);
    return res;
}

QString kdesvnd::cleanUrl(const KUrl&url)
{
    QString cleanpath = url.path();
    while (cleanpath.endsWith("/")) {
        cleanpath.truncate(cleanpath.length()-1);
    }
    return cleanpath;
}

/* just simple name check of course - no network acess! */
bool kdesvnd::isRepository(const KUrl&url)
{
    QString proto = svn::Url::transformProtokoll(url.protocol());
    if (proto=="file") {
        // local access - may a repository
        svn::Revision where = svn::Revision::HEAD;
        svn::StatusEntries dlist;
        try {
            m_Listener->m_Svnclient->status("file://"+cleanUrl(url),svn::DepthEmpty,false,false,false,where);
        } catch (const svn::ClientException&e) {
            kDebug()<< e.msg()<<endl;
            return false;
        }
        return true;
    } else {
        return svn::Url::isValid(proto);
    }
}

bool kdesvnd::isWorkingCopy(const KUrl&_url,QString&base)
{
    base = "";
    KUrl url = _url;
    url = helpers::KTranslateUrl::translateSystemUrl(url);

    if (url.isEmpty()||!url.isLocalFile()||url.protocol()!="file") return false;
    svn::Revision peg(svn_opt_revision_unspecified);
    svn::Revision rev(svn_opt_revision_unspecified);
    svn::InfoEntries e;
    try {
        e = m_Listener->m_Svnclient->info(cleanUrl(url),svn::DepthEmpty,rev,peg);
    } catch (const svn::ClientException&e) {
        return false;
    }
    base=e[0].url();
    return true;
}

bool IListener::contextGetSavedLogin (const QString & realm,QString & username,QString & password)
{
    PwStorage::self()->getLogin(realm,username,password);
    return true;
}

bool IListener::contextGetCachedLogin (const QString & realm,QString & username,QString & password)
{
    PwStorage::self()->getCachedLogin(realm,username,password);
    return true;
}

bool IListener::contextGetLogin (const QString & realm,
                                  QString & username,
                                  QString & password,
                                  bool & maySave)
{
    maySave=false;
    QStringList res = m_back->get_login(realm,username);
    if (res.count()!=3) {
        return false;
    }
    username = res[0];
    password = res[1];
    maySave = (res[2]=="true");
    if (maySave && Kdesvnsettings::passwords_in_wallet() ) {
        PwStorage::self()->setLogin(realm,username,password);
        maySave=false;
    }
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

void IListener::contextNotify(const svn_wc_notify_t * /*action*/)
{
}

bool IListener::contextCancel()
{
    return false;
}

bool IListener::contextGetLogMessage (QString & msg,const svn::CommitItemList&)
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

bool IListener::contextLoadSslClientCertPw(QString&password,const QString&realm)
{
    return PwStorage::self()->getCertPw(realm,password);
}

bool IListener::contextSslClientCertPwPrompt (QString & password,
                                   const QString & realm, bool & maySave)
{
    maySave=false;
    if (PwStorage::self()->getCertPw(realm,password)) {
        return true;
    }
    QStringList res = m_back->get_sslclientcertpw(realm);
    if (res.size()!=2) {
        return false;
    }
    password=res[0];
    maySave=res[1]==QString("true");

    if (maySave && Kdesvnsettings::passwords_in_wallet() ) {
        PwStorage::self()->setCertPw(realm,password);
        maySave=false;
    }

    return true;
}

void IListener::contextProgress(long long int, long long int)
{
}
