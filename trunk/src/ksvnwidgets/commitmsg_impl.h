/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
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
#ifndef COMMITMSG_IMPL_H
#define COMMITMSG_IMPL_H

#include "src/ksvnwidgets/ui_commitmessage.h"
#include "src/ksvnwidgets/models/commitmodelfwd.h"
#include "src/svnqt/commititem.h"

#include <QList>
#include <QPair>

class QStringList;
class CommitModel;
class QSortFilterProxyModel;

class Commitmsg_impl: public QWidget, Ui::CommitMessage {
  Q_OBJECT

protected:
    Commitmsg_impl(const svn::CommitItemList&_items,QWidget *parent=0);
    Commitmsg_impl(const QMap<QString,QString>&_items,QWidget *parent=0);
    Commitmsg_impl(const CommitActionEntries&,const CommitActionEntries&,QWidget *parent = 0);
public:
    Commitmsg_impl(QWidget *parent = 0);
    virtual ~Commitmsg_impl();

    QString getMessage()const;
    bool isKeeplocks()const;
    void initHistory();
    void saveHistory(bool canceld);
    void keepsLocks(bool);

    static QString getLogmessage(bool*ok,svn::Depth*rec,bool*keeps_locks,QWidget*parent=0);
    static QString getLogmessage(const svn::CommitItemList&,bool*ok,svn::Depth*rec,bool*keep_locks,QWidget*parent=0);
    static QString getLogmessage(const QMap<QString,QString>&,bool*ok,svn::Depth*rec,bool*keep_locks,QWidget*parent=0);

    static QString getLogmessage(const CommitActionEntries&,const CommitActionEntries&,
            QObject*callback,
            CommitActionEntries&,
            bool*ok,bool*keep_locks,QWidget*parent=0);

    void addItemWidget(QWidget*);

    svn::Depth getDepth()const;

    CommitActionEntries checkedEntries();
    void hideDepth(bool ahide);

    CommitModelNodePtr currentCommitItem(int column=0);

protected Q_SLOTS:
    virtual void slotHistoryActivated(int);
    virtual void slotUnmarkUnversioned();
    virtual void slotDiffSelected();
    virtual void slotRevertSelected();
    virtual void slotMarkUnversioned();
    virtual void hideNewItems(bool);
    virtual void insertFile();
    virtual void slotItemReverted(const QStringList&);
    virtual void slotItemDoubleClicked(const QModelIndex&);
    virtual void slotSelectAll();

protected:
    static QStringList sLogHistory;
    static const QString groupName;
    static QString sLastMessage;
    static int smax_message_history;
    bool m_hidden;

    void hideButtons(bool);
    void markUnversioned(bool mark);
    void checkSplitterSize();
    void setupModel();
    virtual void insertFile(const QString&);
    virtual void hideKeepsLock(bool);

    CommitModel*m_CurrentModel;
    QSortFilterProxyModel*m_SortModel;

Q_SIGNALS:
    void makeDiff(const QString&,const svn::Revision&,const QString&,const svn::Revision&,QWidget*);
    void sigRevertItem(const QStringList&,bool);
};

#endif
