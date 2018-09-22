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

#include <KLocalizedString>

#include <QApplication>
#include <QDialogButtonBox>
#include <QTimer>
#include <QLabel>
#include <QTextBrowser>
#include <QWidget>
#include <QVBoxLayout>
#include <QProgressBar>

StopDlg::StopDlg(CContextListener *listener, QWidget *parent, const QString &caption, const QString &text)
    : QDialog(parent)
    , m_MinDuration(1000)
    , mCancelled(false)
    , mShown(false)
    , m_BarShown(false)
    , m_netBarShown(false)
    , cstack(nullptr)
    , m_bBox(new QDialogButtonBox(QDialogButtonBox::Cancel, this))
{
    setWindowTitle(caption);

    m_lastLogLines = 0;
    m_lastLog.clear();

    mShowTimer = new QTimer(this);
    m_StopTick.start();

    mainLayout = new QVBoxLayout(this);
    layout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    mainLayout->addWidget(m_bBox);
    mLabel = new QLabel(text, this);
    layout->addWidget(mLabel);
    m_ProgressBar = new QProgressBar(this);
    m_ProgressBar->setRange(0, 15);
    m_ProgressBar->setTextVisible(false);
    layout->addWidget(m_ProgressBar);
    m_NetBar = new QProgressBar(this);
    m_NetBar->setRange(0, 15);
    layout->addWidget(m_NetBar);

    mWait = false;
    m_LogWindow = nullptr;

    connect(mShowTimer, &QTimer::timeout, this, &StopDlg::slotAutoShow);
    connect(m_bBox, &QDialogButtonBox::rejected, this, &StopDlg::slotCancel);
    if (listener) {
        connect(listener, &CContextListener::tickProgress, this, &StopDlg::slotTick);
        connect(listener, &CContextListener::waitShow, this, &StopDlg::slotWait);
        connect(listener, &CContextListener::netProgress,
                this, &StopDlg::slotNetProgres);
        connect(this, &StopDlg::sigCancel, listener, &CContextListener::setCanceled);
    }
    mShowTimer->setSingleShot(true);
    mShowTimer->start(m_MinDuration);
    setMinimumSize(280, 160);
    adjustSize();
}

void StopDlg::showEvent(QShowEvent *e)
{
    if (!cstack) {
        cstack = new CursorStack(Qt::BusyCursor);
    }
    QDialog::showEvent(e);
}

void StopDlg::hideEvent(QHideEvent *e)
{
    delete cstack;
    cstack = nullptr;
    QDialog::hideEvent(e);
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
    QWidget *w = QApplication::activeModalWidget();
    if (w && w != this && w != parentWidget()) {
        hasDialogs = true;
    }
    if (hasDialogs) {
        hide();
    }
    if (mShown || mWait || hasDialogs) {
        mShowTimer->setSingleShot(true);
        if (mWait) {
            //qCDebug(KDESVN_LOG) << "Waiting for show"<<endl;
            mShowTimer->start(m_MinDuration);
        }
        mShowTimer->start(m_MinDuration);
        return;
    }
    m_ProgressBar->hide();
    m_NetBar->hide();
    m_BarShown = false;
    m_netBarShown = false;
    show();
    QCoreApplication::processEvents();
    mShown = true;
    mShowTimer->setSingleShot(true);
    mShowTimer->start(m_MinDuration);
}

void StopDlg::slotCancel()
{
    mCancelled = true;
    emit sigCancel(true);
}

void StopDlg::slotTick()
{
    if (m_StopTick.elapsed() > 500) {
        if (!m_BarShown) {
            m_ProgressBar->show();
            m_BarShown = true;
        }
        if (m_ProgressBar->value() == 15) {
            m_ProgressBar->reset();
        } else {
            m_ProgressBar->setValue(m_ProgressBar->value() + 1);
        }
        m_StopTick.restart();
        QCoreApplication::processEvents();
    }
}

void StopDlg::slotExtraMessage(const QString &msg)
{
    ++m_lastLogLines;
    if (!m_LogWindow) {
        m_LogWindow = new QTextBrowser(this);
        layout->addWidget(m_LogWindow);
        m_LogWindow->show();
        resize(QSize(500, 400).expandedTo(minimumSizeHint()));
    }
    if (m_lastLogLines >= Kdesvnsettings::self()->cmdline_log_minline() &&
            isHidden()) {
        slotAutoShow();
    }
    m_LogWindow->append(msg);
    QCoreApplication::processEvents();
}

void StopDlg::slotNetProgres(long long int current, long long int max)
{
    if (m_StopTick.elapsed() > 300 || (m_BarShown && !m_netBarShown)) {
        if (!m_netBarShown) {
            m_NetBar->show();
            m_netBarShown = true;
        }
        QString s1 = helpers::ByteToString(current);
        if (max > -1 && max != m_NetBar->maximum()) {
            QString s2 = helpers::ByteToString(max);
            m_NetBar->setFormat(i18n("%p% of %1", s2));
            m_NetBar->setRange(0, max);
        }
        if (max == -1) {
            if (m_NetBar->maximum() == -1 || m_NetBar->maximum() < current) {
                m_NetBar->setFormat(i18n("%1 transferred.", s1));
                m_NetBar->setRange(0, current + 1);
            } else {
                m_NetBar->setFormat(i18n("%1 of %2", s1, helpers::ByteToString(m_NetBar->maximum())));
            }
        }
        m_NetBar->setValue(current);
        m_StopTick.restart();
        QCoreApplication::processEvents();
    }
}
