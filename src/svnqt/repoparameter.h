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
 * history and logs, available at https://commits.kde.org/kdesvn.          *
 ***************************************************************************/
#pragma once

#include <QString>
#include <svnqt/svnqt_defines.h>

namespace svn
{

namespace repository
{

class CreateRepoParameterData;

class SVNQT_EXPORT CreateRepoParameter
{
    QString _path;
    QString _fstype;
    bool _bdbnosync;
    bool _bdbautologremove;
    bool _pre_1_5_compat;
    bool _pre_1_6_compat;
    bool _pre_1_8_compat;

public:
    CreateRepoParameter()
        : _fstype(QLatin1String("fsfs"))
        , _bdbnosync(false)
        , _bdbautologremove(true)
        , _pre_1_5_compat(false)
        , _pre_1_6_compat(false)
        , _pre_1_8_compat(false)
    {}

    /** path to create
     * default is emtpy
     */
    const QString &path() const { return _path; }
    /** path to create
     * default is emtpy
     */
    CreateRepoParameter &path(const QString &path) { _path = path; return *this; }
    /** fs type of repository
     *
     * default is "fsfs"
     */
    const QString &fstype() const { return _fstype; }
    /** fs type of repository
     *
     * default is "fsfs"
     */
    CreateRepoParameter &fstype(const QString &fstype) { _fstype = fstype; return *this; }
    /** switch of syncing of bdb
     *
     * default is false
     */
    bool bdbnosync() const { return _bdbnosync; }
    /** switch of syncing of bdb
     *
     * default is false
     */
    CreateRepoParameter &bdbnosync(bool b) { _bdbnosync = b; return *this; }
    /** bdb automatic remove log
     *
     * default is true
     */
    bool bdbautologremove() const { return _bdbautologremove; }
    /** bdb automatic remove log
     *
     * default is true
     */
    CreateRepoParameter &bdbautologremove(bool b) { _bdbautologremove = b; return *this; }

    /** default is false */
    bool pre15_compat() const { return _pre_1_5_compat; }
    /** default is false */
    CreateRepoParameter &pre15_compat(bool b) { _pre_1_5_compat = b; return *this; }
    /** default is false */
    bool pre16_compat() const { return _pre_1_6_compat; }
    /** default is false */
    CreateRepoParameter &pre16_compat(bool b) { _pre_1_6_compat = b; return *this; }
    /** default is false */
    bool pre18_compat() const { return _pre_1_8_compat; }
    /** default is false */
    CreateRepoParameter &pre18_compat(bool b) { _pre_1_8_compat = b; return *this; }

};

} // namespace repository
} // namespace svn
