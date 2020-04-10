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
#ifndef STOPDLG_H
#define STOPDLG_H

#include "svnfrontend/fronthelpers/cursorstack.h"

#include <QDialog>
#include <QElapsedTimer>

class QDialogButtonBox;
class QTimer;
class QLabel;
class QProgressBar;
class QTextBrowser;
class QHideEvent;
class QShowEvent;
class QVBoxLayout;
class CContextListener;

/**
@author Rajko Albrecht
*/
class StopDlg : public QDialog
{
    Q_OBJECT
public:
    StopDlg(CContextListener *listener, QWidget *parent, const QString &caption, const QString &text);
    ~StopDlg();

protected:
    int m_MinDuration;
    bool mCancelled;
    QTimer *mShowTimer;
    bool mShown, mWait;
    QLabel *mLabel;
    QProgressBar *m_ProgressBar;
    QProgressBar *m_NetBar;
    bool m_BarShown;
    bool m_netBarShown;
    QElapsedTimer m_StopTick;
    QTextBrowser *m_LogWindow;
    QVBoxLayout *layout, *mainLayout;

    QString m_lastLog;
    unsigned int m_lastLogLines;
    CursorStack *cstack;
    QDialogButtonBox *m_bBox;

    void showEvent(QShowEvent *e) override;
    void hideEvent(QHideEvent *e) override;

public slots:
    virtual void slotTick();
    virtual void slotWait(bool);
    virtual void slotExtraMessage(const QString &);

protected slots:
    virtual void slotAutoShow();
    virtual void slotCancel();
    virtual void slotNetProgres(long long int, long long int);
signals:
    void sigCancel(bool how);
};

#endif
