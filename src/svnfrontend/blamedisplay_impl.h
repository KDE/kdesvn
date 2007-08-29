/***************************************************************************
 *   Copyright (C) 2006-2007 by Rajko Albrecht                             *
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
#ifndef BLAMEDISPLAY_IMPL_H
#define BLAMEDISPLAY_IMPL_H

#include "blamedisplay.h"
#include "src/svnqt/client.hpp"

class BlameDisplayData;
class SimpleLogCb;
class BlameDisplayItem;
class KListViewSearchLineWidget;

class BlameDisplay_impl:public BlameDisplay
{
    Q_OBJECT
public:
    BlameDisplay_impl(const QString&,const svn::AnnotatedFile&,QWidget*parent=0,const char*name=0);
    BlameDisplay_impl(QWidget*parent=0,const char*name=0);
    virtual ~BlameDisplay_impl();

    virtual void setContent(const QString&,const svn::AnnotatedFile&);
    virtual void setCb(SimpleLogCb*);

    const QColor rev2color(svn_revnum_t)const;

    static void displayBlame(SimpleLogCb*,const QString&,const svn::AnnotatedFile&,QWidget*parent=0,const char*name=0);

public slots:
    virtual void slotGoLine();
    virtual void slotShowCurrentCommit();

protected slots:
    virtual void slotContextMenuRequested(KListView*,QListViewItem*, const QPoint&);
    virtual void slotSelectionChanged();

protected:
    virtual void showCommit(BlameDisplayItem*);
    KListViewSearchLineWidget* m_SearchWidget;

private:
    BlameDisplayData*m_Data;
protected slots:
    virtual void slotItemDoubleClicked(QListViewItem*);
};

#endif
