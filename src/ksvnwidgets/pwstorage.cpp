/***************************************************************************
 *   Copyright (C) 2006-2007 by Rajko Albrecht                             *
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
#include "src/settings/kdesvnsettings.h"

#include <kwallet.h>
#include <kwin.h>
#include <kapp.h>

class PwStorageData
{
public:

    PwStorageData(){
        m_Wallet=0;
    }

    ~PwStorageData()
    {
        delete m_Wallet;
        m_Wallet=0;
    }

    // call only from connectWallet
    KWallet::Wallet*getWallet();
protected:
    KWallet::Wallet* m_Wallet;
};

KWallet::Wallet*PwStorageData::getWallet()
{
    static bool walletOpenFailed = false;
    if (m_Wallet && m_Wallet->isOpen()) {
        return m_Wallet;
    }

    if (KWallet::Wallet::isEnabled() && Kdesvnsettings::passwords_in_wallet()) {
        WId window = 0;
        if ( qApp->activeWindow() ) {
            window = qApp->activeWindow()->winId();
        }
        delete m_Wallet;
        m_Wallet = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(),window);
    }
    if (!m_Wallet) {
        walletOpenFailed = true;
    }
    if (!m_Wallet->hasFolder(WALLETNAME)) {
        m_Wallet->createFolder(WALLETNAME);
    }
    m_Wallet->setFolder(WALLETNAME);
    return m_Wallet;
}

PwStorage*PwStorage::self()
{
    static PwStorage*_me = 0;
    if (!_me) {
        _me = new PwStorage();
    }
    return _me;
}

/*!
    \fn PwStorage::PwStorageData()
 */
PwStorage::PwStorage()
    :QObject()
{
    mData = new PwStorageData;
}

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
    return mData->getWallet()!=0L;
}

/*!
    \fn PwStorage::getCertPw(const QString&realm,QString&pw)
 */
bool PwStorage::getCertPw(const QString&realm,QString&pw)
{
    if (!mData->getWallet()) {
        return false;
    }
    return (mData->getWallet()->readPassword(realm,pw)==0);
}


/*!
    \fn PwStorage::getLogin(const QString&realm,QString&user,QString&pw)
 */
bool PwStorage::getLogin(const QString&realm,QString&user,QString&pw)
{
    if (!mData->getWallet()) {
        return false;
    }
    QMap<QString,QString> content;
    int j = mData->getWallet()->readMap(realm,content);
    if (j!=0||content.find("user")==content.end()) {
        return false;
    }
    user = content["user"];
    pw = content["password"];
    return true;
}

/*!
    \fn PwStorage::setCertPw(const QString&realm, const QString&pw)
 */
bool PwStorage::setCertPw(const QString&realm, const QString&pw)
{
    if (!mData->getWallet()) {
        return false;
    }
    return (mData->getWallet()->writePassword(realm,pw)==0);
}


/*!
    \fn PwStorage::setLogin(const QString&realm,const QString&user,const QString&pw)
 */
bool PwStorage::setLogin(const QString&realm,const QString&user,const QString&pw)
{
    if (!mData->getWallet()) {
        return false;
    }
    QMap<QString,QString> content;
    content["user"]=user;
    content["password"]=pw;
    return (mData->getWallet()->writeMap(realm,content)==0);
}

#include "pwstorage.moc"
