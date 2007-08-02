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
#include "svnfiletip.h"
#include "svnitem.h"

#include <kfileitem.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kdebug.h>

#include <qlabel.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <qpainter.h>
#include <q3scrollview.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <Q3GridLayout>
#include <Q3Frame>
#include <QPixmap>
#include <QEvent>

SvnFileTip::SvnFileTip(Q3ScrollView*parent)
    :  Q3Frame( 0, 0, Qt::WStyle_Customize | Qt::WStyle_NoBorder | Qt::WStyle_Tool | Qt::WStyle_StaysOnTop | WX11BypassWM ),
    m_on( false ),
    m_preview( false ),
    m_filter( false ),
    m_corner( 0 ),
    m_num( 0 ),
    m_view( parent ),
    m_svnitem( 0 ),
    m_previewJob( 0 )
{
    m_iconLabel = new QLabel(this);
    m_textLabel = new QLabel(this);
    m_textLabel->setAlignment(Qt::AlignAuto | Qt::AlignTop);

    Q3GridLayout* layout = new Q3GridLayout(this, 1, 2, 8, 0);
    layout->addWidget(m_iconLabel, 0, 0);
    layout->addWidget(m_textLabel, 0, 1);
    layout->setResizeMode(QLayout::Fixed);

    setPalette( QToolTip::palette() );
    setMargin( 1 );
    setFrameStyle( Q3Frame::Plain | Q3Frame::Box );

    m_timer = new QTimer(this);

    hide();
}


SvnFileTip::~SvnFileTip()
{
   if ( m_previewJob ) {
        m_previewJob->kill();
        m_previewJob = 0;
    }
}

void SvnFileTip::setPreview(bool on)
{
    m_preview = on;
    if(on)
        m_iconLabel->show();
    else
        m_iconLabel->hide();
}

void SvnFileTip::setOptions( bool on, bool preview, int num )
{
    setPreview(preview);
    m_on = on;
    m_num = num;
}

void SvnFileTip::setItem(SvnItem*item, const QRect &rect, const QPixmap *pixmap )
{
    hideTip();

    if (!m_on) return;

    if ( m_previewJob ) {
        m_previewJob->kill();
        m_previewJob = 0;
    }

    m_rect = rect;
    m_svnitem=item;

    if ( m_svnitem ) {

        if (m_preview) {
            if ( pixmap )
              m_iconLabel->setPixmap( *pixmap );
            else
              m_iconLabel->setPixmap( QPixmap() );
        }

        // Don't start immediately, because the user could move the mouse over another item
        // This avoids a quick sequence of started preview-jobs
        m_timer->disconnect( this );
        connect(m_timer, SIGNAL(timeout()), this, SLOT(startDelayed()));
        m_timer->start( 300, true );
    } else {
        m_timer->stop();
    }
}

void SvnFileTip::reposition()
{
    if ( m_rect.isEmpty() || !m_view || !m_view->viewport() ) return;

    QRect rect = m_rect;
    QPoint off = m_view->viewport()->mapToGlobal( m_view->contentsToViewport( rect.topRight() ) );
    rect.moveTopRight( off );

    QPoint pos = rect.center();
    // m_corner:
    // 0: upperleft
    // 1: upperright
    // 2: lowerleft
    // 3: lowerright
    // 4+: none
    m_corner = 0;
    // should the tooltip be shown to the left or to the right of the ivi ?
    QRect desk = KGlobalSettings::desktopGeometry(rect.center());
    if (rect.center().x() + width() > desk.right())
    {
        // to the left
        if (pos.x() - width() < 0) {
            pos.setX(0);
            m_corner = 4;
        } else {
            pos.setX( pos.x() - width() );
            m_corner = 1;
        }
    }
    // should the tooltip be shown above or below the ivi ?
    if (rect.bottom() + height() > desk.bottom())
    {
        // above
        pos.setY( rect.top() - height() );
        m_corner += 2;
    }
    else pos.setY( rect.bottom() + 1 );

    move( pos );
    update();
}

void SvnFileTip::gotPreview( const KFileItem* item, const QPixmap& pixmap )
{
    m_previewJob = 0;
    if (!m_svnitem || item != m_svnitem->fileItem()) return;

    m_iconLabel -> setPixmap(pixmap);
}

void SvnFileTip::gotPreviewResult()
{
    m_previewJob = 0;
}

void SvnFileTip::drawContents( QPainter *p )
{
    static const char * const names[] = {
        "arrow_topleft",
        "arrow_topright",
        "arrow_bottomleft",
        "arrow_bottomright"
    };

    if (m_corner >= 4) {  // 4 is empty, so don't draw anything
        Q3Frame::drawContents( p );
        return;
    }

    if ( m_corners[m_corner].isNull())
        m_corners[m_corner].load( locate( "data", QString::fromLatin1( "konqueror/pics/%1.png" ).arg( names[m_corner] ) ) );

    QPixmap &pix = m_corners[m_corner];

    switch ( m_corner )
    {
        case 0:
            p->drawPixmap( 3, 3, pix );
            break;
        case 1:
            p->drawPixmap( width() - pix.width() - 3, 3, pix );
            break;
        case 2:
            p->drawPixmap( 3, height() - pix.height() - 3, pix );
            break;
        case 3:
            p->drawPixmap( width() - pix.width() - 3, height() - pix.height() - 3, pix );
            break;
    }

    Q3Frame::drawContents( p );
}

void SvnFileTip::setFilter( bool enable )
{
    if ( enable == m_filter ) return;

    if ( enable ) {
        kapp->installEventFilter( this );
        QApplication::setGlobalMouseTracking( true );
    }
    else {
        QApplication::setGlobalMouseTracking( false );
        kapp->removeEventFilter( this );
    }
    m_filter = enable;
}

void SvnFileTip::showTip()
{
    if (!m_svnitem) {
        hide();
        return;
    }
    QString text = m_svnitem->getToolTipText();

    if ( text.isEmpty() ) return;

    m_timer->disconnect( this );
    connect(m_timer, SIGNAL(timeout()), this, SLOT(hideTip()));
    m_timer->start( 15000, true );

    m_textLabel->setText( text );

    setFilter( true );

    reposition();
    show();
}

void SvnFileTip::hideTip()
{
    m_timer->stop();
    setFilter( false );
    if ( isShown() && m_view && m_view->viewport() &&
         (m_view->horizontalScrollBar()->isShown() || m_view->verticalScrollBar()->isShown()) )
      m_view->viewport()->update();
    hide();
}
void SvnFileTip::startDelayed()
{
    if (!m_svnitem) {
        return;
    }
    if ( m_preview && m_svnitem->fileItem() ) {
        KFileItemList oneItem;
        oneItem.append( m_svnitem->fileItem() );

        m_previewJob = KIO::filePreview( oneItem, 256, 256, 64, 70, true, true, 0);
        connect( m_previewJob, SIGNAL( gotPreview( const KFileItem& , const QPixmap & ) ),
                 this, SLOT( gotPreview( const KFileItem& , const QPixmap & ) ) );
        connect( m_previewJob, SIGNAL( result( KIO::Job * ) ),
                 this, SLOT( gotPreviewResult() ) );
    }

    m_timer->disconnect( this );
    connect(m_timer, SIGNAL(timeout()), this, SLOT(showTip()));
    m_timer->start( 400, true );
}

void SvnFileTip::resizeEvent( QResizeEvent* event )
{
    Q3Frame::resizeEvent(event);
    reposition();
}

bool SvnFileTip::eventFilter( QObject *, QEvent *e )
{
    switch ( e->type() )
    {
        case QEvent::Leave:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::FocusIn:
        case QEvent::FocusOut:
        case QEvent::Wheel:
            hideTip();
        default: break;
    }

    return false;
}

#include "svnfiletip.moc"
