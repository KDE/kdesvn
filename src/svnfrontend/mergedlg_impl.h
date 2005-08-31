/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht                                  *
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
#ifndef MERGEDLG_IMPL_H
#define MERGEDLG_IMPL_H

#include "merge_dlg.h"
#include "rangeinput_impl.h"

class MergeDlg_impl: public MergeDlg {
Q_OBJECT
public:
    MergeDlg_impl(QWidget *parent = 0, const char *name = 0,bool src1=true,bool src2=true,bool out=true);
    virtual ~MergeDlg_impl();

    bool recursive()const;
    bool force()const;
    bool ignorerelated()const;
    bool dryrun()const;

    QString Src1()const;
    QString Src2()const;
    QString Dest()const;
    Rangeinput_impl::revision_range getRange()const;
    static bool getMergeRange(Rangeinput_impl::revision_range&range,
        bool*force,bool*recursive,bool*ignorerelated,bool*dry,QWidget*parent=0,const char*name=0);

};

#endif
