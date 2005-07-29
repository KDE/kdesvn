/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht   *
 *   ral@alwins-world.de   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "stopdlg.h"
#include "ccontextlistener.h"
#include <kapplication.h>
#include <klocale.h>
#include <kwin.h>
#include <qtimer.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <kprogress.h>
#include <kdebug.h>

StopDlg::StopDlg(CContextListener*listener,QWidget *parent, const char *name,const QString&caption,const QString&text)
 : KDialogBase(KDialogBase::Plain,caption,KDialogBase::Cancel, KDialogBase::Cancel,parent, name,true)
    ,m_Context(listener),m_MinDuration(2000),mCancelled(false),mShown(false),m_BarShown(false)
{
    KWin::setIcons(winId(), kapp->icon(), kapp->miniIcon());
    mShowTimer = new QTimer(this);
    showButton(KDialogBase::Close, false);
    mCancelText = actionButton(KDialogBase::Cancel)->text();

    QFrame* mainWidget = plainPage();
    QVBoxLayout* layout = new QVBoxLayout(mainWidget, 10);
    mLabel = new QLabel(text, mainWidget);
    layout->addWidget(mLabel);
    m_ProgressBar=new KProgress(15,mainWidget);
    m_ProgressBar->setCenterIndicator (false);
    m_ProgressBar->setTextEnabled(false);

    layout->addWidget(m_ProgressBar);
    mWait = false;

    connect(mShowTimer, SIGNAL(timeout()), this, SLOT(slotAutoShow()));
    connect(m_Context,SIGNAL(tickProgress()),this,SLOT(slotTick()));
    connect(m_Context,SIGNAL(waitShow(bool)),this,SLOT(slotWait(bool)));
    mShowTimer->start(m_MinDuration, true);
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
}

void StopDlg::slotAutoShow()
{
    if (mShown||mWait) {
        if (mWait) {
            kdDebug() << "Waiting for show"<<endl;
            mShowTimer->start(m_MinDuration, true);
        }
        return;
    }
    m_ProgressBar->hide();
    m_BarShown=false;
    show();
    kapp->processEvents();
    mShown = true;
}

void StopDlg::slotCancel()
{
    mCancelled = true;
    m_Context->setCancelled(true);
}

bool StopDlg::cancelld()
{
    return mCancelled;
}

void StopDlg::slotTick()
{
    if (!m_BarShown) {
        m_ProgressBar->show();
        m_BarShown=true;
    }
    if (m_ProgressBar->progress()==15) {
        m_ProgressBar->reset();
    } else {
        m_ProgressBar->setProgress(m_ProgressBar->progress()+1);
    }
    kapp->processEvents();
}

#include "stopdlg.moc"
