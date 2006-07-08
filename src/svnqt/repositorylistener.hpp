#ifndef REPOSITORYLISTENER_HPP
#define REPOSITORYLISTENER_HPP

/**
	@author Rajko Albrecht <ral@alwins-world.de>
*/

#include <qstring.h>

namespace svn {

class RepositoryListener{

public:
    RepositoryListener();
    virtual ~RepositoryListener();

    virtual void sendWarning(const QString&)=0;
    virtual void sendError(const QString&)=0;
    virtual bool isCanceld() =0;

};

}

#endif
