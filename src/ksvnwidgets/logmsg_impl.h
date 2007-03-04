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
#ifndef LOGMSG_IMPL_H
#define LOGMSG_IMPL_H

#include "src/ksvnwidgets/logmessage.h"
#include "src/svnqt/commititem.hpp"
#include <qvaluelist.h>
#include <qpair.h>

class Logmsg_impl: public LogmessageData {
Q_OBJECT
public:
    struct logActionEntry {
        QString _name;
        QString _actionDesc;
        int _kind;
        logActionEntry(const QString&,const QString&,int kind = 0);
        logActionEntry();
    };

    typedef QValueList<logActionEntry> logActionEntries;

    Logmsg_impl(QWidget *parent = 0, const char *name = 0);
    Logmsg_impl(const svn::CommitItemList&_items,QWidget *parent=0, const char *name=0);
    Logmsg_impl(const QMap<QString,QString>&_items,QWidget *parent=0, const char *name=0);
    Logmsg_impl(const logActionEntries&,
        const logActionEntries&,
        QWidget *parent = 0, const char *name = 0);
    virtual ~Logmsg_impl();

    QString getMessage()const;
    bool isRecursive()const;
    void initHistory();
    void saveHistory();

    static QString getLogmessage(bool*ok=0,bool*rec=0,QWidget*parent=0,const char*name=0);
    static QString getLogmessage(const svn::CommitItemList&,bool*ok=0,bool*rec=0,QWidget*parent=0,const char*name=0);
    static QString getLogmessage(const QMap<QString,QString>&,bool*ok=0,bool*rec=0,QWidget*parent=0,const char*name=0);

    static QString getLogmessage(const logActionEntries&,
            const logActionEntries&,
            QObject*callback,
            logActionEntries&,
            bool*ok=0,QWidget*parent=0,const char*name=0);

    void setRecCheckboxtext(const QString&what,bool checked=true);

    logActionEntries selectedEntries();

protected slots:
    virtual void slotHistoryActivated(int);
    virtual void slotUnmarkUnversioned();
    virtual void slotDiffSelected();
    virtual void slotMarkUnversioned();

protected:
    static QValueList<QString> sLogHistory;
    static const QString groupName;
    static unsigned int smax_message_history;
    bool m_hidden;

    void hideButtons(bool);
    void markUnversioned(bool mark);
    void checkSplitterSize();
signals:
    void makeDiff(const QString&,const svn::Revision&,const QString&,const svn::Revision&,QWidget*);
};

#endif
