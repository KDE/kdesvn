#include "oimagezoomer.h"

#include <qimage.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qrect.h>
#include <qpoint.h>
#include <qsize.h>

namespace Opie {
namespace MM {

/**
 * \brief The most simple c'tor
 * The main c'tor. You still need to set a QPixmap/QIMage,
 *  setImageSize,setViewPortSize,setVisiblePoint
 *
 * @param parent The parent widget
 * @param name A name for this widget
 * @param fl The widget flags
 *
 */
OImageZoomer::OImageZoomer( QWidget* parent,  const char* name,  WFlags fl )
    :  QFrame( parent, name, fl ) {
    init();
}


/**
 * \brief This c'tor takes a QPixmap additional
 *
 * You initially set the QPixmap but you still need to provide
 * the additional data to make this widget useful
 *
 * @param pix A Pixmap it'll be converted to a QImage later!
 * @param par The parent widget
 * @param name The name of this widget
 * @param fl The widget flags
 */
OImageZoomer::OImageZoomer( const QPixmap& pix, QWidget* par, const char* name, WFlags fl )
    : QFrame( par, name, fl ) {
    init();
    setImage( pix );
}


/**
 * \brief This c'tor takes a QImage instead
 * You just provide a QImage which is saved. It behaves the same as the others.
 *
 * @param img A Image which will be used for the zoomer content
 * @param par The  parent of the widget
 * @param name The name of the widget
 * @param fl The widgets flags
 */
OImageZoomer::OImageZoomer( const QImage& img, QWidget* par, const char* name,  WFlags fl)
    : QFrame( par, name, fl ) {
    init();
    setImage( img );
}


/**
 * \brief overloaded c'tor
 *
 * This differs only in the arguments it takes
 *
 *
 * @param pSize The size of the Page you show
 * @param vSize The size of the viewport. The size of the visible part of the widget
 * @param par The parent of the widget
 * @param name The name
 * @param fl The window flags
 */
OImageZoomer::OImageZoomer( const QSize& pSize, const QSize& vSize, QWidget* par,
                            const char* name, WFlags fl )
    : QFrame( par, name, fl ), m_imgSize( pSize ),m_visSize( vSize ) {
    init();
}

/**
 * d'tor
 */
OImageZoomer::~OImageZoomer() {

}

void OImageZoomer::init() {
    m_mevent = false;
    setFrameStyle( Panel | Sunken );
}


/**
 * \brief set the page/image size
 * Tell us the QSize of the Data you show to the user. We need this
 * to do the calculations
 *
 * @param size The size of the stuff you want to zoom on
 */
void OImageZoomer::setImageSize( const QSize& size ) {
    m_imgSize = size;
    repaint();
}

/**
 * \brief Set the size of the viewport
 * Tell us the QSize of the viewport. The viewport is the part
 * of the widget which is exposed on the screen
 *
 * @param size Te size of the viewport
 *
 * @see QScrollView::viewport()
 */
void OImageZoomer::setViewPortSize( const QSize& size ) {
    m_visSize = size;
    repaint();
}

/**
 * \brief the point in the topleft corner which is currently visible
 * Set the visible point. This most of the times relate to QScrollView::contentsX()
 * and QScrollView::contentsY()
 *
 * @see setVisiblePoint(int,int)
 */
void OImageZoomer::setVisiblePoint( const QPoint& pt ) {
    m_visPt = pt;
    repaint();
}


/**
 * Set the Image. The image will be resized on resizeEvent
 * and it'll set the QPixmap background
 *
 * @param img The image will be stored internally and used as the background
 */
void OImageZoomer::setImage( const QImage& img) {
    m_img = img;
    resizeEvent( 0 );
    repaint();
}

/**
 * overloaded function it calls the QImage version
 */
void OImageZoomer::setImage( const QPixmap& pix) {
    setImage( pix.convertToImage() );
}

void OImageZoomer::resizeEvent( QResizeEvent* ev ) {
    QFrame::resizeEvent( ev );
    setBackgroundOrigin(  QWidget::WidgetOrigin );
    // TODO Qt3 use PalettePixmap and use size
    QPixmap pix; pix.convertFromImage( m_img.smoothScale( size().width(), size().height() ) );
    setBackgroundPixmap( pix);
}

void OImageZoomer::drawContents( QPainter* p ) {
    /*
     * if the page size
     */
    if ( m_imgSize.isEmpty() )
        return;

   /*
    * paint a red rect which represents the visible size
    *
    * We need to recalculate x,y and width and height of the
    * rect. So image size relates to contentRect
    *
    */
    QRect c( contentsRect() );
    p->setPen( Qt::red );

    /*
     * the contentRect is set equal to the size of the image
     * Rect/Original = NewRectORWidth/OriginalVisibleStuff and then simply we
     * need to add the c.y/x due usage of QFrame
     * For x and y we use the visiblePoint
     * For height and width we use the size of the viewport
     * if width/height would be bigger than our widget we use this width/height
     *
     */
    int len = m_imgSize.width();
    int x = (c.width()*m_visPt.x())/len        + c.x();
    int w = (c.width()*m_visSize.width() )/len + c.x();
    if ( w > c.width() ) w = c.width();

    len = m_imgSize.height();
    int y = (c.height()*m_visPt.y() )/len          + c.y();
    int h = (c.height()*m_visSize.height() )/len + c.y();
    if ( h > c.height() ) h = c.height();

    p->drawRect( x, y, w, h );
}

void OImageZoomer::mousePressEvent( QMouseEvent*) {
    m_mouseX = m_mouseY = -1;
    m_mevent = true;
}

void OImageZoomer::mouseReleaseEvent( QMouseEvent*ev) {
    if (!m_mevent) return;
    int mx, my;
    mx = ev->x();
    my = ev->y();
    int diffx = (mx) * m_imgSize.width() / width();
    int diffy = (my) * m_imgSize.height() / height();
    emit zoomArea(diffx,diffy);
}

void OImageZoomer::mouseMoveEvent( QMouseEvent* ev ) {
    int mx, my;
    mx = ev->x();
    my = ev->y();

    if ( m_mouseX != -1 && m_mouseY != -1 ) {
        m_mevent = false;
        int diffx = ( mx - m_mouseX ) * m_imgSize.width() / width();
        int diffy = ( my - m_mouseY ) * m_imgSize.height() / height();
        emit zoomAreaRel( diffx, diffy );
    }
    m_mouseX = mx;
    m_mouseY = my;
}


}
}

#include "oimagezoomer.moc"
