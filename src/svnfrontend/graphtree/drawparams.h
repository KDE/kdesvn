/* This file is part of KCachegrind.
   Copyright (C) 2002, 2003 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>
   Adapted for the needs of kdesvn  by Rajko Albrecht <ral@alwins-world.de>

   KCachegrind is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

/**
 * A Widget for visualizing hierarchical metrics as areas.
 * The API is similar to QListView.
 *
 * This file defines the following classes:
 *  DrawParams, RectDrawing, TreeMapItem, TreeMapWidget
 *
 * DrawParams/RectDrawing allows reusing of TreeMap drawing
 * functions in other widgets.
 */

#ifndef DRAWPARAMS_H
#define DRAWPARAMS_H

#include <qstring.h>
#include <qwidget.h>
#include <qpixmap.h>
#include <qptrlist.h>
#include <qvaluevector.h>
#include <qcolor.h>
#include <qapplication.h>
#include <qstringlist.h>

class QPopupMenu;
class QString;

class KConfigGroup;


/**
 * Drawing parameters for an object.
 * A Helper Interface for RectDrawing.
 */
class DrawParams
{
public:
  /**
   * Positions for drawing into a rectangle.
   *
   * The specified position assumes no rotation.
   * If there is more than one text for one position, it is put
   * nearer to the center of the item.
   *
   * Drawing at top positions cuts free space from top,
   * drawing at bottom positions cuts from bottom.
   * Default usually gives positions clockwise according to field number.
   */
  enum Position { TopLeft, TopCenter, TopRight,
                  BottomLeft, BottomCenter, BottomRight,
                  Default, Unknown};

  // no constructor as this is an abstract class
  virtual ~DrawParams() {}

  virtual QString  text(int) const = 0;
  virtual QPixmap  pixmap(int) const = 0;
  virtual Position position(int) const = 0;
  // 0: no limit, negative: leave at least -maxLines() free
  virtual int      maxLines(int) const { return 0; }
  virtual int      fieldCount() const { return 0; }

  virtual QColor   backColor() const { return Qt::white; }
  virtual const QFont& font() const = 0;

  virtual bool selected() const { return false; }
  virtual bool current() const { return false; }
  virtual bool shaded() const { return true; }
  virtual bool rotated() const { return false; }
  virtual bool drawFrame() const { return true; }
};


/*
 * DrawParam with attributes stored
 */
class StoredDrawParams: public DrawParams
{
public:
  StoredDrawParams();
  StoredDrawParams(QColor c,
                   bool selected = false, bool current = false);

  // getters
  QString  text(int) const;
  QPixmap  pixmap(int) const;
  Position position(int) const;
  int      maxLines(int) const;
  int      fieldCount() const { return _field.size(); }

  QColor   backColor() const { return _backColor; }
  bool selected() const { return _selected; }
  bool current() const { return _current; }
  bool shaded() const { return _shaded; }
  bool rotated() const { return _rotated; }
  bool drawFrame() const { return _drawFrame; }

  const QFont& font() const;

  // attribute setters
  void setField(int f, QString t, QPixmap pm = QPixmap(),
                Position p = Default, int maxLines = 0);
  void setText(int f, QString);
  void setPixmap(int f, QPixmap);
  void setPosition(int f, Position);
  void setMaxLines(int f, int);
  void setBackColor(QColor c) { _backColor = c; }
  void setSelected(bool b) { _selected = b; }
  void setCurrent(bool b) { _current = b; }
  void setShaded(bool b) { _shaded = b; }
  void setRotated(bool b) { _rotated = b; }
  void drawFrame(bool b) { _drawFrame = b; }

protected:
  QColor _backColor;
  bool _selected :1;
  bool _current :1;
  bool _shaded :1;
  bool _rotated :1;
  bool _drawFrame :1;

private:
  // resize field array if needed to allow to access field <f>
  void ensureField(int f);

  struct Field {
    QString text;
    QPixmap pix;
    Position pos;
    int maxLines;
  };

  QValueVector<Field> _field;
};


/* State for drawing on a rectangle.
 *
 * Following drawing functions are provided:
 * - background drawing with shading and 3D frame
 * - successive pixmap/text drawing at various positions with wrap-around
 *   optimized for minimal space usage (e.g. if a text is drawn at top right
 *   after text on top left, the same line is used if space allows)
 *
 */
class RectDrawing
{
public:
  RectDrawing(QRect);
  ~RectDrawing();

  // The default DrawParams object used.
  DrawParams* drawParams();
  // we take control over the given object (i.e. delete at destruction)
  void setDrawParams(DrawParams*);

  // draw on a given QPainter, use this class as info provider per default
  void drawBack(QPainter*, DrawParams* dp = 0);
  /* Draw field at position() from pixmap()/text() with maxLines().
   * Returns true if something was drawn
   */
  bool drawField(QPainter*, int f, DrawParams* dp = 0);

  // resets rectangle for free space
  void setRect(QRect);

  // Returns the rectangle area still free of text/pixmaps after
  // a number of drawText() calls.
  QRect remainingRect(DrawParams* dp = 0);

private:
  int _usedTopLeft, _usedTopCenter, _usedTopRight;
  int _usedBottomLeft, _usedBottomCenter, _usedBottomRight;
  QRect _rect;

  // temporary
  int _fontHeight;
  QFontMetrics* _fm;
  DrawParams* _dp;
};

#endif
