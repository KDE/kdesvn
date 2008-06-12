/***************************************************************************
 *   Copyright (C) 2005-2007 by Rajko Albrecht                             *
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
#ifndef LOGMSG_IMPL_H
#define LOGMSG_IMPL_H

#include "src/ksvnwidgets/ui_logmessage.h"
#include "src/svnqt/commititem.hpp"
#include <q3valuelist.h>
#include <qpair.h>

class Logmsg_impl: public QDialog, Ui::LogmessageData {
  Q_OBJECT
public:
    struct logActionEntry {
        QString _name;
        QString _actionDesc;

        enum ACTION_TYPE{
            COMMIT=0,
            ADD_COMMIT=1,
            DELETE=2,
            MISSING_DELETE=3
        };
        ACTION_TYPE _kind;
        logActionEntry(const QString&,const QString&,ACTION_TYPE kind = COMMIT);
        logActionEntry();
    };

    typedef Q3ValueList<logActionEntry> logActionEntries;

    Logmsg_impl(QWidget *parent = 0, const char *name = 0);
    Logmsg_impl(const svn::CommitItemList&_items,QWidget *parent=0, const char *name=0);
    Logmsg_impl(const QMap<QString,QString>&_items,QWidget *parent=0, const char *name=0);
    Logmsg_impl(const logActionEntries&,
        const logActionEntries&,
        QWidget *parent = 0, const char *name = 0);
    virtual ~Logmsg_impl();

    QString getMessage()const;
    bool isKeeplocks()const;
    void initHistory();
    void saveHistory(bool canceld);

    static QString getLogmessage(bool*ok,svn::Depth*rec,bool*keeps_locks,QWidget*parent=0,const char*name=0);
    static QString getLogmessage(const svn::CommitItemList&,bool*ok,svn::Depth*rec,bool*keep_locks,QWidget*parent=0,const char*name=0);
    static QString getLogmessage(const QMap<QString,QString>&,bool*ok,svn::Depth*rec,bool*keep_locks,QWidget*parent=0,const char*name=0);

    static QString getLogmessage(const logActionEntries&,
            const logActionEntries&,
            QObject*callback,
            logActionEntries&,
            bool*ok,bool*keep_locks,QWidget*parent=0,const char*name=0);

    void addItemWidget(QWidget*);

    svn::Depth getDepth()const;

    logActionEntries selectedEntries();
    void hideDepth(bool ahide);

protected slots:
    virtual void slotHistoryActivated(int);
    virtual void slotUnmarkUnversioned();
    virtual void slotDiffSelected();
    virtual void slotMarkUnversioned();
    virtual void hideNewItems(bool);

protected:
    static Q3ValueList<QString> sLogHistory;
    QValueList<QListViewItem*> m_Hidden;
    static const QString groupName;
    static QString sLastMessage;
    static unsigned int smax_message_history;
    bool m_hidden;

    void hideButtons(bool);
    void markUnversioned(bool mark);
    void checkSplitterSize();
signals:
    void makeDiff(const QString&,const svn::Revision&,const QString&,const svn::Revision&,QWidget*);
};

#endif
