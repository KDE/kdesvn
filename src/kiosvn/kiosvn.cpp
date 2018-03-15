/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "kiosvn.h"
#include "kdesvn-config.h"
#include "kiolistener.h"

#include <QFile>
#include <QTemporaryDir>
#include <QTemporaryFile>

#include "svnqt/svnqttypes.h"
#include "svnqt/dirent.h"
#include "svnqt/url.h"
#include "svnqt/status.h"
#include "svnqt/targets.h"
#include "svnqt/info_entry.h"
#include "svnqt/client_parameter.h"
#include "svnqt/client_commit_parameter.h"
#include "svnqt/client_update_parameter.h"
#include "settings/kdesvnsettings.h"
#include "helpers/stringhelper.h"
#include "helpers/sshagent.h"
#include "helpers/kdesvn_debug.h"
#include "kdesvndinterface.h"
#include "kio_macros.h"

#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <klocalizedstring.h>

namespace KIO
{

class KioSvnData
{
public:
    explicit KioSvnData(kio_svnProtocol *);
    ~KioSvnData();

    void reInitClient();
    void resetListener();

    KioListener m_Listener;
    bool first_done;
    bool dispProgress;
    bool dispWritten;
    svn::ContextP m_CurrentContext;
    svn::ClientP m_Svnclient;
    svn::Revision urlToRev(const QUrl &);
    QTime m_last;
    qulonglong m_Id;
};

KioSvnData::KioSvnData(kio_svnProtocol *par)
    : m_Listener(par)
    , first_done(false)
    , dispProgress(false)
    , dispWritten(false)
    , m_Svnclient(svn::Client::getobject(svn::ContextP()))
    , m_last(QTime::currentTime())
    , m_Id(0)    // null is an invalid id
{
    reInitClient();
}

void KioSvnData::reInitClient()
{
    if (first_done) {
        return;
    }
    SshAgent ag;
    ag.querySshAgent();

    first_done = true;
    m_CurrentContext = svn::ContextP(new svn::Context);
    m_CurrentContext->setListener(&m_Listener);
    m_Svnclient->setContext(m_CurrentContext);
}

void KioSvnData::resetListener()
{
    if (!first_done) {
        reInitClient();
    }
    m_Listener.uncancel();
}

KioSvnData::~KioSvnData()
{
    m_Listener.setCancel(true);
    /* wait a little bit */
    sleep(1);
    m_CurrentContext->setListener(nullptr);
}

svn::Revision KioSvnData::urlToRev(const QUrl &url)
{
    const QList<QPair<QString, QString>> q = QUrlQuery(url).queryItems();

    /* we try to check if it is ssh and try to get a password for it */
    const QString proto = url.scheme();

    if (proto.contains(QLatin1String("ssh"))) {
        SshAgent ag;
        ag.addSshIdentities();
    }

    svn::Revision rev = svn::Revision::UNDEFINED;
    typedef QPair<QString, QString> myStrPair;
    Q_FOREACH(const myStrPair &p, q) {
        if (p.first == QLatin1String("rev")) {
            const QString v = p.second;
            svn::Revision tmp;
            m_Svnclient->url2Revision(v, rev, tmp);
        }
    }
    return rev;
}

kio_svnProtocol::kio_svnProtocol(const QByteArray &pool_socket, const QByteArray &app_socket)
    : SlaveBase("kio_ksvn", pool_socket, app_socket), StreamWrittenCb()
{
    m_pData = new KioSvnData(this);
    m_pData->m_Id = reinterpret_cast<qulonglong>(this);
}

kio_svnProtocol::~kio_svnProtocol()
{
    unregisterFromDaemon();
    delete m_pData;
}

}

extern "C"
{
    Q_DECL_EXPORT int kdemain(int argc, char **argv);
}

int kdemain(int argc, char **argv)
{
    QCoreApplication::setApplicationName(QLatin1String("kio_ksvn"));
    qCDebug(KDESVN_LOG) << "*** Starting kio_ksvn " << endl;

    if (argc != 4) {
        qCDebug(KDESVN_LOG) << "Usage: kio_ksvn  protocol domain-socket1 domain-socket2" << endl;
        exit(-1);
    }
    KIO::kio_svnProtocol slave(argv[2], argv[3]);
    slave.dispatchLoop();
    qCDebug(KDESVN_LOG) << "*** kio_ksvn Done" << endl;
    return 0;
}

namespace KIO
{

void kio_svnProtocol::listSendDirEntry(const svn::DirEntry &direntry)
{
    const QDateTime dt(direntry.time().toQDateTime());
    KIO::UDSEntry entry;
    if (direntry.name().isEmpty()) {
        qCDebug(KDESVN_LOG) << "Skipping empty entry!" << endl;
        return;
    }
    if (createUDSEntry(direntry.name(),
                       direntry.lastAuthor(),
                       direntry.size(),
                       direntry.kind() == svn_node_dir ? true : false,
                       dt.toTime_t(),
                       entry)) {
        listEntry(entry);
    }
}

/*!
    \fn kio_svnProtocol::listDir (const QUrl&url)
 */
void kio_svnProtocol::listDir(const QUrl &url)
{
    qCDebug(KDESVN_LOG) << "kio_svn::listDir(const QUrl& url) : " << url.url() << endl ;
    m_pData->resetListener();
    svn::DirEntries dlist;
    svn::Revision rev = m_pData->urlToRev(url);
    if (rev == svn::Revision::UNDEFINED) {
        rev = svn::Revision::HEAD;
    }

    try {
        // we ignoring the result 'cause it is done via kiolistener for a smoother insert of items.
        dlist = m_pData->m_Svnclient->list(makeSvnPath(url), rev, rev, svn::DepthImmediates, false);
    } catch (const svn::ClientException &e) {
        QString ex = e.msg();
        qCDebug(KDESVN_LOG) << ex << endl;
        extraError(KIO::ERR_SLAVE_DEFINED, ex);
        return;
    }
    finished();
    qCDebug(KDESVN_LOG) << "Listing finished" << endl;
}

void kio_svnProtocol::stat(const QUrl &url)
{
    qCDebug(KDESVN_LOG) << "kio_svn::stat " << url << endl;
    m_pData->resetListener();
    svn::Revision rev = m_pData->urlToRev(url);
    if (rev == svn::Revision::UNDEFINED) {
        rev = svn::Revision::HEAD;
    }
    svn::Revision peg = rev;
    bool dummy = false;
    svn::InfoEntries e;
    try {
        e = m_pData->m_Svnclient->info(makeSvnPath(url), svn::DepthEmpty, rev, peg);
    } catch (const svn::ClientException &e) {
        QString ex = e.msg();
        qCDebug(KDESVN_LOG) << ex << endl;
        extraError(KIO::ERR_SLAVE_DEFINED, ex);
        return;
    }

    if (e.isEmpty()) {
        dummy = true;
    }

    KIO::UDSEntry entry;
    if (dummy) {
        createUDSEntry(url.fileName(), QString(), 0, true, 0, entry);
    } else {
        const QDateTime dt(e[0].cmtDate().toQDateTime());
        if (e[0].kind() == svn_node_file) {
            createUDSEntry(url.fileName(), QString(), 0, false, dt.toTime_t(), entry);
        } else {
            createUDSEntry(url.fileName(), QString(), 0, true, dt.toTime_t(), entry);
        }
    }
    statEntry(entry);
    finished();
}

void kio_svnProtocol::get(const QUrl &url)
{
    if (m_pData->m_Listener.contextCancel()) {
        finished();
        return;
    }
    svn::Revision rev = m_pData->urlToRev(url);
    if (rev == svn::Revision::UNDEFINED) {
        rev = svn::Revision::HEAD;
    }
    KioByteStream dstream(this, url.fileName());
    try {
        const svn::Path path = makeSvnPath(url);
        svn::InfoEntries e;
        e = m_pData->m_Svnclient->info(path, svn::DepthEmpty, rev, rev);
        if (!e.isEmpty()) {
            totalSize(e.at(0).size());
        }
        m_pData->m_Svnclient->cat(dstream, path, rev, rev);
    } catch (const svn::ClientException &e) {
        QString ex = e.msg();
        // dolphin / Konqueror try to get the content without check if it is a folder when listing a folder
        // which results in a lot of error messages via kio notify
        if (e.apr_err() != SVN_ERR_CLIENT_IS_DIRECTORY) {
            extraError(KIO::ERR_SLAVE_DEFINED, QStringLiteral("Subversion error ") + ex);
        }
        return;
    }
    data(QByteArray()); // empty array means we're done sending the data
    finished();
}

void kio_svnProtocol::mkdir(const QUrl &url, int)
{
    qCDebug(KDESVN_LOG) << "kio_svn::mkdir " << url << endl;
    m_pData->resetListener();
    svn::Revision rev = m_pData->urlToRev(url);
    if (rev == svn::Revision::UNDEFINED) {
        rev = svn::Revision::HEAD;
    }
    if (rev != svn::Revision::HEAD) {
        extraError(KIO::ERR_SLAVE_DEFINED, i18n("Can only write on HEAD revision."));
        return;
    }
    m_pData->m_CurrentContext->setLogMessage(getDefaultLog());
    try {
        m_pData->m_Svnclient->mkdir(makeSvnPath(url), getDefaultLog());
    } catch (const svn::ClientException &e) {
        extraError(KIO::ERR_SLAVE_DEFINED, e.msg());
        return;
    }
    finished();
}

void kio_svnProtocol::mkdir(const QList<QUrl> &urls, int)
{
    try {
        m_pData->m_Svnclient->mkdir(svn::Targets::fromUrlList(urls, svn::Targets::UrlConversion::PreferLocalPath), getDefaultLog());
    } catch (const svn::ClientException &e) {
        extraError(KIO::ERR_SLAVE_DEFINED, e.msg());
        return;
    }
    finished();
}

void kio_svnProtocol::rename(const QUrl &src, const QUrl &target, KIO::JobFlags flags)
{
    qCDebug(KDESVN_LOG) << "kio_svn::rename " << src << " to " << target <<  endl;
    m_pData->resetListener();
    Q_UNUSED(flags);
    //bool force  = flags&KIO::Overwrite;
    m_pData->m_CurrentContext->setLogMessage(getDefaultLog());
    try {
        m_pData->m_Svnclient->move(svn::CopyParameter(makeSvnPath(src), makeSvnPath(target)));
    } catch (const svn::ClientException &e) {
        if (e.apr_err() == SVN_ERR_ENTRY_EXISTS) {
            error(KIO::ERR_DIR_ALREADY_EXIST, e.msg());
        } else {
            extraError(KIO::ERR_SLAVE_DEFINED, e.msg());
        }
        return;
    }
    notify(i18n("Renaming %1 to %2 successful", src.toDisplayString(), target.toDisplayString()));
    finished();
}

void kio_svnProtocol::put(const QUrl &url, int permissions, KIO::JobFlags flags)
{
    Q_UNUSED(permissions);
    m_pData->resetListener();
    svn::Revision rev = m_pData->urlToRev(url);
    if (rev == svn::Revision::UNDEFINED) {
        rev = svn::Revision::HEAD;
    }
    if (rev != svn::Revision::HEAD) {
        extraError(KIO::ERR_SLAVE_DEFINED, i18n("Can only write on HEAD revision."));
        return;
    }
    svn::Revision peg = rev;
    svn::InfoEntries e;
    bool exists = true;
    try {
        e = m_pData->m_Svnclient->info(makeSvnPath(url), svn::DepthEmpty, rev, peg);
    } catch (const svn::ClientException &e) {
        if (e.apr_err() == SVN_ERR_ENTRY_NOT_FOUND || e.apr_err() == SVN_ERR_RA_ILLEGAL_URL) {
            exists = false;
        } else {
            extraError(KIO::ERR_SLAVE_DEFINED, e.msg());
            return;
        }
    }
    QSharedPointer<QFile> tmpfile;
    QSharedPointer<QTemporaryDir> _codir;
    if (exists) {
        if (flags & KIO::Overwrite) {
            if (!supportOverwrite()) {
                extraError(KIO::ERR_SLAVE_DEFINED, i18n("Overwriting existing items is disabled in settings."));
                return;
            }
            _codir = QSharedPointer<QTemporaryDir>(new QTemporaryDir);
            _codir->setAutoRemove(true);
            svn::Path path = makeSvnPath(url);
            path.removeLast();
            try {
                notify(i18n("Start checking out to temporary folder"));
                m_pData->dispWritten = true;
                registerToDaemon();
                startOp(-1, i18n("Checking out %1", path.native()));
                svn::CheckoutParameter params;
                params.moduleName(path).destination(svn::Path(_codir->path())).revision(rev).peg(peg).depth(svn::DepthFiles);
                m_pData->m_Svnclient->checkout(params);
            } catch (const svn::ClientException &e) {
                extraError(KIO::ERR_SLAVE_DEFINED, e.msg());
                return;
            }
            m_pData->dispWritten = false;
            stopOp(i18n("Temporary checkout done."));
            tmpfile = QSharedPointer<QFile>(new QFile(_codir->path() + url.fileName()));
            tmpfile->open(QIODevice::ReadWrite | QIODevice::Truncate);
        } else {
            extraError(KIO::ERR_FILE_ALREADY_EXIST, i18n("Could not write to existing item."));
            return;
        }
    } else {
        QTemporaryFile *_tmpfile = new QTemporaryFile();
        if (!_tmpfile->open()) {
            extraError(KIO::ERR_SLAVE_DEFINED, i18n("Could not open temporary file"));
            delete _tmpfile;
            return;
        }
        tmpfile = QSharedPointer<QFile>(_tmpfile);
    }
    int result = 0;
    QByteArray buffer;
    KIO::fileoffset_t processed_size = 0;
    do {
        dataReq();
        result = readData(buffer);
        if (result > 0) {
            tmpfile->write(buffer);
            processed_size += result;
            processedSize(processed_size);
        }
        buffer.clear();
    } while (result > 0);

    tmpfile->flush();


    if (result != 0) {
        error(KIO::ERR_ABORTED, i18n("Could not retrieve data for write."));
        return;
    }

    totalSize(processed_size);
    written(0);
    m_pData->dispWritten = true;
    registerToDaemon();
    startOp(processed_size, i18n("Committing %1", makeSvnPath(url).path()));
    bool err = false;
    if (exists) {
        svn::CommitParameter commit_parameters;
        commit_parameters.targets(svn::Targets(tmpfile->fileName())).message(getDefaultLog()).depth(svn::DepthEmpty).keepLocks(false);
        try {
            m_pData->m_Svnclient->commit(commit_parameters);
        } catch (const svn::ClientException &e) {
            extraError(KIO::ERR_SLAVE_DEFINED, e.msg());
            err = true;
        }
    } else  {
        try {
            m_pData->m_Svnclient->import(tmpfile->fileName(), svn::Url(makeSvnPath(url)), getDefaultLog(), svn::DepthEmpty, false, false);
        } catch (const svn::ClientException &e) {
            QString ex = e.msg();
            extraError(KIO::ERR_SLAVE_DEFINED, e.msg());
            err = true;
            stopOp(ex);
        }
    }
    m_pData->dispWritten = false;
    if (!err) {
        stopOp(i18n("Wrote %1 to repository", helpers::ByteToString(processed_size)));
        finished();
    }
}

void kio_svnProtocol::copy(const QUrl &src, const QUrl &dest, int permissions, KIO::JobFlags flags)
{
    Q_UNUSED(permissions);
    Q_UNUSED(flags);
    //bool force = flags&KIO::Overwrite;
    m_pData->resetListener();
    qCDebug(KDESVN_LOG) << "kio_svn::copy " << src << " to " << dest <<  endl;
    svn::Revision rev = m_pData->urlToRev(src);
    if (rev == svn::Revision::UNDEFINED) {
        rev = svn::Revision::HEAD;
    }
    m_pData->dispProgress = true;
    m_pData->m_CurrentContext->setLogMessage(getDefaultLog());
    try {
        m_pData->m_Svnclient->copy(makeSvnPath(src), rev, makeSvnPath(dest));
    } catch (const svn::ClientException &e) {
        if (e.apr_err() == SVN_ERR_ENTRY_EXISTS) {
            error(KIO::ERR_DIR_ALREADY_EXIST, e.msg());
        } else {
            extraError(KIO::ERR_SLAVE_DEFINED, e.msg());
        }
        qCDebug(KDESVN_LOG) << "kio_svn::copy aborted" <<  endl;
        return;
    }
    m_pData->dispProgress = false;
    qCDebug(KDESVN_LOG) << "kio_svn::copy finished" <<  endl;
    notify(i18n("Copied %1 to %2", makeSvnPath(src).path(), makeSvnPath(dest).path()));
    finished();
}

void kio_svnProtocol::del(const QUrl &src, bool isfile)
{
    Q_UNUSED(isfile);
    m_pData->resetListener();
    qCDebug(KDESVN_LOG) << "kio_svn::del " << src << endl;
    //m_pData->reInitClient();
    svn::Revision rev = m_pData->urlToRev(src);
    if (rev == svn::Revision::UNDEFINED) {
        rev = svn::Revision::HEAD;
    }
    if (rev != svn::Revision::HEAD) {
        extraError(KIO::ERR_SLAVE_DEFINED, i18n("Can only write on HEAD revision."));
        return;
    }
    m_pData->m_CurrentContext->setLogMessage(getDefaultLog());
    try {
        svn::Targets target(makeSvnPath(src));
        m_pData->m_Svnclient->remove(target, false);
    } catch (const svn::ClientException &e) {
        extraError(KIO::ERR_SLAVE_DEFINED, e.msg());
        qCDebug(KDESVN_LOG) << "kio_svn::del aborted" << endl;
        return;
    }
    qCDebug(KDESVN_LOG) << "kio_svn::del finished" << endl;
    finished();
}

bool kio_svnProtocol::getLogMsg(QString &t)
{
    svn::CommitItemList _items;
    return m_pData->m_Listener.contextGetLogMessage(t, _items);
}

bool kio_svnProtocol::checkWc(const svn::Path &localPath) const
{
    m_pData->resetListener();
    if (!localPath.isSet()) {
        return false;
    }
    svn::Revision peg(svn_opt_revision_unspecified);
    svn::Revision rev(svn_opt_revision_unspecified);
    svn::InfoEntries e;
    try {
        e = m_pData->m_Svnclient->info(localPath, svn::DepthEmpty, rev, peg);
    } catch (const svn::ClientException &e) {
        if (SVN_ERR_WC_NOT_DIRECTORY == e.apr_err()) {
            return false;
        }
        return true;
    }
    return false;
}

svn::Path kio_svnProtocol::makeSvnPath(const QUrl &url) const
{
    const QString scheme = svn::Url::transformProtokoll(url.scheme());
    if (scheme == QLatin1String("file")) {
        const svn::Path path(url.toLocalFile());
        if (checkWc(path)) {
            return path;
        }
    }
    if (url.path().isEmpty()) {
        throw svn::ClientException(QLatin1Char('\'') + url.url() + QLatin1String("' is not a valid subversion url"));
    }

    QUrl tmpUrl(url);
    tmpUrl.setScheme(scheme);
    tmpUrl.setQuery(QString()); // svn doesn't know anything about queries (e.g ?rev=X)

    return svn::Path(tmpUrl.toString(QUrl::NormalizePathSegments));
}

bool kio_svnProtocol::createUDSEntry(const QString &filename, const QString &user, long long int size, bool isdir, time_t mtime, KIO::UDSEntry &entry)
{
    entry.insert(KIO::UDSEntry::UDS_NAME, filename);
    entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, isdir ? S_IFDIR : S_IFREG);
    entry.insert(KIO::UDSEntry::UDS_SIZE, size);
    entry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, mtime);
    entry.insert(KIO::UDSEntry::UDS_USER, user);
    return true;
}

void kio_svnProtocol::special(const QByteArray &data)
{
    qCDebug(KDESVN_LOG) << "kio_svnProtocol::special" << endl;
    QByteArray tmpData(data);
    QDataStream stream(&tmpData, QIODevice::ReadOnly);
    m_pData->resetListener();
    int tmp;
    stream >> tmp;
    qCDebug(KDESVN_LOG) << "kio_svnProtocol::special " << tmp << endl;
    switch (tmp) {
    case SVN_CHECKOUT: {
        QUrl repository, wc;
        int revnumber;
        QString revkind;
        stream >> repository;
        stream >> wc;
        stream >> revnumber;
        stream >> revkind;
        qCDebug(KDESVN_LOG) << "kio_svnProtocol CHECKOUT from " << repository.url() << " to " << wc.url() << " at " << revnumber << " or " << revkind << endl;
        checkout(repository, wc, revnumber, revkind);
        break;
    }
    case SVN_UPDATE: {
        QUrl wc;
        int revnumber;
        QString revkind;
        stream >> wc;
        stream >> revnumber;
        stream >> revkind;
        qCDebug(KDESVN_LOG) << "kio_svnProtocol UPDATE " << wc.url() << " at " << revnumber << " or " << revkind << endl;
        update(wc, revnumber, revkind);
        break;
    }
    case SVN_COMMIT: {
        QList<QUrl> wclist;
        while (!stream.atEnd()) {
            QUrl tmp;
            stream >> tmp;
            wclist << tmp;
        }
        qCDebug(KDESVN_LOG) << "kio_svnProtocol COMMIT" << endl;
        commit(wclist);
        break;
    }
    case SVN_LOG: {
        qCDebug(KDESVN_LOG) << "kio_svnProtocol LOG" << endl;
        int revstart, revend;
        QString revkindstart, revkindend;
        QList<QUrl> targets;
        stream >> revstart;
        stream >> revkindstart;
        stream >> revend;
        stream >> revkindend;
        while (!stream.atEnd()) {
            QUrl tmp;
            stream >> tmp;
            targets << tmp;
        }
        svnlog(revstart, revkindstart, revend, revkindend, targets);
        break;
    }
    case SVN_IMPORT: {
        QUrl wc, repos;
        stream >> repos;
        stream >> wc;
        qCDebug(KDESVN_LOG) << "kio_ksvnProtocol IMPORT" << endl;
        import(repos, wc);
        break;
    }
    case SVN_ADD: {
        QUrl wc;
        qCDebug(KDESVN_LOG) << "kio_ksvnProtocol ADD" << endl;
        stream >> wc;
        add(wc);
        break;
    }
    case SVN_DEL: {
        QList<QUrl> wclist;
        while (!stream.atEnd()) {
            QUrl tmp;
            stream >> tmp;
            wclist << tmp;
        }
        wc_delete(wclist);
        break;
    }
    case SVN_REVERT: {
        QList<QUrl> wclist;
        while (!stream.atEnd()) {
            QUrl tmp;
            stream >> tmp;
            wclist << tmp;
        }
        qCDebug(KDESVN_LOG) << "kio_svnProtocol REVERT" << endl;
        revert(wclist);
        break;
    }
    case SVN_STATUS: {
        QUrl wc;
        bool checkRepos = false;
        bool fullRecurse = false;
        stream >> wc;
        stream >> checkRepos;
        stream >> fullRecurse;
        qCDebug(KDESVN_LOG) << "kio_svnProtocol STATUS" << endl;
        status(wc, checkRepos, fullRecurse);
        break;
    }
    case SVN_MKDIR: {
        QList<QUrl> list;
        stream >> list;
        qCDebug(KDESVN_LOG) << "kio_svnProtocol MKDIR" << endl;
        this->mkdir(list, 0);
        break;
    }
    case SVN_RESOLVE: {
        QUrl url;
        bool recurse;
        stream >> url;
        stream >> recurse;
        qCDebug(KDESVN_LOG) << "kio_svnProtocol RESOLVE" << endl;
        wc_resolve(url, recurse);
        break;
    }
    case SVN_SWITCH: {
        QUrl wc, url;
        bool recurse;
        int revnumber;
        QString revkind;
        stream >> wc;
        stream >> url;
        stream >> recurse;
        stream >> revnumber;
        stream >> revkind;
        qCDebug(KDESVN_LOG) << "kio_svnProtocol SWITCH" << endl;
        wc_switch(wc, url, recurse, revnumber, revkind);
        break;
    }
    case SVN_DIFF: {
        QUrl url1, url2;
        int rev1, rev2;
        bool recurse;
        QString revkind1, revkind2;
        stream >> url1;
        stream >> url2;
        stream >> rev1;
        stream >> revkind1;
        stream >> rev2;
        stream >> revkind2;
        stream >> recurse;
        diff(url1, url2, rev1, revkind1, rev2, revkind2, recurse);
        break;
    }
    default: {
        qCDebug(KDESVN_LOG) << "Unknown special" << endl;
    }
    }
    finished();
}

void kio_svnProtocol::update(const QUrl &url, int revnumber, const QString &revkind)
{
    svn::Revision where(revnumber, revkind);
    m_pData->resetListener();
    /* update is always local - so make a path instead URI */
    svn::Path p(url.path());
    try {
        svn::Targets pathes(p.path());
        // always update externals, too. (third last parameter)
        // no unversioned items allowed (second last parameter)
        // sticky depth (last parameter)
        svn::UpdateParameter _params;
        _params.targets(p.path()).revision(revnumber).depth(svn::DepthInfinity).ignore_externals(false).allow_unversioned(false).sticky_depth(true);
        m_pData->m_Svnclient->update(_params);
    } catch (const svn::ClientException &e) {
        extraError(KIO::ERR_SLAVE_DEFINED, e.msg());
    }
}

void kio_svnProtocol::status(const QUrl &wc, bool cR, bool rec)
{
    svn::StatusEntries dlist;
    svn::StatusParameter params(wc.path());
    m_pData->resetListener();
    try {
        dlist = m_pData->m_Svnclient->status(params.depth(rec ? svn::DepthInfinity : svn::DepthEmpty).all(false).update(cR).noIgnore(false).revision(svn::Revision::UNDEFINED));
    } catch (const svn::ClientException &e) {
        extraError(KIO::ERR_SLAVE_DEFINED, e.msg());
        return;
    }
    qCDebug(KDESVN_LOG) << "Status got " << dlist.count() << " entries." << endl;
    Q_FOREACH (const svn::StatusPtr &s, dlist) {
        if (!s) {
            continue;
        }
        const QString cntStr(QString::number(m_pData->m_Listener.counter()).rightJustified(10, QLatin1Char('0')));
        //QDataStream stream(params, QIODevice::WriteOnly);
        setMetaData(cntStr + QLatin1String("path"), s->path());
        setMetaData(cntStr + QLatin1String("node"), QString::number(s->nodeStatus()));
        setMetaData(cntStr + QLatin1String("text"), QString::number(s->textStatus()));
        setMetaData(cntStr + QLatin1String("prop"), QString::number(s->propStatus()));
        setMetaData(cntStr + QLatin1String("reptxt"), QString::number(s->reposTextStatus()));
        setMetaData(cntStr + QLatin1String("repprop"), QString::number(s->reposPropStatus()));
        setMetaData(cntStr + QLatin1String("rev"), QString::number(s->entry().cmtRev()));
        m_pData->m_Listener.incCounter();
    }
}

void kio_svnProtocol::commit(const QList<QUrl> &urls)
{
    /// @todo replace with direct call to kdesvn?
    QString msg;

    CON_DBUS;

    QDBusReply<QStringList> res = kdesvndInterface.get_logmsg();
    if (!res.isValid()) {
        qWarning() << "Unexpected reply type";
        return;
    }
    QStringList lt = res;

    if (lt.count() != 1) {
        msg = i18n("Wrong or missing log (may cancel pressed).");
        qCDebug(KDESVN_LOG) << msg << endl;
        return;
    }
    msg = lt[0];
    svn::Revision nnum = svn::Revision::UNDEFINED;
    svn::CommitParameter commit_parameters;
    commit_parameters.targets(svn::Targets::fromUrlList(urls, svn::Targets::UrlConversion::PreferLocalPath)).message(msg).depth(svn::DepthInfinity).keepLocks(false);

    try {
        nnum = m_pData->m_Svnclient->commit(commit_parameters);
    } catch (const svn::ClientException &e) {
        extraError(KIO::ERR_SLAVE_DEFINED, e.msg());
    }
    for (long j = 0; j < urls.count(); ++j) {
        QString userstring;
        if (nnum != svn::Revision::UNDEFINED) {
            userstring = i18n("Committed revision %1.", nnum.toString());
        } else {
            userstring = i18n("Nothing to commit.");
        }
        const QString num(QString::number(m_pData->m_Listener.counter()).rightJustified(10, QLatin1Char('0')));
        const QString zero(QStringLiteral("0"));
        setMetaData(num + QLatin1String("path"), urls[j].path());
        setMetaData(num + QLatin1String("action"), zero);
        setMetaData(num + QLatin1String("kind"), zero);
        setMetaData(num + QLatin1String("mime_t"), QString());
        setMetaData(num + QLatin1String("content"), zero);
        setMetaData(num + QLatin1String("prop"), zero);
        setMetaData(num + QLatin1String("rev") , QString::number(nnum));
        setMetaData(num + QLatin1String("string"), userstring);
        m_pData->m_Listener.incCounter();
    }
}

void kio_svnProtocol::checkout(const QUrl &src, const QUrl &target, const int rev, const QString &revstring)
{
    svn::Revision where(rev, revstring);
    try {
        svn::CheckoutParameter params;
        params.moduleName(makeSvnPath(src)).destination(target.path()).revision(where).peg(svn::Revision::UNDEFINED).depth(svn::DepthInfinity);
        m_pData->m_Svnclient->checkout(params);
    } catch (const svn::ClientException &e) {
        extraError(KIO::ERR_SLAVE_DEFINED, e.msg());
    }
}

void kio_svnProtocol::svnlog(int revstart, const QString &revstringstart, int revend, const QString &revstringend, const QList<QUrl> &urls)
{
    svn::Revision start(revstart, revstringstart);
    svn::Revision end(revend, revstringend);
    svn::LogParameter params;
    params.revisionRange(start, end).peg(svn::Revision::UNDEFINED).limit(0).discoverChangedPathes(true).strictNodeHistory(true);

    for (long j = 0; j < urls.count(); ++j) {
        svn::LogEntriesMap logs;
        try {
            m_pData->m_Svnclient->log(params.targets(makeSvnPath(urls[j])), logs);
        } catch (const svn::ClientException &e) {
            extraError(KIO::ERR_SLAVE_DEFINED, e.msg());
            break;
        }
        if (logs.isEmpty()) {
            const QString num(QString::number(m_pData->m_Listener.counter()).rightJustified(10, QLatin1Char('0')));
            setMetaData(num + QStringLiteral("path"), urls[j].path());
            setMetaData(num + QStringLiteral("string"),
                        i18n("Empty logs"));
            m_pData->m_Listener.incCounter();
            continue;
        }

        svn::LogEntriesMap::const_iterator it = logs.constBegin();
        for (; it != logs.constEnd(); ++it) {
            const QString num(QString::number(m_pData->m_Listener.counter()).rightJustified(10, QLatin1Char('0')));
            setMetaData(num + QStringLiteral("path"), urls[j].path());
            setMetaData(num + QStringLiteral("rev"), QString::number((*it).revision));
            setMetaData(num + QStringLiteral("author"), (*it).author);
            setMetaData(num + QStringLiteral("logmessage"), (*it).message);
            m_pData->m_Listener.incCounter();
            for (long z = 0; z < (*it).changedPaths.count(); ++z) {
                const QString num(QString::number(m_pData->m_Listener.counter()).rightJustified(10, QLatin1Char('0')));
                setMetaData(num + QStringLiteral("rev"), QString::number((*it).revision));
                setMetaData(num + QStringLiteral("path"), urls[j].path());
                setMetaData(num + QStringLiteral("loggedpath"), (*it).changedPaths[z].path);
                setMetaData(num + QStringLiteral("loggedaction"), QString(QLatin1Char((*it).changedPaths[z].action)));
                setMetaData(num + QStringLiteral("loggedcopyfrompath"), (*it).changedPaths[z].copyFromPath);
                setMetaData(num + QStringLiteral("loggedcopyfromrevision"), QString::number((*it).changedPaths[z].copyFromRevision));
                m_pData->m_Listener.incCounter();

            }
        }
    }
}

void kio_svnProtocol::revert(const QList<QUrl> &urls)
{
    try {
        m_pData->m_Svnclient->revert(svn::Targets::fromUrlList(urls, svn::Targets::UrlConversion::PreferLocalPath), svn::DepthEmpty);
    } catch (const svn::ClientException &e) {
        extraError(KIO::ERR_SLAVE_DEFINED, e.msg());
    }
}

void kio_svnProtocol::wc_switch(const QUrl &wc, const QUrl &target, bool rec, int rev, const QString &revstring)
{
    svn::Revision where(rev, revstring);
    svn::Path wc_path(wc.path());
    try {
        m_pData->m_Svnclient->doSwitch(wc_path, svn::Url(makeSvnPath(target)), where, rec ? svn::DepthInfinity : svn::DepthFiles);
    } catch (const svn::ClientException &e) {
        extraError(KIO::ERR_SLAVE_DEFINED, e.msg());
    }
}

void kio_svnProtocol::diff(const QUrl &uri1, const QUrl &uri2, int rnum1, const QString &rstring1, int rnum2, const QString &rstring2, bool rec)
{
    QByteArray ex;
    /// @todo read settings for diff (ignore contentype)
    try {
        const svn::Revision r1(rnum1, rstring1);
        const svn::Revision r2(rnum2, rstring2);
        const svn::Path u1 = makeSvnPath(uri1);
        const svn::Path u2 = makeSvnPath(uri2);
        QTemporaryDir tdir;
        qCDebug(KDESVN_LOG) << "kio_ksvn::diff : " << u1.path() << " at revision " << r1.toString() << " with "
                            << u2.path() << " at revision " << r2.toString()
                            << endl ;
        svn::DiffParameter _opts;
        // no peg revision required
        _opts.path1(u1).path2(u2).tmpPath(tdir.path()).
        rev1(r1).rev2(r2).
        ignoreContentType(false).extra(svn::StringArray()).depth(rec ? svn::DepthInfinity : svn::DepthEmpty).ignoreAncestry(false).noDiffDeleted(false).
        relativeTo((u1.path() == u2.path() ? u1 : svn::Path())).changeList(svn::StringArray());

        tdir.setAutoRemove(true);
        ex = m_pData->m_Svnclient->diff(_opts);
    } catch (const svn::ClientException &e) {
        extraError(KIO::ERR_SLAVE_DEFINED, e.msg());
        return;
    }
    QString out = QString::fromUtf8(ex);
    const QString num(QString::number(m_pData->m_Listener.counter()).rightJustified(10, QLatin1Char('0')));
    QTextStream stream(&out);
    while (!stream.atEnd()) {
        setMetaData(num + QStringLiteral("diffresult"), stream.readLine());
        m_pData->m_Listener.incCounter();
    }
}

void kio_svnProtocol::import(const QUrl &repos, const QUrl &wc)
{
    try {
        const svn::Path target = makeSvnPath(repos);
        const QString path = wc.path();
        m_pData->m_Svnclient->import(path, svn::Url(target), QString(), svn::DepthInfinity, false, false);
    } catch (const svn::ClientException &e) {
        extraError(KIO::ERR_SLAVE_DEFINED, e.msg());
        return;
    }
    finished();
}

void kio_svnProtocol::add(const QUrl &wc)
{
    QString path = wc.path();
    try {
        /* rec */
        m_pData->m_Svnclient->add(svn::Path(path), svn::DepthInfinity);
    } catch (const svn::ClientException &e) {
        extraError(KIO::ERR_SLAVE_DEFINED, e.msg());
        return;
    }
    finished();
}

void kio_svnProtocol::wc_delete(const QList<QUrl> &urls)
{
    try {
        m_pData->m_Svnclient->remove(svn::Targets::fromUrlList(urls, svn::Targets::UrlConversion::PreferLocalPath), false);
    } catch (const svn::ClientException &e) {
        extraError(KIO::ERR_SLAVE_DEFINED, e.msg());
        return;
    }
    finished();
}

void kio_svnProtocol::wc_resolve(const QUrl &url, bool recurse)
{
    try {
        svn::Depth depth = recurse ? svn::DepthInfinity : svn::DepthEmpty;
        m_pData->m_Svnclient->resolve(url.path(), depth);
    } catch (const svn::ClientException &e) {
        extraError(KIO::ERR_SLAVE_DEFINED, e.msg());
        return;
    }
    finished();
}

void kio_svnProtocol::streamWritten(const KIO::filesize_t current)
{
    processedSize(current);
}

void kio_svnProtocol::streamSendMime(const QMimeType &mt)
{
    if (mt.isValid()) {
        mimeType(mt.name());
    }
}

void kio_svnProtocol::streamPushData(const QByteArray &streamData)
{
    data(streamData);
}

void kio_svnProtocol::contextProgress(long long int current, long long int max)
{
    if (max > -1) {
        totalSize(KIO::filesize_t(max));
    }

    bool to_dbus = false;
    if (m_pData->dispProgress || m_pData->dispWritten || max > -1) {
        QTime now = QTime::currentTime();
        if (m_pData->m_last.msecsTo(now) >= 90) {
            if (m_pData->dispProgress) {
                processedSize(KIO::filesize_t(current));
            } else {
                written(current);
                to_dbus = useKioprogress();
            }
            m_pData->m_last = now;
        }
    }
    if (to_dbus) {
        CON_DBUS;
        if (max > -1) {
            kdesvndInterface.maxTransferKioOperation(m_pData->m_Id, max);
        }
        kdesvndInterface.transferredKioOperation(m_pData->m_Id, current);
    }
}

bool kio_svnProtocol::supportOverwrite()const
{
    Kdesvnsettings::self()->load();
    return Kdesvnsettings::kio_can_overwrite();
}

bool kio_svnProtocol::useKioprogress()const
{
    Kdesvnsettings::self()->load();
    return Kdesvnsettings::display_dockmsg();
}

/*!
    \fn kio_svnProtocol::getDefaultLog()
 */
QString kio_svnProtocol::getDefaultLog()
{
    QString res;
    Kdesvnsettings::self()->load();
    if (Kdesvnsettings::kio_use_standard_logmsg()) {
        res = Kdesvnsettings::kio_standard_logmsg();
    }
    return res;
}

void kio_svnProtocol::notify(const QString &text)
{
    if (!useKioprogress()) {
        return;
    }
    CON_DBUS;
    kdesvndInterface.notifyKioOperation(text);
}

void kio_svnProtocol::extraError(int _errid, const QString &text)
{
    error(_errid, text);
    if (!text.isNull()) {
        CON_DBUS;
        kdesvndInterface.errorKioOperation(text);
    }
}

void kio_svnProtocol::registerToDaemon()
{
    if (!useKioprogress()) {
        return;
    }
    CON_DBUS;
    kdesvndInterface.registerKioFeedback(m_pData->m_Id);
}

void kio_svnProtocol::unregisterFromDaemon()
{
    CON_DBUS;
    kdesvndInterface.unRegisterKioFeedback(m_pData->m_Id);
}
bool kio_svnProtocol::checkKioCancel()const
{
    if (!useKioprogress()) {
        return false;
    }
    CON_DBUS_VAL(false);
    QDBusReply<bool> res = kdesvndInterface.canceldKioOperation(m_pData->m_Id);
    return res.isValid() ? res.value() : false;
}

void kio_svnProtocol::startOp(qulonglong max, const QString &title)
{
    if (!useKioprogress()) {
        return;
    }
    CON_DBUS;
    kdesvndInterface.maxTransferKioOperation(m_pData->m_Id, max);
    kdesvndInterface.titleKioOperation(m_pData->m_Id, title, title);
    kdesvndInterface.setKioStatus(m_pData->m_Id, 1, QString());
}

void kio_svnProtocol::stopOp(const QString &message)
{
    if (!useKioprogress()) {
        return;
    }
    CON_DBUS;
    kdesvndInterface.setKioStatus(m_pData->m_Id, 0, message);
    unregisterFromDaemon();
}

} // namespace KIO
