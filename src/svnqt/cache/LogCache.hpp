#ifndef _LOG_CACHE_HPP
#define _LOG_CACHE_HPP

#include <qstring.h>
#include <qdir.h>

#include "svnqt/svnqt_defines.hpp"

namespace svn {
    namespace cache {
        class SVNQT_EXPORT LogCache
        {
        protected:
            QString m_BasePath;
            static QString s_CACHE_FOLDER;
            void setupCachePath();
            void setupMainDb();

        public:
             LogCache();
             LogCache(const QString&aBasePath);
             virtual ~LogCache();
        };
    }
}

#endif
