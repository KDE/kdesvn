#ifndef REPOSITORYLISTENER_HPP
#define REPOSITORYLISTENER_HPP

/**
	@author Rajko Albrecht <ral@alwins-world.de>
*/

#include <qstring.h>

namespace svn {

namespace repository {

//! class for callbacks on repository operations
class RepositoryListener{

public:
    //! constructor
    RepositoryListener();
    //! destructor
    virtual ~RepositoryListener();

    //! sends a warning or informative message
    virtual void sendWarning(const QString&)=0;
    //! sends an error message
    virtual void sendError(const QString&)=0;
    //! check if running operation should cancelled
    virtual bool isCanceld() =0;

};

}

}

#endif
