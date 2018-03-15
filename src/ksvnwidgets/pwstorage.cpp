/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
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
#include "pwstorage.h"
#include "kdesvn-config.h"
#include "settings/kdesvnsettings.h"

#include <KWallet>

#include <QApplication>
#include <QWidget>
#include <QMutex>
#include <QMap>
#include <QPair>

class PwStorageData
{
public:

    PwStorageData()
    {
        m_Wallet = nullptr;
    }

    ~PwStorageData()
    {
        delete m_Wallet;
        m_Wallet = nullptr;
    }

    KWallet::Wallet *getWallet();

    typedef QPair<QString, QString> userpw_type;
    typedef QMap<QString, userpw_type> cache_type;

    cache_type *getLoginCache();

    QMutex *getCacheMutex();

protected:
    KWallet::Wallet *m_Wallet;

};

QMutex *PwStorageData::getCacheMutex()
{
    static QMutex _mutex;
    return &_mutex;
}

PwStorageData::cache_type *PwStorageData::getLoginCache()
{
    static PwStorageData::cache_type _LoginCache;
    return &_LoginCache;
}

KWallet::Wallet *PwStorageData::getWallet()
{
    if ((m_Wallet && m_Wallet->isOpen()) || !qApp) {
        return m_Wallet;
    }
    if (KWallet::Wallet::isEnabled()) {
        WId window = 0;
        if (QApplication::activeModalWidget()) {
            window = QApplication::activeModalWidget()->winId();
        } else if (QApplication::activeWindow()) {
            window = QApplication::activeWindow()->winId();
        }
        delete m_Wallet;
        m_Wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), window);
    }
    if (m_Wallet) {
        if (!m_Wallet->hasFolder(QStringLiteral(WALLETNAME))) {
            m_Wallet->createFolder(QStringLiteral(WALLETNAME));
        }
        m_Wallet->setFolder(QStringLiteral(WALLETNAME));
    }
    return m_Wallet;
}

PwStorage *PwStorage::self()
{
    static PwStorage *_me = nullptr;
    if (!_me) {
        _me = new PwStorage();
    }
    return _me;
}

/*!
    \fn PwStorage::PwStorageData()
 */
PwStorage::PwStorage()
    : mData(new PwStorageData)
{}

/*!
    \fn PwStorage::~PwStorageData()
 */
PwStorage::~PwStorage()
{
    delete mData;
}

/*!
    \fn PwStorage::connectWallet()
 */
bool PwStorage::connectWallet()
{
    return mData->getWallet() != nullptr;
}

/*!
    \fn PwStorage::getCertPw(const QString&realm,QString&pw)
 */
bool PwStorage::getCertPw(const QString &realm, QString &pw)
{
    if (!mData->getWallet()) {
        return false;
    }
    return (mData->getWallet()->readPassword(realm, pw) == 0);
}

/*!
    \fn PwStorage::getLogin(const QString&realm,QString&user,QString&pw)
 */
bool PwStorage::getLogin(const QString &realm, QString &user, QString &pw)
{
    if (!mData->getWallet()) {
        return false;
    }
    QMap<QString, QString> content;
    int j = mData->getWallet()->readMap(realm, content);
    if (j != 0 || !content.contains(QStringLiteral("user"))) {
        return true;
    }
    user = content[QStringLiteral("user")];
    pw = content[QStringLiteral("password")];
    return true;
}

bool PwStorage::getCachedLogin(const QString &realm, QString &user, QString &pw)
{
    QMutexLocker lc(mData->getCacheMutex());
    PwStorageData::cache_type::ConstIterator it = mData->getLoginCache()->constFind(realm);
    if (it != mData->getLoginCache()->constEnd()) {
        user = (*it).first;
        pw = (*it).second;
    }
    return true;
}

/*!
    \fn PwStorage::setCertPw(const QString&realm, const QString&pw)
 */
bool PwStorage::setCertPw(const QString &realm, const QString &pw)
{
    if (!mData->getWallet()) {
        return false;
    }
    return (mData->getWallet()->writePassword(realm, pw) == 0);
}

/*!
    \fn PwStorage::setLogin(const QString&realm,const QString&user,const QString&pw)
 */
bool PwStorage::setLogin(const QString &realm, const QString &user, const QString &pw)
{
    if (!mData->getWallet()) {
        return false;
    }
    QMap<QString, QString> content;
    content[QStringLiteral("user")] = user;
    content[QStringLiteral("password")] = pw;
    return (mData->getWallet()->writeMap(realm, content) == 0);
}

bool PwStorage::setCachedLogin(const QString &realm, const QString &user, const QString &pw)
{
    QMutexLocker lc(mData->getCacheMutex());
    PwStorageData::cache_type *_Cache = mData->getLoginCache();
    (*_Cache)[realm] = PwStorageData::userpw_type(user, pw);
    return true;
}
