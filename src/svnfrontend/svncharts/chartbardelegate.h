/***************************************************************************
 *   Copyright (C) 2006 by Witold Wysota                                   *
 *   wysota@qtcentre.org                                                   *
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
#ifndef CHARTBARDELEGATE_H
#define CHARTBARDELEGATE_H

#include <QAbstractItemDelegate>
#include <QStyleOptionViewItem>


class ChartDelegate : public QAbstractItemDelegate {
public:
    ChartDelegate(QObject *parent=0);
    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;

protected:
    virtual void drawItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const=0;
    virtual void drawConnector(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const=0;
    QBrush createBrushFromGradient(const QGradient *g, const QRect &rect) const;
};

/**
    @author Witold Wysota <wysota@qtcentre.org>
*/
class ChartBarDelegate : public ChartDelegate {
public:
    ChartBarDelegate(QObject *parent = 0);
    ~ChartBarDelegate();
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
protected:
    void drawItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawConnector(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const{};
};

class ChartLineDelegate : public ChartDelegate {
public:
     ChartLineDelegate(QObject *parent = 0) : ChartDelegate(parent) {}
     QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
//     void drawConnector(QPainter *painter, const QStyleOptionViewItem &option, const QRect &r1, const QRect &r2) const;
protected:
    void drawItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawConnector(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class ChartPointDelegate : public ChartDelegate {
public:
    ChartPointDelegate(QObject *parent = 0) : ChartDelegate(parent) {}
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
protected:
    void drawItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawConnector(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif
