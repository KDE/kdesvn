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
#pragma once

#include "ksvnwidgets/ksvndialog.h"
#include "svnqt/client.h"

namespace Ui
{
class BlameDisplay;
}
class BlameTreeItem;
class BlameDisplayData;
class QTreeWidgetItem;
class SimpleLogCb;

class BlameDisplay: public KSvnDialog
{
    Q_OBJECT
private:
    explicit BlameDisplay(const QString &what, const svn::AnnotatedFile &blame, SimpleLogCb *cb, QWidget *parent = nullptr);
    ~BlameDisplay();

    void setContent(const QString &what, const svn::AnnotatedFile &blame);
    void showCommit(BlameTreeItem *bti);
public:
    static void displayBlame(SimpleLogCb *_cb, const QString &item, const svn::AnnotatedFile &blame, QWidget *parent);

private Q_SLOTS:
    void slotGoLine();
    void slotShowCurrentCommit();
    void slotItemDoubleClicked(QTreeWidgetItem *item, int);
    void slotCurrentItemChanged(QTreeWidgetItem *item, QTreeWidgetItem *);
    void slotTextCodecChanged(const QString &what);

private:
    Ui::BlameDisplay *m_ui;
    BlameDisplayData *m_Data;

};

