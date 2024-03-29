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
#ifndef KIOSVN_H
#define KIOSVN_H

#include "kiobytestream.h"

#include <qstring.h>

#include <KIO/WorkerBase>

#include <sys/stat.h>

namespace svn
{
class DirEntry;
class Path;
}

namespace KIO
{

class KioSvnData;

/**
@author Rajko Albrecht
*/
class kio_svnProtocol : public KIO::WorkerBase, public StreamWrittenCb
{
public:
    kio_svnProtocol(const QByteArray &pool_socket, const QByteArray &app_socket);
    ~kio_svnProtocol() override;
    // KIO::WorkerBase
    KIO::WorkerResult listDir(const QUrl &url) override;
    KIO::WorkerResult stat(const QUrl &url) override;
    KIO::WorkerResult get(const QUrl &url) override;
    KIO::WorkerResult mkdir(const QUrl &url, int permissions) override;
    KIO::WorkerResult put(const QUrl &url, int permissions, KIO::JobFlags flags) override;
    KIO::WorkerResult rename(const QUrl &src, const QUrl &target, KIO::JobFlags flags) override;
    KIO::WorkerResult del(const QUrl &url, bool isfile) override;
    KIO::WorkerResult copy(const QUrl &src, const QUrl &dest, int permissions, KIO::JobFlags flags) override;
    KIO::WorkerResult special(const QByteArray &data) override;
    // StreamWrittenCb
    void streamWritten(const KIO::filesize_t current) override;
    void streamPushData(const QByteArray &streamData) override;
    void streamSendMime(const QMimeType &mt) override;

    void contextProgress(long long int current, long long int max);
    void listSendDirEntry(const svn::DirEntry &);
    bool checkKioCancel() const;

protected:
    Q_REQUIRED_RESULT KIO::WorkerResult checkout(const QUrl &src, const QUrl &target, const int rev, const QString &revstring);
    Q_REQUIRED_RESULT KIO::WorkerResult update(const QUrl &url, int revnumber, const QString &revkind);
    Q_REQUIRED_RESULT KIO::WorkerResult commit(const QList<QUrl> &urls);
    Q_REQUIRED_RESULT KIO::WorkerResult svnlog(int revstart, const QString &revstringstart, int revend, const QString &revstringend, const QList<QUrl> &urls);
    Q_REQUIRED_RESULT KIO::WorkerResult import(const QUrl &repos, const QUrl &wc);
    Q_REQUIRED_RESULT KIO::WorkerResult add(const QUrl &wc);
    Q_REQUIRED_RESULT KIO::WorkerResult wc_delete(const QList<QUrl> &urls);
    Q_REQUIRED_RESULT KIO::WorkerResult revert(const QList<QUrl> &urls);
    Q_REQUIRED_RESULT KIO::WorkerResult status(const QUrl &wc, bool cR, bool rec);
    Q_REQUIRED_RESULT KIO::WorkerResult mkdir(const QList<QUrl> &urls, int permissions);
    Q_REQUIRED_RESULT KIO::WorkerResult wc_resolve(const QUrl &url, bool recurse);
    Q_REQUIRED_RESULT KIO::WorkerResult wc_switch(const QUrl &wc, const QUrl &target, bool rec, int rev, const QString &revstring);
    Q_REQUIRED_RESULT KIO::WorkerResult
    diff(const QUrl &uri1, const QUrl &uri2, int rnum1, const QString &rstring1, int rnum2, const QString &rstring2, bool rec);
    /* looked on kio::svn from kdesdk */
    enum KSVN_METHOD {
        /* QUrl repository, QUrl target, int revnumber, QString revkind */
        SVN_CHECKOUT = 1,
        /* QUrl wc, int revnumber, QString revkind */
        /* refkind may empty or HEAD or START, will get parsed if revnumber is -1 */
        SVN_UPDATE = 2,
        /* QList<QUrl> */
        SVN_COMMIT = 3,
        /* int revstart, QString revstartstring, int revend, QString revendstring, QList<QUrl> */
        SVN_LOG = 4,
        SVN_IMPORT = 5,
        /* QUrl */
        SVN_ADD = 6,
        /* QList<QUrl> */
        SVN_DEL = 7,
        /* QList<QUrl> */
        SVN_REVERT = 8,
        /* QUrl wc,bool checkRepos, bool recurse */
        SVN_STATUS = 9,
        /* QList<QUrl> */
        SVN_MKDIR = 10,
        /* QUrl, bool */
        SVN_RESOLVE = 11,
        /* QUrl working copy, QUrl new_repository_url, bool recurse, int rev, QString revstring */
        SVN_SWITCH = 12,
        /* QUrl uri1, QUrl uri2, int r1, QString rstring1, int r2, QString rstring 2, bool recursive */
        SVN_DIFF = 13
    };

    void notify(const QString &text);
    Q_REQUIRED_RESULT KIO::WorkerResult extraError(int _errid, const QString &text);

private:
    KioSvnData *m_pData;
    static KIO::UDSEntry createUDSEntry(const QString &filename, const QString &user, long long int size, bool isdir, const QDateTime &mtime);
    svn::Path makeSvnPath(const QUrl &url) const;
    bool checkWc(const svn::Path &localPath) const;
    bool getLogMsg(QString &);

    void registerToDaemon();
    void unregisterFromDaemon();
    void startOp(qulonglong max, const QString &title);
    void stopOp(const QString &message);

protected:
    QString getDefaultLog();
    bool supportOverwrite() const;
    bool useKioprogress() const;
};

}

#endif
