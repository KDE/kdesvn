/*
    Copyright (C) 2010  Rajko Albrecht
    ral@alwins-world.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef CLIENT_UPDATE_PARAMETER_H
#define CLIENT_UPDATE_PARAMETER_H

#include "svnqt/svnqt_defines.h"
#include "svnqt/svnqttypes.h"
#include "svnqt/revision.h"
#include "svnqt/targets.h"
#include "svnqt/shared_pointer.h"

namespace svn{

    struct UpdateParameterData;

    class SVNQT_EXPORT UpdateParameter
    {
        private:
            SharedPointer<UpdateParameterData> _data;
            UpdateParameter();
            ~UpdateParameter();

            const Targets&targets()const;
            UpdateParameter&targets(const Targets&_target);

            const Revision&revision()const;
            UpdateParameter&revision(const Revision&rev);

            Depth depth()const;
            UpdateParameter&depth(Depth depth);

            bool ignore_externals()const;
            UpdateParameter&ignore_externals(bool);

            bool allow_unversioned()const;
            UpdateParameter&allow_unversioned(bool);

            bool sticky_depth()const;
            UpdateParameter&sticky_depth(bool);

    };
}

#endif // CLIENT_UPDATE_PARAMETER_H
