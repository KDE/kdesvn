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
#include "listview/kdesvndirlist.h"

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
      DCOPObject("kdesvnIface")
{
    // setup our layout manager to automatically add our widgets
    QHBoxLayout *top_layout = new QHBoxLayout(this);
    m_Splitter = new QSplitter(this,"m_Splitter");
    m_Splitter->setOrientation( QSplitter::Horizontal );
    //top_layout->setAutoAdd(true);
    m_LeftList=new KdeSvnDirList(m_Splitter);
    m_flist=new kdesvnfilelist(m_Splitter);
    m_flist->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 3, 0, m_flist->sizePolicy().hasHeightForWidth() ) );
    top_layout->addWidget(m_Splitter);

#if 0
    // we want to look for all components that satisfy our needs.  the
    // trader will actually search through *all* registered KDE
    // applications and components -- not just KParts.  So we have to
    // specify two things: a service type and a constraint
    //
    // the service type is like a mime type.  we say that we want all
    // applications and components that can handle HTML -- 'text/html'
    //
    // however, by itself, this will return such things as Netscape..
    // not what we wanted.  so we constrain it by saying that the
    // string 'KParts/ReadOnlyPart' must be found in the ServiceTypes
    // field.  with this, only components of the type we want will be
    // returned.
    KTrader::OfferList offers = KTrader::self()->query("text/html", "'KParts/ReadOnlyPart' in ServiceTypes");

    KLibFactory *factory = 0;
    // in theory, we only care about the first one.. but let's try all
    // offers just in case the first can't be loaded for some reason
    KTrader::OfferList::Iterator it(offers.begin());
    for( ; it != offers.end(); ++it)
    {
        KService::Ptr ptr = (*it);

        // we now know that our offer can handle HTML and is a part.
        // since it is a part, it must also have a library... let's try to
        // load that now
        factory = KLibLoader::self()->factory( ptr->library() );
        if (factory)
        {
            m_html = static_cast<KParts::ReadOnlyPart *>(factory->create(this, ptr->name(), "KParts::ReadOnlyPart"));
            break;
        }
    }

    // if our factory is invalid, then we never found our component
    // and we might as well just exit now
    if (!factory)
    {
        KMessageBox::error(this, i18n("Could not find a suitable HTML component"));
        return;
    }

    connect(m_html, SIGNAL(setWindowCaption(const QString&)),
            this,   SLOT(slotSetTitle(const QString&)));
    connect(m_html, SIGNAL(setStatusBarText(const QString&)),
            this,   SLOT(slotOnURL(const QString&)));
#endif
}

kdesvnView::~kdesvnView()
{
}

void kdesvnView::print(QPainter *p, int height, int width)
{
    // do the actual printing, here
    // p->drawText(etc..)
}

QString kdesvnView::currentURL()
{
#if 0
    return m_html->url().url();
#endif
}

void kdesvnView::openURL(QString url)
{
    openURL(KURL(url));
}

void kdesvnView::openURL(const KURL& url)
{
    /* check later against http/https protocol - then it should be a webdav-svn url */
    if (!url.isLocalFile()) {
        return;
    }
    QFileInfo f(url.path());
    if (!f.isDir()) {
        return;
    }
    m_flist->openURL(url);
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
