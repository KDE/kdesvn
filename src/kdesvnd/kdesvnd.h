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

#include "src/ksvnwidgets/jobviewserverinterface.h"

#include <qstringlist.h>
#include <qstring.h>
#include <kurl.h>
#include <kdedmodule.h>
#include <QDBusVariant>
#include <kcomponentdata.h>

class KdesvndListener;
class KsvnJobView;

class kdesvnd :  public KDEDModule
{
    Q_OBJECT
public:
    kdesvnd(QObject *parent, const QList<QVariant> &);
    virtual ~kdesvnd();

protected:
    bool isWorkingCopy(const KUrl &url, QString &base);
    bool isRepository(const KUrl &url);
    static QString cleanUrl(const KUrl &url);
    KdesvndListener *m_Listener;
    QStringList getActionMenu(const KUrl::List &, bool toplevel);
    KComponentData m_componentData;

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
    QString get_sslclientcertfile();
    // return a logmessage at pos 0, null-size list if cancel hit
    QStringList get_logmsg();

    // return pw loaded from wallet if existent
    QString load_sslclientcertpw(const QString &realm);
    // return pw at pos 0, maysafe at pos 1, null-size if cancel hit.
    QStringList get_sslclientcertpw(const QString &);
    QStringList getActionMenu(const KUrl::List &);
    QStringList getTopLevelActionMenu(const KUrl::List &);
    QStringList getSingleActionMenu(const QString &);

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
