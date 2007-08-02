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

/*
 * A Widget for visualizing hierarchical metrics as areas.
 * The API is similar to QListView.
 */

#include <math.h>

#include <qpainter.h>
#include <qtooltip.h>
#include <qregexp.h>
#include <qstyle.h>
#include <q3popupmenu.h>
//Added by qt3to4:
#include <QPixmap>

#include <klocale.h>
#include <kconfig.h>
#include <kdebug.h>

#include "drawparams.h"


// set this to 1 to enable debug output
#define DEBUG_DRAWING 0
#define MAX_FIELD 12


//
// StoredDrawParams
//
StoredDrawParams::StoredDrawParams()
{
  _selected = false;
  _current = false;
  _shaded = true;
  _rotated = false;

  _backColor = Qt::white;

  // field array has size 0
}

StoredDrawParams::StoredDrawParams(QColor c,
                                   bool selected, bool current)
{
  _backColor = c;

  _selected = selected;
  _current = current;
  _shaded = true;
  _rotated = false;
  _drawFrame = true;

  // field array has size 0
}

QString StoredDrawParams::text(int f) const
{
  if ((f<0) || (f >= (int)_field.size()))
    return QString::null;

  return _field[f].text;
}

QPixmap StoredDrawParams::pixmap(int f) const
{
  if ((f<0) || (f >= (int)_field.size()))
    return QPixmap();

  return _field[f].pix;
}

DrawParams::Position StoredDrawParams::position(int f) const
{
  if ((f<0) || (f >= (int)_field.size()))
    return Default;

  return _field[f].pos;
}

int StoredDrawParams::maxLines(int f) const
{
  if ((f<0) || (f >= (int)_field.size()))
    return 0;

  return _field[f].maxLines;
}

const QFont& StoredDrawParams::font() const
{
  static QFont* f = 0;
  if (!f) f = new QFont(QApplication::font());

  return *f;
}

void StoredDrawParams::ensureField(int f)
{
  static Field* def = 0;
  if (!def) {
    def = new Field();
    def->pos = Default;
    def->maxLines = 0;
  }

  if (f<0 || f>=MAX_FIELD) return;

  if ((int)_field.size() < f+1) _field.resize(f+1, *def);
}


void StoredDrawParams::setField(int f, QString t, QPixmap pm,
                           Position p, int maxLines)
{
  if (f<0 || f>=MAX_FIELD) return;
  ensureField(f);

  _field[f].text = t;
  _field[f].pix  = pm;
  _field[f].pos  = p;
  _field[f].maxLines = maxLines;
}

void StoredDrawParams::setText(int f, QString t)
{
  if (f<0 || f>=MAX_FIELD) return;
  ensureField(f);

  _field[f].text = t;
}

void StoredDrawParams::setPixmap(int f, QPixmap pm)
{
  if (f<0 || f>=MAX_FIELD) return;
  ensureField(f);

  _field[f].pix = pm;
}

void StoredDrawParams::setPosition(int f, Position p)
{
  if (f<0 || f>=MAX_FIELD) return;
  ensureField(f);

  _field[f].pos = p;
}

void StoredDrawParams::setMaxLines(int f, int m)
{
  if (f<0 || f>=MAX_FIELD) return;
  ensureField(f);

  _field[f].maxLines = m;
}



//
// RectDrawing
//

RectDrawing::RectDrawing(QRect r)
{
  _fm = 0;
  _dp = 0;
  setRect(r);
}


RectDrawing::~RectDrawing()
{
  delete _fm;
  delete _dp;
}

DrawParams* RectDrawing::drawParams()
{
  if (!_dp)
    _dp = new StoredDrawParams();

  return _dp;
}


void RectDrawing::setDrawParams(DrawParams* dp)
{
  if (_dp) delete _dp;
  _dp = dp;
}

void RectDrawing::setRect(QRect r)
{
  _rect = r;

  _usedTopLeft = 0;
  _usedTopCenter = 0;
  _usedTopRight = 0;
  _usedBottomLeft = 0;
  _usedBottomCenter = 0;
  _usedBottomRight = 0;

  _fontHeight = 0;
}

QRect RectDrawing::remainingRect(DrawParams* dp)
{
  if (!dp) dp = drawParams();

  if ((_usedTopLeft >0) ||
      (_usedTopCenter >0) ||
      (_usedTopRight >0)) {
    if (dp->rotated())
      _rect.setLeft(_rect.left() + _fontHeight);
    else
      _rect.setTop(_rect.top() + _fontHeight);
  }

  if ((_usedBottomLeft >0) ||
      (_usedBottomCenter >0) ||
      (_usedBottomRight >0)) {
    if (dp->rotated())
      _rect.setRight(_rect.right() - _fontHeight);
    else
      _rect.setBottom(_rect.bottom() - _fontHeight);
  }
  return _rect;
}


void RectDrawing::drawBack(QPainter* p, DrawParams* dp)
{
  if (!dp) dp = drawParams();
  if (_rect.width()<=0 || _rect.height()<=0) return;

  QRect r = _rect;
  QColor normal = dp->backColor();
  if (dp->selected()) normal = normal.light();
  bool isCurrent = dp->current();

  if (dp->drawFrame() || isCurrent) {
    // 3D raised/sunken frame effect...
    QColor high = normal.light();
    QColor low = normal.dark();
    p->setPen( isCurrent ? low:high);
    p->drawLine(r.left(), r.top(), r.right(), r.top());
    p->drawLine(r.left(), r.top(), r.left(), r.bottom());
    p->setPen( isCurrent ? high:low);
    p->drawLine(r.right(), r.top(), r.right(), r.bottom());
    p->drawLine(r.left(), r.bottom(), r.right(), r.bottom());
    r.setRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2);
  }
  if (r.width()<=0 || r.height()<=0) return;

  if (dp->shaded()) {
    // some shading
    bool goDark = qGray(normal.rgb())>128;
    int rBase, gBase, bBase;
    normal.rgb(&rBase, &gBase, &bBase);
    p->setBrush(QBrush::NoBrush);

    // shade parameters:
    int d = 7;
    float factor = 0.1, forth=0.7, back1 =0.9, toBack2 = .7, back2 = 0.97;

    // coefficient corrections because of rectangle size
    int s = r.width();
    if (s > r.height()) s = r.height();
    if (s<100) {
      forth -= .3  * (100-s)/100;
      back1 -= .2  * (100-s)/100;
      back2 -= .02 * (100-s)/100;
    }


    // maximal color difference
    int rDiff = goDark ? -rBase/d : (255-rBase)/d;
    int gDiff = goDark ? -gBase/d : (255-gBase)/d;
    int bDiff = goDark ? -bBase/d : (255-bBase)/d;

    QColor shadeColor;
    while (factor<.95) {
      shadeColor.setRgb((int)(rBase+factor*rDiff+.5),
                        (int)(gBase+factor*gDiff+.5),
                        (int)(bBase+factor*bDiff+.5));
      p->setPen(shadeColor);
      p->drawRect(r);
      r.setRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2);
      if (r.width()<=0 || r.height()<=0) return;
      factor = 1.0 - ((1.0 - factor) * forth);
    }

    // and back (1st half)
    while (factor>toBack2) {
      shadeColor.setRgb((int)(rBase+factor*rDiff+.5),
                        (int)(gBase+factor*gDiff+.5),
                        (int)(bBase+factor*bDiff+.5));
      p->setPen(shadeColor);
      p->drawRect(r);
      r.setRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2);
      if (r.width()<=0 || r.height()<=0) return;
      factor = 1.0 - ((1.0 - factor) / back1);
    }

    // and back (2nd half)
    while ( factor>.01) {
      shadeColor.setRgb((int)(rBase+factor*rDiff+.5),
                        (int)(gBase+factor*gDiff+.5),
                        (int)(bBase+factor*bDiff+.5));
      p->setPen(shadeColor);
      p->drawRect(r);
      r.setRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2);
      if (r.width()<=0 || r.height()<=0) return;

      factor = factor * back2;
    }
  }

  // fill inside
  p->setPen(QPen::NoPen);
  p->setBrush(normal);
  p->drawRect(r);
}


bool RectDrawing::drawField(QPainter* p, int f, DrawParams* dp)
{
  if (!dp) dp = drawParams();

  if (!_fm) {
    _fm = new QFontMetrics(dp->font());
    _fontHeight = _fm->height();
  }

  QRect r = _rect;

  if (0) kdDebug(90100) << "DrawField: Rect " << r.x() << "/" << r.y()
		   << " - " << r.width() << "x" << r.height() << endl;

  int h = _fontHeight;
  bool rotate = dp->rotated();
  int width   = (rotate ? r.height() : r.width()) -4;
  int height  = (rotate ? r.width() : r.height());
  int lines   = height / h;

  // stop if we have no space available
  if (lines<1) return false;

  // calculate free space in first line (<unused>)
  int pos = dp->position(f);
  if (pos == DrawParams::Default) {
    switch(f%4) {
    case 0: pos = DrawParams::TopLeft; break;
    case 1: pos = DrawParams::TopRight; break;
    case 2: pos = DrawParams::BottomRight; break;
    case 3: pos = DrawParams::BottomLeft; break;
    }
  }

  int unused = 0;
  bool isBottom = false;
  bool isCenter = false;
  bool isRight = false;
  int* used = 0;
  switch(pos) {
  case DrawParams::TopLeft:
    used = &_usedTopLeft;
    if (_usedTopLeft == 0) {
      if (_usedTopCenter)
        unused = (width - _usedTopCenter)/2;
      else
        unused = width - _usedTopRight;
    }
    break;

  case DrawParams::TopCenter:
    isCenter = true;
    used = &_usedTopCenter;
    if (_usedTopCenter == 0) {
      if (_usedTopLeft > _usedTopRight)
        unused = width - 2 * _usedTopLeft;
      else
        unused = width - 2 * _usedTopRight;
    }
    break;

  case DrawParams::TopRight:
    isRight = true;
    used = &_usedTopRight;
    if (_usedTopRight == 0) {
      if (_usedTopCenter)
        unused = (width - _usedTopCenter)/2;
      else
        unused = width - _usedTopLeft;
    }
    break;

  case DrawParams::BottomLeft:
    isBottom = true;
    used = &_usedBottomLeft;
    if (_usedBottomLeft == 0) {
      if (_usedBottomCenter)
        unused = (width - _usedBottomCenter)/2;
      else
        unused = width - _usedBottomRight;
    }
    break;

  case DrawParams::BottomCenter:
    isCenter = true;
    isBottom = true;
    used = &_usedBottomCenter;
    if (_usedBottomCenter == 0) {
      if (_usedBottomLeft > _usedBottomRight)
        unused = width - 2 * _usedBottomLeft;
      else
        unused = width - 2 * _usedBottomRight;
    }
    break;

  case DrawParams::BottomRight:
    isRight = true;
    isBottom = true;
    used = &_usedBottomRight;
    if (_usedBottomRight == 0) {
      if (_usedBottomCenter)
        unused = (width - _usedBottomCenter)/2;
      else
        unused = width - _usedBottomLeft;
    }
    break;
  }

  if (isBottom) {
    if ((_usedTopLeft >0) ||
        (_usedTopCenter >0) ||
        (_usedTopRight >0))
      lines--;
  }
  else if (!isBottom) {
    if ((_usedBottomLeft >0) ||
        (_usedBottomCenter >0) ||
        (_usedBottomRight >0))
      lines--;
  }
  if (lines<1) return false;


  int y = isBottom ? height - h : 0;

  if (unused < 0) unused = 0;
  if (unused == 0) {
    // no space available in last line at this position
    y = isBottom ? (y-h) : (y+h);
    lines--;

    if (lines<1) return false;

    // new line: reset used space
    if (isBottom)
      _usedBottomLeft = _usedBottomCenter = _usedBottomRight = 0;
    else
      _usedTopLeft = _usedTopCenter = _usedTopRight = 0;

    unused = width;
  }

  // stop as soon as possible when there's no space for "..."
  static int dotW = 0;
  if (!dotW) dotW = _fm->width("...");
  if (width < dotW) return false;

  // get text and pixmap now, only if we need to, because it is possible
  // that they are calculated on demand (and this can take some time)
  QString name = dp->text(f);
  if (name.isEmpty()) return 0;
  QPixmap pix = dp->pixmap(f);

  // check if pixmap can be drawn
  int pixW = pix.width();
  int pixH = pix.height();
  int pixY = 0;
  bool pixDrawn = true;
  if (pixW>0) {
    pixW += 2; // X distance from pix
    if ((width < pixW + dotW) || (height < pixH)) {
      // don't draw
      pixW = 0;
    }
    else
      pixDrawn = false;
  }

  // width of text and pixmap to be drawn
  int w = pixW + _fm->width(name);

  if (0) kdDebug(90100) << "  For '" << name << "': Unused " << unused
		   << ", StrW " << w << ", Width " << width << endl;

  // if we have limited space at 1st line:
  // use it only if whole name does fit in last line...
  if ((unused < width) && (w > unused)) {
    y = isBottom ? (y-h) : (y+h);
    lines--;

    if (lines<1) return false;

    // new line: reset used space
    if (isBottom)
      _usedBottomLeft = _usedBottomCenter = _usedBottomRight = 0;
    else
      _usedTopLeft = _usedTopCenter = _usedTopRight = 0;
  }

  p->save();
  p->setPen( (qGray(dp->backColor().rgb())>100) ? Qt::black : Qt::white);
  p->setFont(dp->font());
  if (rotate) {
    //p->translate(r.x()+2, r.y()+r.height());
    p->translate(r.x(), r.y()+r.height()-2);
    p->rotate(270);
  }
  else
    p->translate(r.x()+2, r.y());


  // adjust available lines according to maxLines
  int max = dp->maxLines(f);
  if ((max > 0) && (lines>max)) lines = max;

  /* loop over name parts to break up string depending on available width.
   * every char category change is supposed a possible break,
   * with the exception Uppercase=>Lowercase.
   * It's good enough for numbers, Symbols...
   *
   * If the text is to be written at the bottom, we start with the
   * end of the string (so everything is reverted)
   */
  QString remaining;
  int origLines = lines;
  while (lines>0) {

    if (w>width && lines>1) {
      int lastBreakPos = name.length(), lastWidth = w;
      int len = name.length();
      QChar::Category caOld, ca;

      if (!isBottom) {
        // start with comparing categories of last 2 chars
        caOld = name[len-1].category();
        while (len>2) {
          len--;
          ca = name[len-1].category();
          if (ca != caOld) {
            // "Aa" has no break between...
            if (ca == QChar::Letter_Uppercase &&
                caOld == QChar::Letter_Lowercase) {
              caOld = ca;
              continue;
            }
            caOld = ca;
            lastBreakPos = len;
            w = pixW + _fm->width(name, len);
            lastWidth = w;
            if (w <= width) break;
          }
        }
        w = lastWidth;
        remaining = name.mid(lastBreakPos);
        // remove space on break point
        if (name[lastBreakPos-1].category() == QChar::Separator_Space)
          name = name.left(lastBreakPos-1);
        else
          name = name.left(lastBreakPos);
      }
      else { // bottom
        int l = len;
        caOld = name[l-len].category();
        while (len>2) {
          len--;
          ca = name[l-len].category();

          if (ca != caOld) {
            // "Aa" has no break between...
            if (caOld == QChar::Letter_Uppercase &&
                ca == QChar::Letter_Lowercase) {
              caOld = ca;
              continue;
            }
            caOld = ca;
            lastBreakPos = len;
            w = pixW + _fm->width(name.right(len));
            lastWidth = w;
            if (w <= width) break;
          }
        }
        w = lastWidth;
        remaining = name.left(l-lastBreakPos);
        // remove space on break point
        if (name[l-lastBreakPos].category() == QChar::Separator_Space)
          name = name.right(lastBreakPos-1);
        else
          name = name.right(lastBreakPos);
      }
    }
    else
      remaining = QString::null;

    /* truncate and add ... if needed */
    if (w>width) {
      int len = name.length();
      w += dotW;
      while (len>2 && (w > width)) {
        len--;
        w = pixW + _fm->width(name, len) + dotW;
      }
      // stop drawing: we cannot draw 2 chars + "..."
      if (w>width) break;

      name = name.left(len) + "...";
    }

    int x = 0;
    if (isCenter)
      x = (width - w)/2;
    else if (isRight)
      x = width - w;

    if (!pixDrawn) {
        pixY = y+(h-pixH)/2; // default: center vertically
        if (pixH > h) pixY = isBottom ? y-(pixH-h) : y;

	p->drawPixmap( x, pixY, pix);

        // for distance to next text
	pixY = isBottom ? (pixY - h - 2) : (pixY + pixH + 2);
	pixDrawn = true;
    }


    if (0) kdDebug(90100) << "  Drawing '" << name << "' at "
		     << x+pixW << "/" << y << endl;

    p->drawText( x+pixW, y,
		 width - pixW, h,
		 Qt::AlignLeft, name);
    y = isBottom ? (y-h) : (y+h);
    lines--;

    if (remaining.isEmpty()) break;
    name = remaining;
    w = pixW + _fm->width(name);
  }

  // make sure the pix stays visible
  if (pixDrawn && (pixY>0)) {
    if (isBottom && (pixY<y)) y = pixY;
    if (!isBottom && (pixY>y)) y = pixY;
  }

  if (origLines > lines) {
    // if only 1 line written, don't reset _used* vars
    if (lines - origLines >1) {
      if (isBottom)
        _usedBottomLeft = _usedBottomCenter = _usedBottomRight = 0;
      else
        _usedTopLeft = _usedTopCenter = _usedTopRight = 0;
    }

    // take back one line
    y = isBottom ? (y+h) : (y-h);
    if (used) *used = w;
  }

  // update free space
  if (!isBottom) {
    if (rotate)
      _rect.setRect(r.x()+y, r.y(), r.width()-y, r.height());
    else
      _rect.setRect(r.x(), r.y()+y, r.width(), r.height()-y);
  }
  else {
    if (rotate)
      _rect.setRect(r.x(), r.y(), y+h, r.height());
    else
      _rect.setRect(r.x(), r.y(), r.width(), y+h);
  }

  p->restore();

  return true;
}
