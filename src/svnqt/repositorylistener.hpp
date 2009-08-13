/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
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
#ifndef REPOSITORYLISTENER_HPP
#define REPOSITORYLISTENER_HPP

/**
	@author Rajko Albrecht <ral@alwins-world.de>
*/

#include "svnqt/svnqt_defines.hpp"
#include <qstring.h>

namespace svn {

namespace repository {

//! class for callbacks on repository operations
class SVNQT_EXPORT RepositoryListener{

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
