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
#include <qcolor.h>
#include <qapplication.h>
#include <qstringlist.h>

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
                    Default, Unknown
                  };

    // no constructor as this is an abstract class
    virtual ~DrawParams() {}

    virtual QString  text(int) const = 0;
    virtual QPixmap  pixmap(int) const = 0;
    virtual Position position(int) const = 0;
    // 0: no limit, negative: leave at least -maxLines() free
    virtual int      maxLines(int) const
    {
        return 0;
    }
    virtual int      fieldCount() const
    {
        return 0;
    }

    virtual QColor   backColor() const
    {
        return Qt::white;
    }
    virtual QFont font() const = 0;

    virtual bool selected() const
    {
        return false;
    }
    virtual bool current() const
    {
        return false;
    }
    virtual bool shaded() const
    {
        return true;
    }
    virtual bool rotated() const
    {
        return false;
    }
    virtual bool drawFrame() const
    {
        return true;
    }
};

/*
 * DrawParam with attributes stored
 */
class StoredDrawParams: public DrawParams
{
public:
    explicit StoredDrawParams();
    explicit StoredDrawParams(const QColor &c,
                              bool selected = false,
                              bool current = false);

    // getters
    QString  text(int) const override;
    QPixmap  pixmap(int) const override;
    Position position(int) const override;
    int      maxLines(int) const override;
    int      fieldCount() const override
    {
        return _field.size();
    }

    QColor   backColor() const override
    {
        return _backColor;
    }
    bool selected() const override
    {
        return _selected;
    }
    bool current() const override
    {
        return _current;
    }
    bool shaded() const override
    {
        return _shaded;
    }
    bool rotated() const override
    {
        return _rotated;
    }
    bool drawFrame() const override
    {
        return _drawFrame;
    }

    QFont font() const override;

    // attribute setters
    void setField(int f, const QString &t, const QPixmap &pm = QPixmap(),
                  Position p = Default, int maxLines = 0);
    void setText(int f, const QString &);
    void setPixmap(int f, QPixmap);
    void setPosition(int f, Position);
    void setMaxLines(int f, int);
    void setBackColor(QColor c)
    {
        _backColor = c;
    }
    void setSelected(bool b)
    {
        _selected = b;
    }
    void setCurrent(bool b)
    {
        _current = b;
    }
    void setShaded(bool b)
    {
        _shaded = b;
    }
    void setRotated(bool b)
    {
        _rotated = b;
    }
    void drawFrame(bool b)
    {
        _drawFrame = b;
    }

protected:
    QColor _backColor;
    bool _selected : 1;
    bool _current : 1;
    bool _shaded : 1;
    bool _rotated : 1;
    bool _drawFrame : 1;

private:
    // resize field array if needed to allow to access field <f>
    void ensureField(int f);

    struct Field {
        QString text;
        QPixmap pix;
        Position pos = Unknown;
        int maxLines = 0;
    };

    QVector<Field> _field;
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
    explicit RectDrawing(const QRect &r);
    ~RectDrawing();

    // The default DrawParams object used.
    DrawParams *drawParams();
    // we take control over the given object (i.e. delete at destruction)
    void setDrawParams(DrawParams *);

    // draw on a given QPainter, use this class as info provider per default
    void drawBack(QPainter *, DrawParams *dp = nullptr);
    /* Draw field at position() from pixmap()/text() with maxLines().
     * Returns true if something was drawn
     */
    bool drawField(QPainter *, int f, DrawParams *dp = nullptr);

    // resets rectangle for free space
    void setRect(QRect);

    // Returns the rectangle area still free of text/pixmaps after
    // a number of drawText() calls.
    QRect remainingRect(DrawParams *dp = nullptr);

private:
    int _usedTopLeft, _usedTopCenter, _usedTopRight;
    int _usedBottomLeft, _usedBottomCenter, _usedBottomRight;
    QRect _rect;

    // temporary
    int _fontHeight;
    QFontMetrics *_fm;
    DrawParams *_dp;
};

#endif
