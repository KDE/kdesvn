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
#include "stopdlg.h"
#include "ccontextlistener.h"
#include "settings/kdesvnsettings.h"
#include "helpers/stringhelper.h"

#include <kapplication.h>
#include <klocale.h>
#include <kvbox.h>
#include <kwindowsystem.h>
#include <kdebug.h>
#include <ktextbrowser.h>
#include <kiconloader.h>

#include <QTimer>
#include <QPushButton>
#include <QLayout>
#include <QLabel>
#include <QWidget>
#include <QShowEvent>
#include <QHideEvent>
#include <QVBoxLayout>
#include <QProgressBar>

StopDlg::StopDlg(QObject*listener,QWidget *parent, const char *name,const QString&caption,const QString&text)
 : KDialog(parent)
    ,m_Context(listener),m_MinDuration(1000),mCancelled(false),mShown(false),m_BarShown(false),
    cstack(0)
{
    setObjectName(name);
    setCaption(caption);

    setButtons(KDialog::Cancel);
    m_mainWidget = new QFrame( this );
    setMainWidget( m_mainWidget );

    m_lastLogLines = 0;
    m_lastLog = "";

    mShowTimer = new QTimer(this);
    m_StopTick.start();
    showButton(KDialog::Close, false);
    mCancelText = buttonText(KDialog::Cancel);

    layout = new QVBoxLayout(m_mainWidget);
    mLabel = new QLabel(text, m_mainWidget);
    layout->addWidget(mLabel);
    m_ProgressBar=new QProgressBar(m_mainWidget);
    m_ProgressBar->setRange(0,15);
    m_ProgressBar->setTextVisible(false);
    layout->addWidget(m_ProgressBar);
    m_NetBar = new QProgressBar(m_mainWidget);
    m_NetBar->setRange(0,15);
    layout->addWidget(m_NetBar);

    mWait = false;
    m_LogWindow = 0;

    connect(mShowTimer, SIGNAL(timeout()), this, SLOT(slotAutoShow()));
    connect(this,SIGNAL(cancelClicked()),this,SLOT(slotCancel()));
    if (m_Context) {
        connect(m_Context,SIGNAL(tickProgress()),this,SLOT(slotTick()));
        connect(m_Context,SIGNAL(waitShow(bool)),this,SLOT(slotWait(bool)));
        connect(m_Context,SIGNAL(netProgress(long long int, long long int)),
                this,SLOT(slotNetProgres(long long int, long long int)));
        connect(this,SIGNAL(sigCancel(bool)),m_Context,SLOT(setCanceled(bool)));
    }
    mShowTimer->setSingleShot(true);
    mShowTimer->start(m_MinDuration);
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
        hide();
    }
    if (mShown||mWait||hasDialogs) {
        mShowTimer->setSingleShot(true);
        if (mWait) {
            //kDebug(9510) << "Waiting for show"<<endl;
            mShowTimer->start(m_MinDuration);
        }
        mShowTimer->start(m_MinDuration);
        return;
    }
    m_ProgressBar->hide();
    m_NetBar->hide();
    m_BarShown=false;
    m_netBarShown=false;
    show();
    kapp->processEvents();
    mShown = true;
    mShowTimer->setSingleShot(true);
    mShowTimer->start(m_MinDuration);
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
        if (m_ProgressBar->value()==15) {
            m_ProgressBar->reset();
        } else {
            m_ProgressBar->setValue(m_ProgressBar->value()+1);
        }
        m_StopTick.restart();
        kapp->processEvents();
    }
}

void StopDlg::slotExtraMessage(const QString&msg)
{
    ++m_lastLogLines;
    if (!m_LogWindow) {
        m_LogWindow = new KTextBrowser(m_mainWidget);
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
        QString s1 = helpers::ByteToString(current);
        if (max > -1 && max != m_NetBar->maximum()) {
            QString s2 = helpers::ByteToString(max);
            m_NetBar->setFormat(i18n("%p% of %1",s2));
            m_NetBar->setRange(0,max);
        }
        if (max == -1) {
            if (m_NetBar->maximum()==-1 || m_NetBar->maximum()<current) {
                m_NetBar->setFormat(i18n("%1 transferred.",s1));
                m_NetBar->setRange(0,current+1);
            } else {
                m_NetBar->setFormat(i18n("%1 of %2",s1,helpers::ByteToString(m_NetBar->maximum())));
            }
        }
        m_NetBar->setValue(current);
        m_StopTick.restart();
        kapp->processEvents();
    }
}

StopSimpleDlg::StopSimpleDlg(QWidget *parent, const char *name,const QString&caption,const QString&text)
    : StopDlg(0,parent,name,caption,text),cancelld(false)
{
    connect(this,SIGNAL(sigCancel(bool)),this,SLOT(slotSimpleCancel(bool)));
}

void StopSimpleDlg::slotSimpleCancel(bool how)
{
    cancelld = how;
}

void StopSimpleDlg::makeCancel()
{
    slotSimpleCancel(true);
}

#include "stopdlg.moc"
