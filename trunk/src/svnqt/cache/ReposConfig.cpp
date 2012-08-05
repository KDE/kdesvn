/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
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
#include "ReposConfig.h"
#include "LogCache.h"
#include "src/svnqt/path.h"

namespace svn
{
namespace cache
{

class ReposConfigPrivate
{
public:
    static QByteArray serializeList(const QList<QByteArray> &list);
    static QStringList deserializeList(const QByteArray &data);
    static QVariant convertToQVariant(const QByteArray& value, const QVariant& aDefault);
};

static QList<int> asIntList(const QByteArray& string)
{
    QList<int> list;
    Q_FOREACH(const QByteArray& s, string.split(','))
        list << s.toInt();
    return list;
}

QByteArray ReposConfigPrivate::serializeList(const QList<QByteArray> &list)
{
    QByteArray value = "";

    if (!list.isEmpty()) {
        QList<QByteArray>::ConstIterator it = list.constBegin();
        const QList<QByteArray>::ConstIterator end = list.constEnd();

        value = QByteArray(*it).replace('\\', "\\\\").replace(',', "\\,");

        while (++it != end) {
            // In the loop, so it is not done when there is only one element.
            // Doing it repeatedly is a pretty cheap operation.
            value.reserve(4096);

            value += ',';
            value += QByteArray(*it).replace('\\', "\\\\").replace(',', "\\,");
        }

        // To be able to distinguish an empty list from a list with one empty element.
        if (value.isEmpty())
            value = "\\0";
    }

    return value;
}

QStringList ReposConfigPrivate::deserializeList(const QByteArray &data)
{
    if (data.isEmpty())
        return QStringList();
    if (data == "\\0")
        return QStringList(QString());
    QStringList value;
    QString val;
    val.reserve(data.size());
    bool quoted = false;
    for (int p = 0; p < data.length(); p++) {
        if (quoted) {
            val += data[p];
            quoted = false;
        } else if (data[p] == '\\') {
            quoted = true;
        } else if (data[p] == ',') {
            val.squeeze(); // release any unused memory
            value.append(val);
            val.clear();
            val.reserve(data.size() - p);
        } else {
            val += data[p];
        }
    }
    value.append(val);
    return value;
}

QVariant ReposConfigPrivate::convertToQVariant(const QByteArray& value, const QVariant& aDefault)
{
    // if a type handler is added here you must add a QVConversions definition
    // to conversion_check.h, or ConversionCheck::to_QVariant will not allow
    // readEntry<T> to convert to QVariant.
    switch( aDefault.type() ) {
        case QVariant::Invalid:
            return QVariant();
        case QVariant::String:
            // this should return the raw string not the dollar expanded string.
            // imho if processed string is wanted should call
            // readEntry(key, QString) not readEntry(key, QVariant)
            return QString::fromUtf8(value);
        case QVariant::List:
        case QVariant::StringList:
            return deserializeList(value);
        case QVariant::ByteArray:
            return value;
        case QVariant::Bool: {
            const QByteArray lower(value.toLower());
            if (lower == "false" || lower == "no" || lower == "off" || lower == "0")
                return false;
            return true;
        }
        case QVariant::Double:
        case QMetaType::Float:
        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::LongLong:
        case QVariant::ULongLong: {
            QVariant tmp = value;
            if ( !tmp.convert(aDefault.type()) )
                tmp = aDefault;
            return tmp;
        }
        case QVariant::DateTime: {
            const QList<int> list = asIntList(value);
            if ( list.count() != 6 ) {
                return aDefault;
            }
            const QDate date( list.at( 0 ), list.at( 1 ), list.at( 2 ) );
            const QTime time( list.at( 3 ), list.at( 4 ), list.at( 5 ) );
            const QDateTime dt( date, time );
            if ( !dt.isValid() ) {
                return aDefault;
            }
            return dt;
        }
        case QVariant::Date: {
            QList<int> list = asIntList(value);
            if ( list.count() == 6 )
                list = list.mid(0, 3); // don't break config files that stored QDate as QDateTime
            if ( list.count() != 3 ) {
                return aDefault;
            }
            const QDate date( list.at( 0 ), list.at( 1 ), list.at( 2 ) );
            if ( !date.isValid() ) {
                return aDefault;
            }
            return date;
        }
        default:
            break;
    }

    qWarning("unhandled type %s",aDefault.typeName());
    return QVariant();
}

ReposConfig* ReposConfig::mSelf = 0;

ReposConfig*ReposConfig::self()
{
    if (!mSelf) {
        mSelf=new ReposConfig();
    }
    return mSelf;
}

ReposConfig::ReposConfig()
{
}


void ReposConfig::setValue(const QString&repository,const QString&key,const QVariant&value)
{

    QByteArray data;
    switch( value.type() ) {
        case QVariant::Invalid:
            data = "";
            break;
        case QVariant::ByteArray:
            data = value.toByteArray();
            break;
        case QVariant::String:
        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::Double:
        case QMetaType::Float:
        case QVariant::Bool:
        case QVariant::LongLong:
        case QVariant::ULongLong:
            data = value.toString().toUtf8();
            break;
        case QVariant::List:
        case QVariant::StringList:
            setValue(repository, key, value.toList());
            return;
        case QVariant::Date: {
            QVariantList list;
            const QDate date = value.toDate();

            list.insert( 0, date.year() );
            list.insert( 1, date.month() );
            list.insert( 2, date.day() );

            setValue(repository, key, list);
            return;
        }
        case QVariant::DateTime: {
            QVariantList list;
            const QDateTime rDateTime = value.toDateTime();

            const QTime time = rDateTime.time();
            const QDate date = rDateTime.date();

            list.insert( 0, date.year() );
            list.insert( 1, date.month() );
            list.insert( 2, date.day() );

            list.insert( 3, time.hour() );
            list.insert( 4, time.minute() );
            list.insert( 5, time.second() );

            setValue(repository, key, list);
            return;
        }

        default:
            qWarning("ReposConfig: Unhandled type");
            return;
        }

        svn::cache::LogCache::self()->setRepositoryParameter(svn::Path(repository),key,data);
}

void ReposConfig::eraseValue(const QString&repository,const QString&key)
{
    svn::cache::LogCache::self()->setRepositoryParameter(svn::Path(repository),key,QVariant());
}

void ReposConfig::setValue(const QString&repository,const QString&key,const QVariantList&list)
{
    QList<QByteArray> data;

    foreach(const QVariant& v, list) {
        if (v.type() == QVariant::ByteArray)
            data << v.toByteArray();
        else
            data << v.toString().toUtf8();
    }

    setValue(repository, key, ReposConfigPrivate::serializeList(data));
}

void ReposConfig::setValue(const QString&repository,const QString&key,const QStringList&list)
{
    QList<QByteArray> balist;
    foreach(const QString &entry, list) {
        balist.append(entry.toUtf8());
    }
    setValue(repository,key,ReposConfigPrivate::serializeList(balist));
}

void ReposConfig::setValue(const QString&repository,const QString&key,const QString&value)
{
    setValue(repository,key,value.toUtf8());
}

QVariant ReposConfig::readEntry(const QString&repository,const QString&key, const QVariant&aDefault)
{
    QVariant v = svn::cache::LogCache::self()->getRepositoryParameter(svn::Path(repository),key);
    if (!v.isValid()) {
        return aDefault;
    }
    return ReposConfigPrivate::convertToQVariant(v.toByteArray(),aDefault);
}

int ReposConfig::readEntry(const QString&repository,const QString&key,int aDefault)
{
    return readEntry(repository,key,QVariant(aDefault)).toInt();
}

bool ReposConfig::readEntry(const QString&repository,const QString&key,bool aDefault)
{
    return readEntry(repository,key,QVariant(aDefault)).toBool();
}

QStringList ReposConfig::readEntry(const QString&repository,const QString&key,const QStringList&aDefault)
{
    return readEntry(repository,key,QVariant(aDefault)).toStringList();
}

} // namespace cache
} // namespace svn
