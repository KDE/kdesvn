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


#include "kdesvnview.h"
#include "listview/kdesvnfilelist.h"

#include <qpainter.h>
#include <qlayout.h>
#include <qfileinfo.h>
#include <qheader.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qsplitter.h>

#include <kurl.h>

#include <ktrader.h>
#include <klibloader.h>
#include <kmessagebox.h>
#include <krun.h>
#include <klocale.h>

kdesvnView::kdesvnView(QWidget *parent)
    : QWidget(parent),
      DCOPObject("kdesvnIface"),
      m_currentURL("")
{
    QHBoxLayout *top_layout = new QHBoxLayout(this);
    m_flist=new kdesvnfilelist(this);
    m_flist->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 1, 0, m_flist->sizePolicy().hasHeightForWidth() ) );
    top_layout->addWidget(m_flist);
}

kdesvnView::~kdesvnView()
{
}

KActionCollection*kdesvnView::filesActions()
{
    return m_flist->filesActions();
}

//void kdesvnView::print(QPainter *p, int height, int width)
void kdesvnView::print(QPainter *, int , int)
{
    // do the actual printing, here
    // p->drawText(etc..)
}

QString kdesvnView::currentURL()
{
    return m_currentURL;
}

void kdesvnView::openURL(QString url)
{
    openURL(KURL(url));
}

void kdesvnView::openURL(const KURL& url)
{
    m_currentURL = "";
    if (url.isLocalFile()) {
        QFileInfo f(url.path());
        if (!f.isDir()) {
            return;
        }
    } else if (url.protocol()!="http"&&url.protocol()!="https") {
        return;
    }
    slotSetTitle(url.prettyURL());
    if (m_flist->openURL(url)) {
        slotOnURL(i18n("Repository opened"));
        m_currentURL=url.url();
        //m_LeftList->setCurrentUrl(url.url());
        //svn::StatusEntries temp; temp.push_back(m_flist->maindir());
        //m_LeftList->appendEntries(temp);
        //m_LeftList->appendEntries(m_flist->directories());
    } else {
        QString t = m_flist->lastError();
        if (t.isEmpty()) {
            t = i18n("Could not open repository");
        }
        slotOnURL(t);
    }
}

void kdesvnView::slotOnURL(const QString& url)
{
    emit signalChangeStatusbar(url);
}

void kdesvnView::slotSetTitle(const QString& title)
{
    emit signalChangeCaption(title);
}

#include "kdesvnview.moc"
