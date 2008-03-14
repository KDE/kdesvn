#ifndef _LOG_CACHE_HPP
#define _LOG_CACHE_HPP

#include <qstring.h>
#include <qdir.h>

#include "svnqt/svnqt_defines.hpp"
#include "svnqt/shared_pointer.hpp"

namespace svn {
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

        public:
            ///! should used for testing only!
            LogCache(const QString&aBasePath);
            virtual ~LogCache();
            static LogCache* self();
            QDataBase reposDb(const QString&aRepository);
        };
    }
}

#endif
