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
#include "stopdlg.h"
#include "ccontextlistener.h"
#include "settings/kdesvnsettings.h"
#include "helpers/stringhelper.h"

#include <kapplication.h>
#include <klocale.h>
#include <kwin.h>
#include <qtimer.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qwidget.h>
//Added by qt3to4:
#include <QShowEvent>
#include <QHideEvent>
#include <Q3VBoxLayout>
#include <Q3Frame>
#include <kprogress.h>
#include <kdebug.h>
#include <ktextbrowser.h>

StopDlg::StopDlg(QObject*listener,QWidget *parent, const char *name,const QString&caption,const QString&text)
 : KDialogBase(KDialogBase::Plain,caption,KDialogBase::Cancel, KDialogBase::Cancel,parent, name,true)
    ,m_Context(listener),m_MinDuration(1000),mCancelled(false),mShown(false),m_BarShown(false),
    cstack(0)
{
    KWin::setIcons(winId(), kapp->icon(), kapp->miniIcon());
    m_lastLogLines = 0;
    m_lastLog = "";

    mShowTimer = new QTimer(this);
    m_StopTick.start();
    showButton(KDialogBase::Close, false);
    mCancelText = actionButton(KDialogBase::Cancel)->text();

    Q3Frame* mainWidget = plainPage();
    layout = new Q3VBoxLayout(mainWidget, 10);
    mLabel = new QLabel(text, mainWidget);
    layout->addWidget(mLabel);
    m_ProgressBar=new KProgress(15,mainWidget);
    m_ProgressBar->setCenterIndicator (false);
    m_ProgressBar->setTextEnabled(false);
    layout->addWidget(m_ProgressBar);
    m_NetBar = new KProgress(15,mainWidget);
    layout->addWidget(m_NetBar);

    mWait = false;
    m_LogWindow = 0;

    connect(mShowTimer, SIGNAL(timeout()), this, SLOT(slotAutoShow()));
    connect(m_Context,SIGNAL(tickProgress()),this,SLOT(slotTick()));
    connect(m_Context,SIGNAL(waitShow(bool)),this,SLOT(slotWait(bool)));
    connect(m_Context,SIGNAL(netProgress(long long int, long long int)),
        this,SLOT(slotNetProgres(long long int, long long int)));
    connect(this,SIGNAL(sigCancel(bool)),m_Context,SLOT(setCanceled(bool)));
    mShowTimer->start(m_MinDuration, true);
    setMinimumSize(280,160);
    adjustSize();
}

void StopDlg::showEvent( QShowEvent*)
{
    cstack = new CursorStack(Qt::BusyCursor);
}

void StopDlg::hideEvent(QHideEvent*)
{
    delete cstack; cstack = 0;
}

void StopDlg::slotWait(bool how)
{
    mWait = how;
    if (mShown && mWait) {
        hide();
        mShown = false;
    }
}

StopDlg::~StopDlg()
{
    delete cstack;
}

void StopDlg::slotAutoShow()
{
    bool hasDialogs = false;
    QWidget * w = kapp->activeModalWidget();
    if (w && w!=this && w!=parentWidget() ) {
        hasDialogs = true;
    }
    if (hasDialogs) {
        kdDebug()<<"Hide me! (" << caption() << ")" << endl;
        hide();
    }
    if (mShown||mWait||hasDialogs) {
        if (mWait) {
            //kdDebug() << "Waiting for show"<<endl;
            mShowTimer->start(m_MinDuration, true);
        }
        mShowTimer->start(m_MinDuration, true);
        return;
    }
    m_ProgressBar->hide();
    m_NetBar->hide();
    m_BarShown=false;
    m_netBarShown=false;
    show();
    kapp->processEvents();
    mShown = true;
    mShowTimer->start(m_MinDuration, true);
}

void StopDlg::slotCancel()
{
    mCancelled = true;
    emit sigCancel(true);
}

bool StopDlg::cancelld()
{
    return mCancelled;
}

void StopDlg::slotTick()
{
    if (m_StopTick.elapsed()>500) {
        if (!m_BarShown) {
            m_ProgressBar->show();
            m_BarShown=true;
        }
        if (m_ProgressBar->progress()==15) {
            m_ProgressBar->reset();
        } else {
            m_ProgressBar->setProgress(m_ProgressBar->progress()+1);
        }
        m_StopTick.restart();
        kapp->processEvents();
    }
}

void StopDlg::slotExtraMessage(const QString&msg)
{
    ++m_lastLogLines;
    if (!m_LogWindow) {
        Q3Frame* mainWidget = plainPage();
        m_LogWindow = new KTextBrowser(mainWidget);
        layout->addWidget(m_LogWindow);
        m_LogWindow->show();
        resize( QSize(500, 400).expandedTo(minimumSizeHint()) );
    }
    if (m_lastLogLines >= Kdesvnsettings::self()->cmdline_log_minline() &&
        isHidden() ) {
        slotAutoShow();
    }
    m_LogWindow->append(msg);
    kapp->processEvents();
}

void StopDlg::slotNetProgres(long long int current, long long int max)
{
    if (m_StopTick.elapsed()>300||(m_BarShown&&!m_netBarShown)) {
        if (!m_netBarShown) {
            m_NetBar->show();
            m_netBarShown=true;
        }
        QString s1 = helpers::ByteToString()(current);
        if (max > -1 && max != m_NetBar->totalSteps()) {
            QString s2 = helpers::ByteToString()(max);
            m_NetBar->setFormat(i18n("%1 of %2").arg(s1).arg(s2));
            m_NetBar->setTotalSteps(max);
        }
        if (max == -1) {
            m_NetBar->setFormat(i18n("%1 transferred.").arg(s1));
            m_NetBar->setTotalSteps(current+1);
        }
        m_NetBar->setValue(current);
        m_StopTick.restart();
        kapp->processEvents();
    }
}

#include "stopdlg.moc"
