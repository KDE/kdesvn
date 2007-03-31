#ifndef _PWSTORAGE_H
#define _PWSTORAGE_H

#include <qstring.h>

class PwStorageData;

class PwStorage
{
protected:
    PwStorageData* mData;
public:
    PwStorage();
    virtual ~PwStorage();
    bool getCertPw(const QString&realm,QString&pw);
    bool getLogin(const QString&realm,QString&user,QString&pw);
    bool setCertPw(const QString&realm, const QString&pw);
    bool setLogin(const QString&realm,const QString&user,const QString&pw);
protected:
    bool connectWallet();
    bool initWallet();
};

#endif

