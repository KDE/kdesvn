#ifndef OPIE_ODP_IMAGE_ZOOMER_H
#define OPIE_ODP_IMAGE_ZOOMER_H

#include <qframe.h>
#include <qimage.h>

class QPixmap;
class QRect;
class QPoint;


namespace Opie {
namespace MM   {

/**
 * \brief small class to zoom over a Page
 *
 *  This class represents your page but smaller.
 *  It can draw a Rect on top of an Image/Pixmap you supply
 *  and you can allow the user easily zooming/moving
 *  over your widget.
 *  All you need to do is to supply a image/pixmap, the visible size
 *  and the original image/pixmap size and the current visible top/left
 *  position.
 *
 *  This Image works perfectly with QScrollView as you can connect
 *  QScrollView::contentsMoving to setVisiblePoint slot and the zoomAreRel
 *  to the QScrollView::scrollBy slot. Now you would only need to watch
 *  the resize event anf give us the new information about QScrollView::viewport
 *
 *  You need to position and set the size of this widget! using setFixedSize() is quite
 *  a good idea for this widget
 *
 * @see QScrollView
 * @see QScrollView::viewport()
 *
 * @since 1.2
 *
 */
class OImageZoomer : public QFrame {
    Q_OBJECT
public:
    OImageZoomer( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    OImageZoomer( const QPixmap&,QWidget* parent = 0, const char* name = 0,  WFlags fl = 0 );
    OImageZoomer( const QImage&, QWidget* parent = 0, const char* name= 0, WFlags fl = 0 );
    OImageZoomer( const QSize&, const QSize&, QWidget* par, const char*, WFlags fl );
    ~OImageZoomer();

public slots:
    void setImageSize( const QSize& );
    void setViewPortSize( const QSize& );
    void setVisiblePoint( const QPoint& );
    void setVisiblePoint( int x, int y );
    void setImage( const QImage& );
    void setImage( const QPixmap& );

signals:
    /**
     * Relative movement in the coordinates of the viewport
     * This signal can easily be connected to QScrollView::scrollBy.
     * This signal is emitted from within the mouseMoveEvent of this widget
     *
     *
     * @param x The way to move relative on the X-Axis
     * @param y The way to move relative on the Y-Axis
     *
     * @see setVisiblePoint
     * @see QScrollView::scrollBy
     */
    void zoomAreaRel( int x,int y);

    /**
     * Here you get absolute coordinates.
     * This slot will be emitted from within the mouseReleaseEvent of this widget.
     * if no mouse move where done.
     * So you may not delete this widget
     *
     * @param x The absolute X Coordinate to scroll to.
     * @param y The absolute Y Coordinate to scroll to.
     *
     */
    void zoomArea( int x,int y);

public:
    /**
     * make sure to call these if you reimplement
     * @internal
     */
    void resizeEvent( QResizeEvent* );

protected:
    /**
     * make sure to call these if you reimplement
     * @internal
     */
    void drawContents( QPainter* p );

    /**
     * make sure to call these if you reimplememt
     * @internal
     */
    virtual void mousePressEvent( QMouseEvent* ev );
    /**
     * make sure to call these if you reimplement
     * @internal
     */
    virtual void mouseMoveEvent( QMouseEvent* ev );
    /**
     * make sure to call these if you reimplement
     * @internal
     */
    virtual void mouseReleaseEvent( QMouseEvent* ev );

private:
    /**
     * @internal
     */
    void init();
    QImage m_img;
    QSize m_imgSize, m_visSize;
    QPoint m_visPt;
    int m_mouseX, m_mouseY;
    bool m_mevent;
};

/**
 * This slot is present for convience. You can connect the
 * QScrollView::contentsMoved to this slot and it calls the QPoint
 * version for you
 * This realtes to QScrollView::contentsX() and QScrollView::contentsY()
 *
 * @param x The top left x coordinate
 * @param y The top left y coorisnate
 */
inline void OImageZoomer::setVisiblePoint( int x, int y ) {
    setVisiblePoint( QPoint( x, y ) );
}

}
}
#endif
