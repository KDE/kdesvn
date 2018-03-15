/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   http://kdesvn.alwins-world.de/                                        *
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

#ifndef kdesvnd_H
#define kdesvnd_H

#include "ksvnwidgets/jobviewserverinterface.h"

#include <KDEDModule>
#include <QDBusVariant>
#include <QString>
#include <QStringList>
#include <QUrl>

class KdesvndListener;
class KsvnJobView;

class kdesvnd : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kdesvnd")

public:
    kdesvnd(QObject *parent, const QList<QVariant> &);
    ~kdesvnd();

protected:
    bool isWorkingCopy(const QUrl &url) const;
    bool isRepository(const QUrl &url) const;
    static QString cleanUrl(const QUrl &url);
    KdesvndListener *m_Listener;
    QStringList getActionMenu(const QList<QUrl> &list, bool toplevel) const;

    org::kde::JobViewServer m_uiserver;

    QHash<qulonglong, KsvnJobView *> progressJobView;

public Q_SLOTS:
    //! get a subversion login
    /*!
    * \param realm the realm
    * \param user default username
    * \return a stringlist containing username-password-saveit as "true" or "false" or empty list if cancel hit.
    */
    QStringList get_login(const QString &, const QString &);
    //! get a saved subversion login
    /*!
    * \param realm the realm
    * \param user default username
    * \return a stringlist containing username-password
    */
    QStringList get_saved_login(const QString &realm, const QString &user);

    // return: -1 don't accept 0 accept temporary 1 accept always
    //               hostname, fingerprint, validFrom, validUntil, issuerDName, realm,
    int get_sslaccept(const QString &, const QString &, const QString &, const QString &, const QString &, const QString &);

    // returns cert file or empty string
    QString get_sslclientcertfile() const;
    // return a logmessage at pos 0, null-size list if cancel hit
    QStringList get_logmsg() const;

    // return pw loaded from wallet if existent
    QString load_sslclientcertpw(const QString &realm);
    // return pw at pos 0, maysafe at pos 1, null-size if cancel hit.
    QStringList get_sslclientcertpw(const QString &);
    QStringList getActionMenu(const QStringList &urlList) const;
    QStringList getTopLevelActionMenu(const QStringList &urlList) const;
    QStringList getSingleActionMenu(const QString &) const;

    bool canceldKioOperation(qulonglong kioid);
    void maxTransferKioOperation(qulonglong kioid, qulonglong maxtransfer);
    void registerKioFeedback(qulonglong kioid);
    void titleKioOperation(qulonglong kioid, const QString &title, const QString &label);
    void transferredKioOperation(qulonglong kioid, qulonglong transferred);
    void unRegisterKioFeedback(qulonglong kioid);
    void notifyKioOperation(const QString &text);
    void errorKioOperation(const QString &text);
    //! set status from KIO
    /*!
     * \param kioid the kio makes an action
     * \param status the status to set: 0 - stopped (terminated) 1 - running 2 - canceld (terminated)
     * \param message a message to print when not running
     */
    void setKioStatus(qulonglong kioid, int status, const QString &message);
};
#endif
