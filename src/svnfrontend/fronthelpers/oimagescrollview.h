#ifndef _IMAGE_SCROLL_VIEW_H
#define _IMAGE_SCROLL_VIEW_H

#include <qscrollview.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qdialog.h>
#include <qbitarray.h>


class QPainter;

namespace Opie { namespace MM {

    class OImageZoomer;
/**
 * \brief Class displaying an image with scrollbars
 *
 * This class displays various image formats supported by QT an
 * gives a small interface for basics display modifications.
 *
 * @see QScrollView
 *
 * @since 1.2
 */
class OImageScrollView:public QScrollView
{
    Q_OBJECT
public:
    enum  Rotation {
        Rotate0,
        Rotate90,
        Rotate180,
        Rotate270
    };

    /**
     * Standard constructor
     * @param parent the parent widget
     * @param name the name of the widget
     * @param fl widget flags. The flag Qt::WRepaintNoErase will be always set.
     */
    OImageScrollView( QWidget* parent, const char* name = 0, WFlags fl = 0 );
    /**
     * constructor
     * @param aImage QImage object to display
     * @param parent the parent widget
     * @param name the name of the widget
     * @param fl widget flags. The flag Qt::WRepaintNoErase will be always set.
     * @param always_scale if the image should be scaled into the display
     * @param rfit the image will be rotated to fit
     */
    OImageScrollView (const QImage&aImage, QWidget * parent=0, const char * name=0, WFlags f=0,bool always_scale=false,bool rfit=false );
    /**
     * constructor
     * @param aFile image file to display
     * @param parent the parent widget
     * @param name the name of the widget
     * @param fl widget flags. The flag Qt::WRepaintNoErase will be always set.
     * @param always_scale if the image should be scaled into the display
     * @param rfit the image will be rotated to fit
     */
    OImageScrollView (const QString&aFile, QWidget * parent=0, const char * name=0, WFlags f=0,bool always_scale=false,bool rfit=false );
    virtual ~OImageScrollView();

    /**
     * sets the WDestructiveClose flag to the view
     */
    virtual void setDestructiveClose();

    /**
     * set if the image should be rotate to best fit
     * and repaint it if set to a new value.
     *
     * Be carefull - autorating real large images cost time!
     * @param how if true then autorotate otherwise not
     */
    virtual void setAutoRotate(bool how);
    /**
     * set if the image should be scaled to the size of the viewport if larger(!)
     *
     * if autoscaling is set when loading a jpeg image, it will use a feature of
     * jpeg lib to load the image scaled to display size. If switch of later the
     * image will reloaded.
     *
     * @param how true - display image scaled down otherwise not
     */
    virtual void setAutoScale(bool how);
    /**
     * set if the image should be scaled to the size of the viewport if larger(!)
     * and/or rotate to best fit. You avoid double repainting when you want to switch
     * booth values.
     *
     * if autoscaling is set when loading a jpeg image, it will use a feature of
     * jpeg lib to load the image scaled to display size. If switch of later the
     * image will reloaded.
     *
     * @param scale true - display image scaled down otherwise not
     * @param rotate true - the image will rotate for best fit
     */
    virtual void setAutoScaleRotate(bool scale, bool rotate);
    /**
     * set if there should be displayed a small zoomer widget at the right bottom of
     * the view when the image is larger than the viewport.
     *
     * @param how true - display zoomer
     */
    virtual void setShowZoomer(bool how);

    /**
     * return the current value of the autorotate flag.
     */
    virtual bool AutoRotate()const;
    /**
     * return the current value of the autoscale flag.
     */
    virtual bool AutoScale()const;
    /**
     * return the current value of the show zoomer flag.
     */
    virtual bool ShowZoomer()const;

    /**
     * set a display intensity
     * @param value the intensity value, will calcuated to a percent value (value/100)
     * @param reload should the real image recalculated complete or just work on current display.
     * @return the new intensity
     */
    virtual int setIntensity(int value,bool reload=false);
    /**
     * return the current display intensity
     */
    virtual const int Intensity()const;


public slots:
    /**
     * Displays a new image, calculations will made immediately.
     *
     * @param aImage the image to display
     */
    virtual void setImage(const QImage&aImage);

signals:
    /**
     * emitted when the display image size has changed.
     */
    void imageSizeChanged( const QSize& );
    /**
     * emitted when the size of the viewport has changed, eg. in resizeEvent of
     * the view.
     *
     * @see QWidget::resizeEvent
     */
    void viewportSizeChanged( const QSize& );

protected:
    virtual void drawContents ( QPainter * p, int clipx, int clipy, int clipw, int cliph );
    void init();

    Opie::MM::OImageZoomer *_zoomer;
    QImage _image_data;
    QImage _original_data;
    QPixmap _pdata;
    int _intensity;
    bool _newImage;

    int _mouseStartPosX,_mouseStartPosY;

    QBitArray m_states;

    Rotation m_last_rot;
    QString m_lastName;
    virtual void rescaleImage(int w, int h);

    virtual void rotate_into_data(Rotation r);
    virtual void generateImage();
    bool image_fit_into(const QSize&s);
    void check_zoomer();

    /* internal bitset manipulation */
    virtual bool ImageIsJpeg()const;
    virtual void setImageIsJpeg(bool how);
    virtual bool ImageScaledLoaded()const;
    virtual void setImageScaledLoaded(bool how);
    virtual bool FirstResizeDone()const;
    virtual void setFirstResizeDone(bool how);
    virtual void apply_gamma(int aValue);

protected slots:
    virtual void viewportMouseMoveEvent(QMouseEvent* e);
    virtual void contentsMousePressEvent ( QMouseEvent * e);
    virtual void resizeEvent(QResizeEvent * e);
    virtual void keyPressEvent(QKeyEvent * e);
};

}
}

#endif
