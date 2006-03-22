#ifndef __VERSION_CHECK_HPP
#define __VERSION_CHECK_HPP

class QString;

#define SVNQT_MAJOR 0
#define SVNQT_MINOR 8
#define SVNQT_PATCH 0

#define SVNQT_VERSIONSTRING "0.8.0"

namespace svn {
    class Version {

    public:
        Version(){}
        ~Version(){}

        static bool client_version_compatible();
        static const QString linked_version();
        static const QString running_version();

        static int version_major();
        static int version_minor();
    };
}

namespace svnqt {
    class SvnqtVersion {

    public:
        struct SvnqtVersionTag{
            int _major;
            int _minor;
            int _patch;
        };

        SvnqtVersion();
        static int version_major();
        static int version_minor();
        static int version_patch();
        static bool compatible(const SvnqtVersionTag&running);
    };
}

#endif
