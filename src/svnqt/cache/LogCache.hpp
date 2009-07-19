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
