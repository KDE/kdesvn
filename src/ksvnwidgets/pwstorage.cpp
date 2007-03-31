#include "pwstorage.h"
#include "kdesvn-config.h"
#include "src/settings/kdesvnsettings.h"

#include <kwallet.h>

class PwStorageData
{
public:
    KWallet::Wallet *m_Wallet;
    PwStorageData(){m_Wallet = 0;}
    ~PwStorageData(){}
};


/*!
    \fn PwStorage::PwStorageData()
 */
 PwStorage::PwStorage()
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
    if (!Kdesvnsettings::passwords_in_wallet()) {
        return false;
    }
    if (!mData->m_Wallet) {
        mData->m_Wallet = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(),0);
    }
    if (!mData->m_Wallet) {
        return false;
    }
    return true;
}


/*!
    \fn PwStorage::getCertPw(const QString&realm,QString&pw)
 */
bool PwStorage::getCertPw(const QString&realm,QString&pw)
{
    if (!connectWallet()) {
        return false;
    }
    return (mData->m_Wallet->readPassword(realm,pw)==0);
}


/*!
    \fn PwStorage::getLogin(const QString&realm,QString&user,QString&pw)
 */
bool PwStorage::getLogin(const QString&realm,QString&user,QString&pw)
{
    if (!connectWallet()) {
        return false;
    }
    if (mData->m_Wallet->hasFolder(WALLETNAME) ) {
        mData->m_Wallet->setFolder(WALLETNAME);
        QMap<QString,QString> content;
        int j = mData->m_Wallet->readMap(realm,content);
        if (j!=0||content.find("user")==content.end()) {
            return false;
        }
        user = content["user"];
        pw = content["password"];
        return true;
    }
    return false;
}


/*!
    \fn PwStorage::setCertPw(const QString&realm, const QString&pw)
 */
bool PwStorage::setCertPw(const QString&realm, const QString&pw)
{
    if (!initWallet()) {
        return false;
    }
    return (mData->m_Wallet->writePassword(realm,pw)==0);
}


/*!
    \fn PwStorage::setLogin(const QString&realm,const QString&user,const QString&pw)
 */
bool PwStorage::setLogin(const QString&realm,const QString&user,const QString&pw)
{
    if (!initWallet()) {
        return false;
    }
    QMap<QString,QString> content;
    content["user"]=user;
    content["password"]=pw;
    return (mData->m_Wallet->writeMap(realm,content)==0);
}


/*!
    \fn PwStorage::initWallet()
 */
bool PwStorage::initWallet()
{
    if (!connectWallet()) {
        return false;
    }
    if (!mData->m_Wallet->hasFolder(WALLETNAME)) {
        mData->m_Wallet->createFolder(WALLETNAME);
    }
    if (!mData->m_Wallet->setFolder(WALLETNAME)) {
        return false;
    }
    return true;
}
