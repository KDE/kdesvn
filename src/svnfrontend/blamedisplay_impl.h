/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
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
#ifndef BLAMEDISPLAY_IMPL_H
#define BLAMEDISPLAY_IMPL_H

#include "ui_blamedisplay.h"
#include "src/svnqt/client.hpp"

class BlameDisplayData;
class SimpleLogCb;
class BlameTreeItem;
class QAction;

class BlameDisplay_impl:public QWidget,public Ui::BlameDisplay
{
    Q_OBJECT
public:
    BlameDisplay_impl(QWidget*parent=0);
    virtual ~BlameDisplay_impl();

    virtual void setContent(const QString&,const svn::AnnotatedFile&);
    virtual void setCb(SimpleLogCb*);

    const QColor rev2color(svn_revnum_t)const;
    static void displayBlame(SimpleLogCb*,const QString&,const svn::AnnotatedFile&,QWidget*parent=0);

public slots:
    virtual void slotGoLine();
    virtual void slotShowCurrentCommit();

protected slots:
    virtual void slotCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*);
    virtual void slotTextCodecChanged(const QString&);

protected:
    virtual void showCommit(BlameTreeItem*);

private:
    BlameDisplayData*m_Data;
protected slots:
    virtual void slotItemDoubleClicked(QTreeWidgetItem*,int);
};

#endif
