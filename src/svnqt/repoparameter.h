/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
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
#ifndef REPOPARAMETER_H
#define REPOPARAMETER_H

#include <QString>

#include "svnqt/shared_pointer.h"
#include "svnqt/svnqt_defines.h"

namespace svn {

namespace repository {

struct CreateRepoParameterData;

class SVNQT_EXPORT CreateRepoParameter
{
    SharedPointer<CreateRepoParameterData> _data;

public:
    CreateRepoParameter();
    ~CreateRepoParameter();

    /** path to create
     * default is emtpy
     */
    const QString&path()const;
    /** path to create
     * default is emtpy
     */
    CreateRepoParameter&path(const QString&);
    /** fs type of repository
     *
     * default is "fsfs"
     */
    const QString&fstype()const;
    /** fs type of repository
     *
     * default is "fsfs"
     */
    CreateRepoParameter&fstype(const QString&);
    /** switch of syncing of bdb
     *
     * default is false
     */
    bool bdbnosync()const;
    /** switch of syncing of bdb
     *
     * default is false
     */
    CreateRepoParameter&bdbnosync(bool);
    /** bdb automatic remove log
     *
     * default is true
     */
    bool bdbautologremove()const;
    /** bdb automatic remove log
     *
     * default is true
     */
    CreateRepoParameter&bdbautologremove(bool);
    /** default is false */
    bool pre14_compat()const;
    /** default is false */
    CreateRepoParameter&pre14_compat(bool);
    /** default is false */
    bool pre15_compat()const;
    /** default is false */
    CreateRepoParameter&pre15_compat(bool);
    /** default is false */
    bool pre16_compat()const;
    /** default is false */
    CreateRepoParameter&pre16_compat(bool);

};

} // namespace repository
} // namespace svn
#endif
