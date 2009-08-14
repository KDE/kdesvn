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
#ifndef _LOG_CACHE_HPP
#define _LOG_CACHE_HPP

#include <QString>
#include <QDir>
#include <QVariant>

#include "svnqt/svnqt_defines.hpp"
#include "svnqt/shared_pointer.hpp"

namespace svn {
    class Path;
    namespace cache {

        class LogCacheData;

        class SVNQT_EXPORT LogCache
        {
        private:
            svn::SharedPointer<LogCacheData> m_CacheData;

        protected:
            LogCache();
            static LogCache* mSelf;
            QString m_BasePath;
            static QString s_CACHE_FOLDER;
            void setupCachePath();
            void setupMainDb();
            int databaseVersion()const;
            void databaseVersion(int newversion);

        public:
            ///! should used for testing only!
            LogCache(const QString&aBasePath);
            virtual ~LogCache();
            static LogCache* self();
            QDataBase reposDb(const QString&aRepository);
            QStringList cachedRepositories()const;

            bool valid()const;

            QVariant getRepositoryParameter(const svn::Path&repository,const QString&key)const;
            //! set or delete parameter
            /*!
             * if value is invalid the parameter will removed from database
             */
            bool setRepositoryParameter(const svn::Path&repository,const QString&key,const QVariant&value);
            bool deleteRepository(const QString&aRepository);
        };
    }
}

#endif
