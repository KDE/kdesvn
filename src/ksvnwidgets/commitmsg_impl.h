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

#include "ksvnwidgets/ui_commitmessage.h"
#include "ksvnwidgets/models/commitmodelfwd.h"
#include "svnqt/commititem.h"


class QStringList;
class CommitModel;
class CommitFilterModel;

class Commitmsg_impl: public QWidget, Ui::CommitMessage
{
    Q_OBJECT

protected:
    explicit Commitmsg_impl(const svn::CommitItemList &_items, QWidget *parent = nullptr);
    explicit Commitmsg_impl(const CommitActionEntries &, const CommitActionEntries &, QWidget *parent = nullptr);
public:
    explicit Commitmsg_impl(QWidget *parent = nullptr);
    ~Commitmsg_impl();

    QString getMessage()const;
    bool isKeeplocks()const;
    void initHistory();
    void saveHistory(bool canceld);
    void keepsLocks(bool);

    static QString getLogmessage(bool *ok, svn::Depth *rec, bool *keep_locks, QWidget *parent = nullptr);
    static QString getLogmessage(const svn::CommitItemList &, bool *ok, svn::Depth *rec, bool *keep_locks, QWidget *parent = nullptr);

    static QString getLogmessage(const CommitActionEntries &, const CommitActionEntries &,
                                 QObject *callback,
                                 CommitActionEntries &,
                                 bool *ok, bool *keep_locks, QWidget *parent = nullptr);

    void addItemWidget(QWidget *);

    svn::Depth getDepth()const;

    CommitActionEntries checkedEntries();
    void hideDepth(bool ahide);

    CommitModelNodePtr currentCommitItem(int column = 0);

private:
    static QString getLogmessageInternal(Commitmsg_impl *ptr, bool *ok, svn::Depth *rec, bool *keep_locks, CommitActionEntries *result, QWidget *parent);
protected Q_SLOTS:
    void slotHistoryActivated(int);
    void slotUnmarkUnversioned();
    void slotDiffSelected();
    void slotRevertSelected();
    void slotMarkUnversioned();
    void hideNewItems(bool hide);
    void insertFile();
    void slotItemReverted(const QStringList &);
    void slotItemDoubleClicked(const QModelIndex &index);
    void slotCurrentItemChanged(const QModelIndex &current);
    void slotSelectAll();
    void slotUnselectAll();

protected:
    static QStringList sLogHistory;
    static QString sLastMessage;
    static int smax_message_history;
    bool m_hidden;

    void hideButtons(bool);
    void markUnversioned(bool mark);
    void checkSplitterSize();
    void setupModel();
    virtual void insertFile(const QString &);
    virtual void hideKeepsLock(bool);

    CommitModel *m_CurrentModel;
    CommitFilterModel *m_SortModel;

Q_SIGNALS:
    void makeDiff(const QString &, const svn::Revision &, const QString &, const svn::Revision &, QWidget *);
    void sigRevertItem(const QStringList &);
};

#endif
