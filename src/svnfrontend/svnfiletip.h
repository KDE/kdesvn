/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht                                  *
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
 /* this is mostly a copy of KonqFileTip
  * when kdesvn get part of KDE itself it should replaced then with the original stuff
  * now we make sure we may us it
  */

#ifndef SVNFILETIP_H
#define SVNFILETIP_H
#include <qframe.h>
#include <qpixmap.h>
#include <kio/previewjob.h>

class KFileItem;
class QLabel;
class QScrollView;
class QTimer;
class SvnItem;

/**
@author Rajko Albrecht
*/
class SvnFileTip : public QFrame
{
Q_OBJECT
public:
    SvnFileTip(QScrollView*parent);
    virtual ~SvnFileTip();
    void setPreview(bool on);

    /**
      @param on show tooltip at all
      @param preview include file preview in tooltip
      @param num the number of tooltip texts to get from KFileItem
      */
    void setOptions( bool on, bool preview, int num );

    /** Set the item from which to get the tip information
      @param item the item from which to get the tip information
      @param rect the rectangle around which the tip will be shown
      @param pixmap the pixmap to be shown. If 0, no pixmap is shown
      */
    void setItem(SvnItem*item, const QRect &rect = QRect(),
                  const QPixmap *pixmap = 0 );

    virtual bool eventFilter( QObject *, QEvent *e );

  protected:
    virtual void drawContents( QPainter *p );
    virtual void resizeEvent( QResizeEvent * );

  private slots:
    void gotPreview( const KFileItem*, const QPixmap& );
    void gotPreviewResult();

    void startDelayed();
    void showTip();
    void hideTip();

  private:
    void setFilter( bool enable );

    void reposition();

    QLabel*    m_iconLabel;
    QLabel*    m_textLabel;
    bool       m_on : 1;
    bool       m_preview : 1;  // shall the preview icon be shown
    bool       m_filter : 1;
    QPixmap    m_corners[4];
    int        m_corner;
    int        m_num;
    QScrollView* m_view;
    SvnItem* m_svnitem;
    KIO::PreviewJob* m_previewJob;
    QRect      m_rect;
    QTimer*    m_timer;
};

#endif
