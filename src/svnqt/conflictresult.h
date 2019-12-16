/***************************************************************************
 *   Copyright (C) 2007-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   https://kde.org/applications/development/org.kde.kdesvn               *
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

#ifndef SVNQT_CONFLICT_RESULT_H
#define SVNQT_CONFLICT_RESULT_H

struct svn_wc_conflict_result_t;

#include <svnqt/pool.h>
#include <svnqt/svnqt_defines.h>
#include <svn_types.h>

#include <QString>

namespace svn
{

class SVNQT_EXPORT ConflictResult
{
public:
    enum ConflictChoice {
        //! let user make a call to resolve
        ChoosePostpone,
        ChooseBase,
        ChooseTheirsFull,
        ChooseMineFull,
        ChooseTheirsConflict,
        ChooseMineConflict,
        ChooseMerged
    };
    ConflictResult();
    //! Copy constructor
    /*! only useful wenn build with subversion 1.5 or newer
     */
    explicit ConflictResult(const svn_wc_conflict_result_t *);

    const QString &mergedFile()const
    {
        return m_MergedFile;
    }
    void setMergedFile(const QString &aMergedfile);

    ConflictChoice choice()const
    {
        return m_choice;
    }
    void setChoice(ConflictChoice aValue);

    const svn_wc_conflict_result_t *result(apr_pool_t *pool)const;
    void assignResult(svn_wc_conflict_result_t **aResult, apr_pool_t *pool)const;

protected:
    ConflictChoice m_choice;
    //! Merged file
    /*! will only used if m_choice is ChooseMerged
     */
    QString m_MergedFile;
};

}

#endif
