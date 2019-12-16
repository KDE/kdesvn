/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   https://kde.org/applications/development/org.kde.kdesvn               *
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
/* This file was part of KCachegrind.
   Copyright (C) 2002, 2003 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>
   Adapted for the needs of kdesvn  by Rajko Albrecht <ral@alwins-world.de>
*/
/*
 * A Widget for visualizing hierarchical metrics as areas.
 * The API is similar to QListView.
 */

#include "drawparams.h"

#include <math.h>

#include <QFontDatabase>
#include <QPainter>

// set this to 1 to enable debug output
#define DEBUG_DRAWING 0
#define MAX_FIELD 12

//
// StoredDrawParams
//
StoredDrawParams::StoredDrawParams()
    : _backColor(Qt::white)
    , _selected(false)
    , _current(false)
    , _shaded(true)
    , _rotated(false)
    , _drawFrame(false)
{
    // field array has size 0
}

StoredDrawParams::StoredDrawParams(const QColor &c,
                                   bool selected,
                                   bool current)
    : _backColor(c)
    , _selected(selected)
    , _current(current)
    , _shaded(true)
    , _rotated(false)
    , _drawFrame(true)
{
    // field array has size 0
}

QString StoredDrawParams::text(int f) const
{
    if ((f < 0) || (f >= _field.size())) {
        return QString();
    }

    return _field[f].text;
}

QPixmap StoredDrawParams::pixmap(int f) const
{
    if ((f < 0) || (f >= _field.size())) {
        return QPixmap();
    }

    return _field[f].pix;
}

DrawParams::Position StoredDrawParams::position(int f) const
{
    if ((f < 0) || (f >= _field.size())) {
        return Default;
    }

    return _field[f].pos;
}

int StoredDrawParams::maxLines(int f) const
{
    if ((f < 0) || (f >= _field.size())) {
        return 0;
    }

    return _field[f].maxLines;
}

QFont StoredDrawParams::font() const
{
    return QFontDatabase::systemFont(QFontDatabase::FixedFont);
}

void StoredDrawParams::ensureField(int f)
{
    static Field *def = nullptr;
    if (!def) {
        def = new Field();
        def->pos = Default;
        def->maxLines = 0;
    }

    if (f < 0 || f >= MAX_FIELD) {
        return;
    }

    while (_field.size() < f + 1) {
        _field.append(*def);
    }
}

void StoredDrawParams::setField(int f, const QString &t, const QPixmap &pm,
                                Position p, int maxLines)
{
    if (f < 0 || f >= MAX_FIELD) {
        return;
    }
    ensureField(f);

    _field[f].text = t;
    _field[f].pix  = pm;
    _field[f].pos  = p;
    _field[f].maxLines = maxLines;
}

void StoredDrawParams::setText(int f, const QString &t)
{
    if (f < 0 || f >= MAX_FIELD) {
        return;
    }
    ensureField(f);

    _field[f].text = t;
}

void StoredDrawParams::setPixmap(int f, QPixmap pm)
{
    if (f < 0 || f >= MAX_FIELD) {
        return;
    }
    ensureField(f);

    _field[f].pix = pm;
}

void StoredDrawParams::setPosition(int f, Position p)
{
    if (f < 0 || f >= MAX_FIELD) {
        return;
    }
    ensureField(f);

    _field[f].pos = p;
}

void StoredDrawParams::setMaxLines(int f, int m)
{
    if (f < 0 || f >= MAX_FIELD) {
        return;
    }
    ensureField(f);

    _field[f].maxLines = m;
}

//
// RectDrawing
//

RectDrawing::RectDrawing(const QRect &r)
  : _fm(nullptr)
  , _dp(nullptr)
{
    setRect(r);
}

RectDrawing::~RectDrawing()
{
    delete _fm;
    delete _dp;
}

DrawParams *RectDrawing::drawParams()
{
    if (!_dp) {
        _dp = new StoredDrawParams();
    }

    return _dp;
}

void RectDrawing::setDrawParams(DrawParams *dp)
{
    if (_dp) {
        delete _dp;
    }
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

QRect RectDrawing::remainingRect(DrawParams *dp)
{
    if (!dp) {
        dp = drawParams();
    }

    if ((_usedTopLeft > 0) ||
            (_usedTopCenter > 0) ||
            (_usedTopRight > 0)) {
        if (dp->rotated()) {
            _rect.setLeft(_rect.left() + _fontHeight);
        } else {
            _rect.setTop(_rect.top() + _fontHeight);
        }
    }

    if ((_usedBottomLeft > 0) ||
            (_usedBottomCenter > 0) ||
            (_usedBottomRight > 0)) {
        if (dp->rotated()) {
            _rect.setRight(_rect.right() - _fontHeight);
        } else {
            _rect.setBottom(_rect.bottom() - _fontHeight);
        }
    }
    return _rect;
}

void RectDrawing::drawBack(QPainter *p, DrawParams *dp)
{
    if (!dp) {
        dp = drawParams();
    }
    if (_rect.width() <= 0 || _rect.height() <= 0) {
        return;
    }

    QRect r = _rect;
    QColor normal = dp->backColor();
    if (dp->selected()) {
        normal = normal.lighter();
    }
    bool isCurrent = dp->current();

    if (dp->drawFrame() || isCurrent) {
        // 3D raised/sunken frame effect...
        QColor high = normal.lighter();
        QColor low = normal.darker();
        p->setPen(isCurrent ? low : high);
        p->drawLine(r.left(), r.top(), r.right(), r.top());
        p->drawLine(r.left(), r.top(), r.left(), r.bottom());
        p->setPen(isCurrent ? high : low);
        p->drawLine(r.right(), r.top(), r.right(), r.bottom());
        p->drawLine(r.left(), r.bottom(), r.right(), r.bottom());
        r.setRect(r.x() + 1, r.y() + 1, r.width() - 2, r.height() - 2);
    }
    if (r.width() <= 0 || r.height() <= 0) {
        return;
    }
    if (dp->shaded() && (r.width() > 0 && r.height() > 0)) {
        // adjustment for drawRect semantic in Qt4: decrement height/width
        r.setRect(r.x(), r.y(), r.width() - 1, r.height() - 1);

        // some shading
        bool goDark = qGray(normal.rgb()) > 128;
        int rBase, gBase, bBase;
        normal.getRgb(&rBase, &gBase, &bBase);
        p->setBrush(Qt::NoBrush);

        // shade parameters:
        int d = 7;
        double factor = 0.1, forth = 0.7, back1 = 0.9, toBack2 = .7, back2 = 0.97;

        // coefficient corrections because of rectangle size
        int s = r.width();
        if (s > r.height()) {
            s = r.height();
        }
        if (s < 100) {
            forth -= .3  * (100 - s) / 100;
            back1 -= .2  * (100 - s) / 100;
            back2 -= .02 * (100 - s) / 100;
        }

        // maximal color difference
        int rDiff = goDark ? -rBase / d : (255 - rBase) / d;
        int gDiff = goDark ? -gBase / d : (255 - gBase) / d;
        int bDiff = goDark ? -bBase / d : (255 - bBase) / d;

        QColor shadeColor;
        while (factor < .95 && (r.width() >= 0 && r.height() >= 0)) {
            shadeColor.setRgb(qRound(rBase + factor * rDiff),
                              qRound(gBase + factor * gDiff),
                              qRound(bBase + factor * bDiff));
            p->setPen(shadeColor);
            p->drawRect(r);
            r.setRect(r.x() + 1, r.y() + 1, r.width() - 2, r.height() - 2);
            factor = 1.0 - ((1.0 - factor) * forth);
        }

        // and back (1st half)
        while (factor > toBack2 && (r.width() >= 0 && r.height() >= 0)) {
            shadeColor.setRgb(qRound(rBase + factor * rDiff),
                              qRound(gBase + factor * gDiff),
                              qRound(bBase + factor * bDiff));
            p->setPen(shadeColor);
            p->drawRect(r);
            r.setRect(r.x() + 1, r.y() + 1, r.width() - 2, r.height() - 2);
            factor = 1.0 - ((1.0 - factor) / back1);
        }

        // and back (2nd half)
        while (factor > .01 && (r.width() >= 0 && r.height() >= 0)) {
            shadeColor.setRgb(qRound(rBase + factor * rDiff),
                              qRound(gBase + factor * gDiff),
                              qRound(bBase + factor * bDiff));
            p->setPen(shadeColor);
            p->drawRect(r);
            r.setRect(r.x() + 1, r.y() + 1, r.width() - 2, r.height() - 2);
            factor = factor * back2;
        }

        normal = shadeColor;
        // for filling, width and height has to be incremented again
        r.setRect(r.x(), r.y(), r.width() + 1, r.height() + 1);
    }

    // fill inside
    p->fillRect(r, normal);
}

/* Helper for drawField
* Find a line break position in a string, given a font and maximum width
*
* Returns the actually used width, and sets <breakPos>
*/
static
int findBreak(int &breakPos, QString text, QFontMetrics *fm, int maxWidth)
{
    int usedWidth;

    // does full text fit?
    breakPos = text.length();
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    usedWidth = fm->horizontalAdvance(text);
#else
    usedWidth = fm->width(text);
#endif
    if (usedWidth < maxWidth) {
        return usedWidth;
    }

    // now lower breakPos until best position is found.
    // first by binary search, resulting in a position a little bit too large
    int bottomPos = 0;
    while (qAbs(maxWidth - usedWidth) > 3 * fm->maxWidth()) {
        int halfPos = (bottomPos + breakPos) / 2;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        int halfWidth = fm->horizontalAdvance(text, halfPos);
#else
        int halfWidth = fm->width(text, halfPos);
#endif
        if (halfWidth < maxWidth) {
            bottomPos = halfPos;
        } else {
            breakPos = halfPos;
            usedWidth = halfWidth;
        }
    }

    // final position by taking break boundaries into account.
    // possible break boundaries are changing char categories but not middle of "Aa"
    QChar::Category lastCat, cat;
    int pos = breakPos;
    lastCat = text[pos - 1].category();
    // at minimum 2 chars before break
    while (pos > 2) {
        pos--;
        cat = text[pos - 1].category();
        if (cat == lastCat) {
            continue;
        }

        // "Aa" has not a possible break inbetween
        if ((cat == QChar::Letter_Uppercase) &&
                (lastCat == QChar::Letter_Lowercase)) {
            lastCat = cat;
            continue;
        }
        lastCat = cat;

        breakPos = pos;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        usedWidth = fm->horizontalAdvance(text, breakPos);
#else
        usedWidth = fm->width(text, breakPos);
#endif
        if (usedWidth < maxWidth) {
            break;
        }
    }
    return usedWidth;
}

/* Helper for drawField
* Find last line break position in a string from backwards,
* given a font and maximum width
*
* Returns the actually used width, and sets <breakPos>
*/
static
int findBreakBackwards(int &breakPos, QString text, QFontMetrics *fm, int maxWidth)
{
    int usedWidth;

    // does full text fit?
    breakPos = 0;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    usedWidth = fm->horizontalAdvance(text);
#else
    usedWidth = fm->width(text);
#endif
    if (usedWidth < maxWidth) {
        return usedWidth;
    }

    // now raise breakPos until best position is found.
    // first by binary search, resulting in a position a little bit too small
    int topPos = text.length();
    while (qAbs(maxWidth - usedWidth) > 3 * fm->maxWidth()) {
        int halfPos = (breakPos + topPos) / 2;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        int halfWidth = fm->horizontalAdvance(text.mid(halfPos));
#else
        int halfWidth = fm->width(text.mid(halfPos));
#endif
        if (halfWidth < maxWidth) {
            breakPos = halfPos;
            usedWidth = halfWidth;
        } else {
            topPos = halfPos;
        }
    }

    // final position by taking break boundaries into account.
    // possible break boundaries are changing char categories but not middle of "Aa"
    QChar::Category lastCat, cat;
    int pos = breakPos;
    lastCat = text[pos].category();
    // at minimum 2 chars before break
    while (pos < text.length() - 2) {
        pos++;
        cat = text[pos].category();
        if (cat == lastCat) {
            continue;
        }

        // "Aa" has not a possible break inbetween
        if ((lastCat == QChar::Letter_Uppercase) &&
                (cat == QChar::Letter_Lowercase)) {
            lastCat = cat;
            continue;
        }
        lastCat = cat;

        breakPos = pos;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        usedWidth = fm->horizontalAdvance(text.mid(breakPos));
#else
        usedWidth = fm->width(text.mid(breakPos));
#endif
        if (usedWidth < maxWidth) {
            break;
        }
    }
    return usedWidth;
}

bool RectDrawing::drawField(QPainter *p, int f, DrawParams *dp)
{
    if (!dp) {
        dp = drawParams();
    }

    if (!_fm) {
        _fm = new QFontMetrics(dp->font());
        _fontHeight = _fm->height();
    }

    QRect r = _rect;

    int h = _fontHeight;
    bool rotate = dp->rotated();
    int width   = (rotate ? r.height() : r.width()) - 4;
    int height  = (rotate ? r.width() : r.height());
    int lines   = height / h;

    // stop if there is no space available
    if (lines < 1) {
        return false;
    }

    // calculate free space in first line (<unused>)
    int pos = dp->position(f);
    if (pos == DrawParams::Default) {
        switch (f % 4) {
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
    int *used = nullptr;
    switch (pos) {
    case DrawParams::TopLeft:
        used = &_usedTopLeft;
        if (_usedTopLeft == 0) {
            if (_usedTopCenter) {
                unused = (width - _usedTopCenter) / 2;
            } else {
                unused = width - _usedTopRight;
            }
        }
        break;

    case DrawParams::TopCenter:
        isCenter = true;
        used = &_usedTopCenter;
        if (_usedTopCenter == 0) {
            if (_usedTopLeft > _usedTopRight) {
                unused = width - 2 * _usedTopLeft;
            } else {
                unused = width - 2 * _usedTopRight;
            }
        }
        break;

    case DrawParams::TopRight:
        isRight = true;
        used = &_usedTopRight;
        if (_usedTopRight == 0) {
            if (_usedTopCenter) {
                unused = (width - _usedTopCenter) / 2;
            } else {
                unused = width - _usedTopLeft;
            }
        }
        break;

    case DrawParams::BottomLeft:
        isBottom = true;
        used = &_usedBottomLeft;
        if (_usedBottomLeft == 0) {
            if (_usedBottomCenter) {
                unused = (width - _usedBottomCenter) / 2;
            } else {
                unused = width - _usedBottomRight;
            }
        }
        break;

    case DrawParams::BottomCenter:
        isCenter = true;
        isBottom = true;
        used = &_usedBottomCenter;
        if (_usedBottomCenter == 0) {
            if (_usedBottomLeft > _usedBottomRight) {
                unused = width - 2 * _usedBottomLeft;
            } else {
                unused = width - 2 * _usedBottomRight;
            }
        }
        break;

    case DrawParams::BottomRight:
        isRight = true;
        isBottom = true;
        used = &_usedBottomRight;
        if (_usedBottomRight == 0) {
            if (_usedBottomCenter) {
                unused = (width - _usedBottomCenter) / 2;
            } else {
                unused = width - _usedBottomLeft;
            }
        }
        break;
    }
    if (isBottom) {
        if ((_usedTopLeft > 0) ||
                (_usedTopCenter > 0) ||
                (_usedTopRight > 0)) {
            lines--;
        }
    } else if (!isBottom) {
        if ((_usedBottomLeft > 0) ||
                (_usedBottomCenter > 0) ||
                (_usedBottomRight > 0)) {
            lines--;
        }
    }
    if (lines < 1) {
        return false;
    }

    int y = isBottom ? height - h : 0;

    if (unused < 0) {
        unused = 0;
    }
    if (unused == 0) {
        // no space available in last line at this position
        y = isBottom ? (y - h) : (y + h);
        lines--;

        if (lines < 1) {
            return false;
        }

        // new line: reset used space
        if (isBottom) {
            _usedBottomLeft = _usedBottomCenter = _usedBottomRight = 0;
        } else {
            _usedTopLeft = _usedTopCenter = _usedTopRight = 0;
        }

        unused = width;
    }

    // stop as soon as possible when there is no space for "..."
    static int dotW = 0;
    if (!dotW) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        dotW = _fm->horizontalAdvance(QLatin1String("..."));
#else
        dotW = _fm->width(QLatin1String("..."));
#endif
    }
    if (width < dotW) {
        return false;
    }

    // get text and pixmap now, only if we need to, because it is possible
    // that they are calculated on demand (and this can take some time)
    QString name = dp->text(f);
    if (name.isEmpty()) {
        return false;
    }
    QPixmap pix = dp->pixmap(f);

    // check if pixmap can be drawn
    int pixW = pix.width();
    int pixH = pix.height();
    int pixY = 0;
    bool pixDrawn = true;
    if (pixW > 0) {
        pixW += 2; // X distance from pix
        if ((width < pixW + dotW) || (height < pixH)) {
            // do not draw
            pixW = 0;
        } else {
            pixDrawn = false;
        }
    }

    // width of text and pixmap to be drawn
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    int w = pixW + _fm->horizontalAdvance(name);
#else
    int w = pixW + _fm->width(name);
#endif

    // if we have limited space at 1st line:
    // use it only if whole name does fit in last line...
    if ((unused < width) && (w > unused)) {
        y = isBottom ? (y - h) : (y + h);
        lines--;

        if (lines < 1) {
            return false;
        }

        // new line: reset used space
        if (isBottom) {
            _usedBottomLeft = _usedBottomCenter = _usedBottomRight = 0;
        } else {
            _usedTopLeft = _usedTopCenter = _usedTopRight = 0;
        }
    }

    p->save();
    p->setPen((qGray(dp->backColor().rgb()) > 100) ? Qt::black : Qt::white);
    p->setFont(dp->font());
    if (rotate) {
        //p->translate(r.x()+2, r.y()+r.height());
        p->translate(r.x(), r.y() + r.height() - 2);
        p->rotate(270);
    } else {
        p->translate(r.x() + 2, r.y());
    }
    // adjust available lines according to maxLines
    int max = dp->maxLines(f);
    if ((max > 0) && (lines > max)) {
        lines = max;
    }
    /* loop over name parts to break up string depending on available width.
     * every char category change is supposed a possible break,
     * with the exception Uppercase=>Lowercase.
     * It is good enough for numbers, Symbols...
     *
     * If the text is to be written at the bottom, we start with the
     * end of the string (so everything is reverted)
    */
    QString remaining;
    int origLines = lines;
    while (lines > 0) {

        // more than one line: search for line break
        if (w > width && lines > 1) {
            int breakPos;

            if (!isBottom) {
                w = pixW + findBreak(breakPos, name, _fm, width - pixW);

                remaining = name.mid(breakPos);
                // remove space on break point
                if (name[breakPos - 1].category() == QChar::Separator_Space) {
                    name = name.left(breakPos - 1);
                } else {
                    name = name.left(breakPos);
                }
            } else { // bottom
                w = pixW + findBreakBackwards(breakPos, name, _fm, width - pixW);

                remaining = name.left(breakPos);
                // remove space on break point
                if (name[breakPos].category() == QChar::Separator_Space) {
                    name = name.mid(breakPos + 1);
                } else {
                    name = name.mid(breakPos);
                }
            }
        } else {
            remaining.clear();
        }
        /* truncate and add ... if needed */
        if (w > width) {
            name = _fm->elidedText(name, Qt::ElideRight, width - pixW);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
            w = _fm->horizontalAdvance(name) + pixW;
#else
            w = _fm->width(name) + pixW;
#endif
        }

        int x = 0;
        if (isCenter) {
            x = (width - w) / 2;
        } else if (isRight) {
            x = width - w;
        }
        if (!pixDrawn) {
            pixY = y + (h - pixH) / 2; // default: center vertically
            if (pixH > h) {
                pixY = isBottom ? y - (pixH - h) : y;
            }

            p->drawPixmap(x, pixY, pix);

            // for distance to next text
            pixY = isBottom ? (pixY - h - 2) : (pixY + pixH + 2);
            pixDrawn = true;
        }
        p->drawText(x + pixW, y,
                    width - pixW, h,
                    Qt::AlignLeft, name);
        y = isBottom ? (y - h) : (y + h);
        lines--;

        if (remaining.isEmpty()) {
            break;
        }
        name = remaining;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        w = pixW + _fm->horizontalAdvance(name);
#else
        w = pixW + _fm->width(name);
#endif
    }

    // make sure the pix stays visible
    if (pixDrawn && (pixY > 0)) {
        if (isBottom && (pixY < y)) {
            y = pixY;
        }
        if (!isBottom && (pixY > y)) {
            y = pixY;
        }
    }

    if (origLines > lines) {
        // if only 1 line written, do not reset _used* vars
        if (lines - origLines > 1) {
            if (isBottom) {
                _usedBottomLeft = _usedBottomCenter = _usedBottomRight = 0;
            } else {
                _usedTopLeft = _usedTopCenter = _usedTopRight = 0;
            }
        }

        // take back one line
        y = isBottom ? (y + h) : (y - h);
        if (used) {
            *used = w;
        }
    }

    // update free space
    if (!isBottom) {
        if (rotate) {
            _rect.setRect(r.x() + y, r.y(), r.width() - y, r.height());
        } else {
            _rect.setRect(r.x(), r.y() + y, r.width(), r.height() - y);
        }
    } else {
        if (rotate) {
            _rect.setRect(r.x(), r.y(), y + h, r.height());
        } else {
            _rect.setRect(r.x(), r.y(), r.width(), y + h);
        }
    }

    p->restore();

    return true;
}
