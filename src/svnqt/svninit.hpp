#include <qglobal.h>

#ifndef SVN_INIT_HPP
#define SVN_INIT_HPP

namespace svn
{
  //! this namespace contains only internal stuff not for public use
  namespace internal {
    //! small helper class
    /*!
        There will be an static instance created for calling the constructor at program load.
     */
    class SvnInit
    {
    public:
        //! constructor calling initialize functions
        SvnInit();
        ~SvnInit(){};
        static void initsvn();
    };
  }
}

#endif
