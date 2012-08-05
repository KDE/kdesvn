/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   http://kdesvn.alwins-world.de/                                        *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this program (in the file LGPL.txt); if not,         *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at http://kdesvn.alwins-world.de.           *
 ***************************************************************************/

/***************************************************************************
 * Lot of code and ideas taken from KDE 4 KConfigGroup source code         *
 * ksvn://anonsvn.kde.org/home/kde/branches/KDE/4.2/kdelibs/kdecore/config *
 ***************************************************************************/

#ifndef REPOSCONFIG_HPP
#define REPOSCONFIG_HPP

#include <QVariant>
#include <QString>
#include <QList>

#include "svnqt/cache/conversion_check.h"
#include "svnqt/svnqt_defines.h"

namespace svn
{
namespace cache
{

class SVNQT_EXPORT ReposConfig
{

private:
    static ReposConfig* mSelf;
protected:
    ReposConfig();
    template <typename T> void writeCheck(const QString&repository,const QString&key, const T &value);
    template <typename T> void writeListCheck(const QString&repository,const QString&key, const QList<T> &value);

public:
    static ReposConfig*self();

    template<typename T> void setValue(const QString&repository,const QString&key,const T&value);
    template<typename T> void setValue(const QString&repository,const QString&key,const QList<T>&value);
    void setValue(const QString&repository,const QString&key,const QStringList&value);
    void setValue(const QString&repository,const QString&key,const QVariant&value);
    void setValue(const QString&repository,const QString&key,const QVariantList&list);
    void setValue(const QString&repository,const QString&key,const QString&value);

    //! special setter
    void eraseValue(const QString&repository,const QString&key);

    QVariant readEntry(const QString&repository,const QString&key, const QVariant&aDefault);
    int readEntry(const QString&repository,const QString&key,int aDefault);
    bool readEntry(const QString&repository,const QString&key,bool aDefault);
    QStringList readEntry(const QString&repository,const QString&key,const QStringList&aDefault);
};

template<typename T> inline void ReposConfig::setValue(const QString&repository,const QString&key,const T&value)
{
    writeCheck(repository,key,value);
}

template <typename T> inline
void ReposConfig::writeCheck(const QString&repository,const QString&key, const T &value)
{
     ConversionCheck::to_QVariant<T>();
     setValue(repository,key, qVariantFromValue(value));
}

template<typename T> inline
void ReposConfig::setValue(const QString&repository,const QString&key,const QList<T>&value)
{
    writeListCheck(repository,key,value);
}

template <typename T> inline
void ReposConfig::writeListCheck(const QString&repository,const QString&key, const QList<T> & list)
{
    ConversionCheck::to_QVariant<T>();
    ConversionCheck::to_QString<T>();
    QVariantList data;
    Q_FOREACH(const T &value, list) {
        data.append(qVariantFromValue(value));
    }
    setValue(repository, key, data);
}

} // namespace cache
} // namespace svn

#endif
