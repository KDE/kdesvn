/***************************************************************************
 *   Copyright (C) 2007-2010 by Rajko Albrecht  ral@alwins-world.de        *
 *   http://kdesvn.alwins-world.de/                                        *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this program (in the file LGPL.txt); if not,         *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at http://kdesvn.alwins-world.de.           *
 ***************************************************************************/

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
            //! internal data
            SharedPointer<UpdateParameterData> _data;

        public:
            //! constructor
            UpdateParameter();
            //! non-virtual destructor
            ~UpdateParameter();

            //! returns the targets for update
            const Targets&targets()const;
            //! set the targets for update
            UpdateParameter&targets(const Targets&_target);

            //! returns the revision the update should work on
            const Revision&revision()const;
            //! set the revision the update should work on
            UpdateParameter&revision(const Revision&rev);

            //! return depth of update operation
            /*!
             * \sa svn::Depth
             */
            Depth depth()const;
            //! set depth of update operation
            /*!
             * \sa svn::Depth
             */
            UpdateParameter&depth(Depth depth);

            //! return if update should ignore external definitions
            bool ignore_externals()const;
            //! set if update should ignore external definitions
            UpdateParameter&ignore_externals(bool);

            bool allow_unversioned()const;
            UpdateParameter&allow_unversioned(bool);

            bool sticky_depth()const;
            UpdateParameter&sticky_depth(bool);
            
            bool make_parents()const;
            UpdateParameter&make_parents(bool);
            
            bool add_as_modification()const;
            UpdateParameter&add_as_modification(bool);
            
    };
}

#endif // CLIENT_UPDATE_PARAMETER_H
