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
#include "chartbardelegate.h"
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QPainter>


ChartDelegate::ChartDelegate( QObject * parent ) : QAbstractItemDelegate(parent) {}


void ChartDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const {
    /* retrieve data */
    const QAbstractItemModel *model = index.model();
    //    int itemValue = model->data(index).toInt();
    /* set brush */
    Qt::BrushStyle brushStyle = Qt::SolidPattern;
    QStyleOptionViewItem opt = option;
    if(option.state & QStyle::State_Selected) {
        brushStyle = Qt::Dense3Pattern;
    }
    if(option.state & QStyle::State_HasFocus) {
        brushStyle = Qt::Dense4Pattern;
    }
    QVariant value;

    /* font */
    value = model->data(index, Qt::FontRole);
    if(value.isValid())
        opt.font = qvariant_cast<QFont>(value);

    /* alignment */
    value = model->data(index, Qt::TextAlignmentRole);
    if(value.isValid())
        opt.displayAlignment = QFlag(value.toInt());
    else
        opt.displayAlignment |= Qt::AlignHCenter;

    /* text colour */
    value = model->data(index, Qt::TextColorRole);
    if(value.isValid() && qvariant_cast<QColor>(value).isValid())
        opt.palette.setColor(QPalette::Text, qvariant_cast<QColor>(value));

    /* decoration (fill) */
    value = model->data(index, Qt::DecorationRole);
    if(value.isValid()) {
        /* try colour */
        QColor color = qvariant_cast<QColor>(value);
        if(color.isValid()) {
            painter->setBrush(QBrush(color,brushStyle));
            /* try brush */
        } else if(value.type()==QVariant::Brush) {
            QBrush brush = qvariant_cast<QBrush>(value);
            /* try linear gradient */
            if(brush.gradient() && brush.gradient()->type()==QGradient::LinearGradient) {
                brush = createBrushFromGradient(brush.gradient(), opt.rect);
                if(brushStyle!=Qt::SolidPattern) {
                    brush.setStyle(brushStyle);

                }
            }
            painter->setBrush(brush);
        }
    }

    /* paint item */
    drawItem(painter, option, index);
    drawConnector(painter, option, index);
}

QBrush ChartDelegate::createBrushFromGradient(const QGradient *g, const QRect &rect ) const {
    //    const QGradient *g = brush.gradient();
    QLinearGradient grad(((QLinearGradient*)g)->start(), QPointF(0, rect.height()));
    grad.setStops(g->stops());
    QBrush brush = QBrush(grad);
    brush.setColor(g->stops()[0].second);
    return brush;
}



ChartBarDelegate::ChartBarDelegate(QObject *parent)
        : ChartDelegate(parent) {}


ChartBarDelegate::~ChartBarDelegate() {}


QSize ChartBarDelegate::sizeHint( const QStyleOptionViewItem &, const QModelIndex & ) const {
    return QSize();
}

void ChartBarDelegate::drawItem( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex &  ) const {
    painter->drawRect(option.rect);
}

void ChartLineDelegate::drawItem( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex &) const {
    int x = option.rect.x()+option.rect.width()/2;
    int y = option.rect.top();
    bool aa = painter->renderHints() & QPainter::Antialiasing;
    painter->setRenderHint(QPainter::Antialiasing, true);
    int r = option.rect.width()/5;
    painter->drawEllipse(x-r, y-r, 2*r+1, 2*r+1);
    painter->setRenderHint(QPainter::Antialiasing, aa);
}



void ChartLineDelegate::drawConnector( QPainter *, const QStyleOptionViewItem &, const QModelIndex & ) const {}

QSize ChartLineDelegate::sizeHint( const QStyleOptionViewItem &, const QModelIndex & ) const {
    return QSize();
}

void ChartPointDelegate::drawItem( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex &) const {
    int x = option.rect.x()+option.rect.width()/2;
    int y = option.rect.top();
    bool aa = painter->renderHints() & QPainter::Antialiasing;
    painter->setRenderHint(QPainter::Antialiasing, true);
    int r = option.rect.width()/5;
    painter->drawEllipse(x-r, y-r, 2*r+1, 2*r+1);
    painter->setRenderHint(QPainter::Antialiasing, aa);
}



void ChartPointDelegate::drawConnector( QPainter *, const QStyleOptionViewItem &, const QModelIndex & ) const {}

QSize ChartPointDelegate::sizeHint( const QStyleOptionViewItem &, const QModelIndex & ) const {
    return QSize();
}

// void ChartLineDelegate::drawConnector( QPainter * painter, const QStyleOptionViewItem & option, const QRect & r1, const QRect & r2 ) const {
//     int x1 = r1.x()+r1.width()/2;
//     int x2 = r2.x()+r2.width()/2;
//     painter->drawLine(x1, r1.top(), x2, r2.top());
// }

