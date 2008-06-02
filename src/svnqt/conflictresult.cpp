/***************************************************************************
 *   Copyright (C) 2007 by Rajko Albrecht  ral@alwins-world.de             *
 *   http://kdesvn.alwins-world.de/                                        *
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

#include "conflictresult.hpp"

#include "svnqt_defines.hpp"

#include <svn_wc.h>

namespace svn
{
    ConflictResult::ConflictResult()
    {
    }

    ConflictResult::ConflictResult(const svn_wc_conflict_result_t*aResult)
    {
        if (!aResult){
            return;
        }
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
        switch (aResult->choice){
            case svn_wc_conflict_choose_base:
                m_choice=ChooseBase;
                break;
            case svn_wc_conflict_choose_theirs_full:
                m_choice=ChooseTheirsFull;
                break;
            case svn_wc_conflict_choose_mine_full:
                m_choice=ChooseMineFull;
                break;
            case svn_wc_conflict_choose_theirs_conflict:
                m_choice=ChooseTheirsConflict;
                break;
            case svn_wc_conflict_choose_mine_conflict:
                m_choice=ChooseMineConflict;
                break;
            case svn_wc_conflict_choose_merged:
                m_choice=ChooseMerged;
                break;
            case svn_wc_conflict_choose_postpone:
            default:
                m_choice=ChoosePostpone;
                break;
        }
        if (aResult->merged_file) {
            m_MergedFile=QString::FROMUTF8(aResult->merged_file);
        } else {
            m_MergedFile=QString::null;
        }
#else
        Q_UNUSED(aResult);
#endif
    }

    void ConflictResult::setMergedFile(const QString&aMergedfile) {
        m_MergedFile=aMergedfile;
    }

    void ConflictResult::setChoice(ConflictChoice aValue)
    {
        m_choice=aValue;
    }
}
