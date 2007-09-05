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
#ifndef STOPDLG_H
#define STOPDLG_H

#include "src/svnfrontend/fronthelpers/cursorstack.h"

#include <kdialogbase.h>

#include <qdatetime.h>
#include <qobject.h>

class QTimer;

class CContextListener;
class QLabel;
class KProgress;
class KTextBrowser;
class QVBoxLayout;

/**
@author Rajko Albrecht
*/
class StopDlg : public KDialogBase
{
Q_OBJECT
public:
    StopDlg(QObject*,QWidget *parent = 0, const char *name = 0,const QString&caption=QString::null,const QString&text=QString::null);
    virtual ~StopDlg();

    bool cancelld();

protected:
    QObject*m_Context;
    int m_MinDuration;
    bool mCancelled;
    QTimer * mShowTimer;
    QString mCancelText;
    bool mShown,mWait;
    QLabel*mLabel;
    KProgress*m_ProgressBar;
    KProgress*m_NetBar;
    bool m_BarShown;
    bool m_netBarShown;
    QTime m_StopTick;
    KTextBrowser*m_LogWindow;
    QVBoxLayout*layout;

    QString m_lastLog;
    unsigned int m_lastLogLines;
    CursorStack * cstack;

    virtual void showEvent(QShowEvent*);
    virtual void hideEvent(QHideEvent*);

public slots:
    virtual void slotTick();
    virtual void slotWait(bool);
    virtual void slotExtraMessage(const QString&);

protected slots:
    virtual void slotAutoShow();
    virtual void slotCancel();
    virtual void slotNetProgres(long long int, long long int);
signals:
    void sigCancel(bool how);
};

class StopSimpleDlg:public StopDlg
{
    Q_OBJECT
public:
    StopSimpleDlg(QWidget *parent = 0, const char *name = 0,const QString&caption=QString::null,const QString&text=QString::null);
    virtual ~StopSimpleDlg(){}

    bool isCanceld()const{return cancelld;}

public slots:
    virtual void makeCancel();

protected slots:
    virtual void slotSimpleCancel(bool);

protected:
    bool cancelld;
};

#endif
