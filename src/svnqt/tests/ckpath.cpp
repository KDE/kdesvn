#include "src/svnqt/path.hpp"
#include <qstring.h>
#include "svnqt/svnqt_defines.hpp"
#include <iostream>

int main(int,char**)
{
    svn::Path pa("/test/foo/bar/");
    if (pa.path()!=QString("/test/foo/bar")) {
        std::cout << "No cleanup of components" << std::endl;
        return -1;
    }
    pa.removeLast();
    if (pa.path()!=QString("/test/foo")) {
        std::cout<<"removeLast didn't work." << std::endl;
        return -1;
    }
    unsigned j = 0;
    while (pa.length()>0) {
        std::cout << pa.path().TOASCII().data() << std::endl;
        pa.removeLast();
        ++j;
    }
    return 0;
}

