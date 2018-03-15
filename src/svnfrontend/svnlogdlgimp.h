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
#ifndef SVNLOGDLGIMP_H
#define SVNLOGDLGIMP_H

#include "ui_svnlogdlg.h"
#include "simple_logcb.h"
#include "svnqt/log_entry.h"
#include "svnqt/client.h"

#include <QRegExp>

class QDialogButtonBox;
class QKeyEvent;
class QTreeWidgetItem;
class SvnActions;
class SvnLogModel;
class QSortFilterProxyModel;
class QModelIndex;

class SvnLogDlgImp: public QDialog, public Ui::LogDialog, public SimpleLogCb
{
    Q_OBJECT
public:
    SvnLogDlgImp(SvnActions *ac, bool modal, QWidget *parent = nullptr);
    ~SvnLogDlgImp();
    void dispLog(const svn::LogEntriesMapPtr &log, const QString &what, const QString &root, const svn::Revision &peg, const QString &pegUrl);
    void saveSize();
    bool getSingleLog(svn::LogEntry &t, const svn::Revision &r, const QString &what, const svn::Revision &peg, QString &root) override;

signals:
    void makeDiff(const QString &, const svn::Revision &, const QString &, const svn::Revision &, QWidget *);
    void makeCat(const svn::Revision &, const QString &, const QString &, const svn::Revision &, QWidget *);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void showEvent(QShowEvent *e) override;

protected slots:
    void slotDispPrevious();
    void slotDispSelected();
    void slotRevisionSelected();
    void slotPrevFifty();
    void slotBeginHead();
    void slotHelpRequested();

private:
    QString _name;
    QString _base;
    SvnActions *m_Actions;
    bool m_ControlKeyDown;
    svn::LogEntriesMapPtr m_Entries;
    SvnLogModel *m_CurrentModel;
    QSortFilterProxyModel *m_SortModel;

    QString _bugurl;

    void dispLog(const svn::LogEntriesMapPtr &);

    QRegExp _r1, _r2;

protected slots:
    void slotListEntries();
    void slotChangedPathContextMenu(const QPoint &);
    void slotSingleDoubleClicked(QTreeWidgetItem *, int);
    void slotGetLogs();
    void slotBlameItem();
    void slotSelectionChanged(const QItemSelection &, const QItemSelection &);
    void slotCustomContextMenu(const QPoint &);

protected:
    /* it works 'cause we use single selection only */
    QModelIndex selectedRow(int column = 0);
    void replaceBugids(QString &msg);
    QString genReplace(const QString &);
    svn::Revision m_peg;
    svn::Path m_PegUrl;
};

#endif
