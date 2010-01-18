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
#include "columnchartview.h"
#include "chartbardelegate.h"
#include <QPainter>
#include <QScrollBar>
#include <QAbstractItemModel>
#include <QPainter>
#include <QHelpEvent>
#include <QToolTip>
#include <QWhatsThis>



ColumnChartView::ColumnChartView(QWidget *parent) : QAbstractItemView(parent) {

    _minVal = 0;
    _maxVal = 100;

    _canvasHMargin = 30;
    _canvasVMargin = 30;

    _vertGridLines = 5;
    _minorVertGridLines = 0;
    _legendwidth = 100;
    setItemDelegate(new ChartBarDelegate(this));
    updateGeometries();

    _flags = 0 | VerticalGrid | VerticalScale | ChartTitle;
    connect(this, SIGNAL(xTitleChanged()), viewport(), SLOT(update()));
    connect(this, SIGNAL(yTitleChanged()), viewport(), SLOT(update()));
    connect(this, SIGNAL(chartTitleChanged()), viewport(), SLOT(update()));

}

ColumnChartView::~ColumnChartView() {}

void ColumnChartView::scrollTo( const QModelIndex & /*index*/, ScrollHint /*hint*/ ) {
    return;
}

QModelIndex ColumnChartView::indexAt( const QPoint & point ) const {
    if(model()->rowCount(rootIndex())==0)
        return QModelIndex();

    int wx = point.x() + horizontalScrollBar()->value();
    int wy = point.y() + verticalScrollBar()->value();
    // qDebug("indexAt: %d, %d", wx, wy);
    //     if( wx<(int)_canvasHMargin
    //             || wy<(int)_canvasVMargin
    //             || wx>viewport()->width()-(int)_canvasHMargin
    //             || wy>viewport()->height()-(int)_canvasVMargin
    //       )
    //         return QModelIndex();
    uint items = model()->rowCount(rootIndex());
    uint jtems = model()->columnCount(rootIndex());
    for(uint ind = 0; ind < items; ind++) {
        for(uint jnd = 0; jnd < jtems; jnd++) {
            QModelIndex i = model()->index(ind, jnd);
            QRect r = visualRect(i);
            if(r.contains(QPoint(wx,wy))) {
                return i;
            }
        }
    }
    return QModelIndex();
}

QRect ColumnChartView::visualRect( const QModelIndex & index ) const {
//qDebug(qPrintable(QString::number(__LINE__)));
    if(!index.isValid())
        return QRect();
    int itemwidth = itemWidth();
    int seriesCount = index.model()->columnCount();
    int columnWidth = itemWidth()/seriesCount;
    int maxh = chartSize().height();
    int valueSpan = _maxVal - _minVal;
    int itemValue = model()->data(index).toInt();
    if(itemValue >_maxVal)
        itemValue = _maxVal;
    int l = maxh+4+chartTitleSize().height();
    int h = qRound(1.0*maxh*(itemValue-_minVal)/valueSpan);

    QRect itemRect(
        qRound(4+yTitleSize().height()+4+valueSize().width()+itemwidth/4+index.row()*itemwidth*3/2 +1.1*columnWidth*index.column()),
        l-h,
        4*columnWidth/5,
        h
    );
    return itemRect;
}

void ColumnChartView::paintEvent( QPaintEvent * ) {
//    qDebug("PAINT EVENT");
    QPainter painter(viewport());
    painter.save();
    painter.translate(- horizontalScrollBar()->value(), - verticalScrollBar()->value());
#ifdef DEBUG

    painter.fillRect(4+yTitleSize().height()+4+valueSize().width(), 4+chartTitleSize().height(), chartSize().width(), chartSize().height(), QColor(220,220,220));
#endif

    if(_flags & VerticalGrid)
        drawGridLines(&painter);
    drawAxis(&painter);
    if(_flags & Legend)
        drawLegend(&painter);
    if(_flags & VerticalScale)
        drawVerticalScale(&painter);
    if(_flags & ChartTitle)
        drawChartTitle(&painter);
    if(_flags & XTitle)
        drawXTitle(&painter);
    if(_flags & YTitle)
        drawYTitle(&painter);
    drawRowNames(&painter);
    painter.restore();
    if(!model())
        return;
    drawCanvas(&painter);
}

void ColumnChartView::drawCanvas( QPainter * p ) {

    int items = model()->rowCount(rootIndex());
    int jtems = model()->columnCount(rootIndex());

    //     ChartBarDelegate *chartdelegate = dynamic_cast<ChartBarDelegate*>(itemDelegate());
    //     if(chartdelegate) {
    //         for(int i=1;i<items;i++) {
    //             for(int j=1;j<jtems;j++) {
    //                 QModelIndex index = model()->index(i,j);
    //                 QPen pen = painter.pen();
    //                 QPen pennew = QPen(QColor(model()->data(index, Qt::DecorationRole).toString()));
    //                 pennew.setWidth(3);
    //                 bool aa = painter.renderHints() & QPainter::Antialiasing;
    //                 painter.setRenderHint(QPainter::Antialiasing, true);
    //                 painter.setPen(pennew);
    //                 QStyleOptionViewItem option = viewOptions();
    //                 chartdelegate->drawConnector(&painter, option, visualRect(index), visualRect(model()->index(index.row()-1, index.column(), index.parent())));
    //                 painter.setRenderHint(QPainter::Antialiasing, aa);
    //                 painter.setPen(pen);
    //             }
    //         }
    //     }
    //
    for(int i=0;i<items;i++) {
        for(int j=jtems-1;j>=0;j--) {
            paintItem(p, model()->index(i,j, rootIndex()));
        }
    }


}

bool ColumnChartView::isIndexHidden( const QModelIndex & /*index*/ ) const {
    return true;
}

int ColumnChartView::horizontalOffset( ) const {
    return horizontalScrollBar()->value();
}

QModelIndex ColumnChartView::moveCursor( CursorAction cursorAction, Qt::KeyboardModifiers /*modifiers*/ ) {
    QModelIndex curr = currentIndex();
    switch (cursorAction) {
    case MoveUp:
        if(curr.row()>0)
            curr = model()->index(curr.row()-1, curr.column(), curr.parent());
        break;
    case MovePrevious:
        if(curr.column()<1 && curr.row()>0) {
            curr = model()->index(curr.row()-1, model()->columnCount()-1, curr.parent());
            break;
        }
    case MoveLeft:
        if(curr.column()>0)
            curr = model()->index(curr.row(), curr.column()-1, curr.parent());
        break;
    case MoveDown:
        if(curr.row()<model()->rowCount(rootIndex())-1)
            curr = model()->index(curr.row()+1, curr.column(), curr.parent());
        break;
    case MoveNext:
        if(curr.column()==model()->columnCount(rootIndex())-1 && curr.row()<model()->rowCount(rootIndex())-1) {
            curr = model()->index(curr.row()+1, 1, curr.parent());
            break;
        }
    case MoveRight:
        if(curr.column()<model()->columnCount(rootIndex())-1)
            curr = model()->index(curr.row(), curr.column()+1, curr.parent());
        break;
    default:
        return curr;
    }
    viewport()->update();
    return curr;
}

void ColumnChartView::setSelection( const QRect & rect, QItemSelectionModel::SelectionFlags flags ) {
//qDebug(qPrintable(QString::number(__LINE__)));
    QModelIndexList indexes;
    QRect contentsRect = rect.translated(horizontalScrollBar()->value(),
                                         verticalScrollBar()->value());
    int rows = model()->rowCount(rootIndex());
    int columns = model()->columnCount(rootIndex());
    QRect adj = contentsRect.adjusted(-1,-1,1,1);
    for (int row = 0; row < rows; ++row)
        for(int column = 0; column < columns; ++column) {
            QModelIndex index = model()->index(row, column, rootIndex());
            QRect r = visualRect(index);
            r.translate(horizontalScrollBar()->value(), verticalScrollBar()->value());
            if (adj.intersects(r)) {
                indexes.append(index);
            }
        }
    if (!indexes.empty() > 0) {
        int firstRow = indexes[0].row();
        int lastRow = indexes[0].row();
        int firstColumn = indexes[0].column();
        int lastColumn = indexes[0].column();
        for (int i = 1; i < indexes.size(); ++i) {
            firstRow = qMin(firstRow, indexes[i].row());
            lastRow = qMax(lastRow, indexes[i].row());
            firstColumn = qMin(firstColumn, indexes[i].column());
            lastColumn = qMax(lastColumn, indexes[i].column());
        }
        QItemSelection selection(
            model()->index(firstRow, firstColumn, rootIndex()),
            model()->index(lastRow, lastColumn, rootIndex()));
        selectionModel()->select(selection, flags);
        viewport()->update();
    } else {
        QModelIndex noIndex;
        QItemSelection selection(noIndex, noIndex);
        selectionModel()->select(selection, flags);
    }
    selectionRect = rect;
}

QRegion ColumnChartView::visualRegionForSelection( const QItemSelection & selection ) const {
//qDebug(qPrintable(QString::number(__LINE__)));
    int ranges = selection.count();

    if (ranges == 0)
        return QRect();

    int firstRow = selection.at(0).top();
    int lastRow = selection.at(0).top();

    for (int i = 0; i < ranges; ++i) {
        firstRow = qMin(firstRow, selection.at(i).top());
        lastRow = qMax(lastRow, selection.at(i).bottom());
    }

    QModelIndex firstItem = model()->index(qMin(firstRow, lastRow), 0, rootIndex());
    QModelIndex lastItem = model()->index(qMax(firstRow, lastRow), 0, rootIndex());

    QRect firstRect = visualRect(firstItem);
    QRect lastRect = visualRect(lastItem);

    return firstRect.unite(lastRect);

}

int ColumnChartView::verticalOffset( ) const {
    return verticalScrollBar()->value();
}

void ColumnChartView::mouseReleaseEvent( QMouseEvent * event ) {
    QAbstractItemView::mouseReleaseEvent(event);
    selectionRect = QRect();
    viewport()->update();
}

void ColumnChartView::paintItem( QPainter * p, const QModelIndex & index ) {
    QItemSelectionModel *selections = selectionModel();
    QStyleOptionViewItem option = viewOptions();
    option.rect = visualRect(index).translated(-horizontalScrollBar()->value(), -verticalScrollBar()->value());
    if(currentIndex() == index) {
        option.state |= QStyle::State_HasFocus;
    }
    if(selections->isSelected(index)) {
        option.state |= QStyle::State_Selected;
    }
    //QAbstractItemDelegate *delegate = itemDelegate();
    ChartDelegate *delegate = dynamic_cast<ChartDelegate*>(itemDelegate());
    delegate->paint(p, option, index);


    if (!selectionRect.isEmpty()) {
        QStyleOptionRubberBand band;
        band.shape = QRubberBand::Rectangle;
        band.rect = selectionRect;
        p->save();
        style()->drawControl(QStyle::CE_RubberBand, &band, p);
        p->restore();
    }

}

int ColumnChartView::itemWidth( ) const {
    int maxw = qMax((int)(viewport()->width()-horizontalMargins()), chartSize().width());
    return 2*(maxw/model()->rowCount(rootIndex()))/3;
}

bool ColumnChartView::event( QEvent * event ) {
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        QModelIndex index = indexAt(helpEvent->pos());
        if (index.isValid()) {
            const QVariant &tip = model()->data(index, Qt::ToolTipRole);
            if(!tip.isNull()) {
                QToolTip::showText(helpEvent->globalPos(), tip.toString());
                return true;
            }
        }
    } else if(event->type() == QEvent::WhatsThis) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        QModelIndex index = indexAt(helpEvent->pos());
        if (index.isValid()) {
            const QVariant &tip = model()->data(index, Qt::WhatsThisRole);
            if(!tip.isNull()) {
                QWhatsThis::showText(helpEvent->globalPos(), tip.toString(), this);
                return true;
            }
        }
    }
    return QAbstractItemView::event(event);
}

void ColumnChartView::resizeEvent( QResizeEvent *e ) {
//qDebug(qPrintable(QString::number(__LINE__)));
    QAbstractItemView::resizeEvent(e);
    updateGeometries();
}

void ColumnChartView::updateGeometries( ) {
//qDebug(qPrintable(QString::number(__LINE__)));
    if(model()) {
        int minimumWidth = model()->columnCount()*model()->rowCount()*7;
        int minimumHeight = _vertGridLines*(1+font().pointSize());
        setChartSize(QSize(minimumWidth, minimumHeight));
    } else
        setChartSize(viewport()->size());
    int viswid = yTitleSize().height()+4+_legendwidth+8+valueSize().width();
    int vishei = xTitleSize().height()+chartTitleSize().height()+8+rowNameSize().height();
    setChartSize(QSize(qMax(chartSize().width(), viewport()->width()-viswid),
                       qMax(chartSize().height(), viewport()->height()-vishei)));
    horizontalScrollBar()->setPageStep(viewport()->width());
    horizontalScrollBar()->setRange(0, qMax(0, chartSize().width()+viswid - viewport()->width()));
    verticalScrollBar()->setPageStep(viewport()->height());
    verticalScrollBar()->setRange(0, qMax(0, chartSize().height()+vishei - viewport()->height()));
}

void ColumnChartView::scrollContentsBy( int dx, int dy ) {
    viewport()->scroll(dx, dy);
}

void ColumnChartView::drawGridLines( QPainter * painter ) {
//qDebug(qPrintable(QString::number(__LINE__)));
    int maxw = chartSize().width();
    int maxh = chartSize().height();
    painter->save();
    painter->translate(4+yTitleSize().height()+4+valueSize().width(), 4+chartTitleSize().height()+maxh);

    QStyleOptionViewItem option = viewOptions();
    const int gridHint = style()->styleHint(QStyle::SH_Table_GridLineColor, &option, this);
    const QColor gridColor = static_cast<QRgb>(gridHint);
    QPen p(gridColor);
    QPen minorP(p);
    minorP.setStyle(Qt::DotLine);
    //QPen nrcolor = palette().color(QPalette::WindowText);
    int lineh;

    float majorDist = 1.0*maxh/_vertGridLines;
    float minorDist = majorDist / (_minorVertGridLines+1);
    for(uint i=1;i<=_vertGridLines;i++) {
        painter->setPen(p);
        lineh  = qRound(i*majorDist);
        painter->drawLine(0, -lineh, maxw, -lineh);
        painter->setPen(minorP);
        for(uint j=1;j<=_minorVertGridLines;j++) {
            painter->drawLine(0, -lineh+qRound(j*minorDist), maxw, -lineh+qRound(j*minorDist));
        }
    }
    painter->restore();
}

void ColumnChartView::drawAxis( QPainter * painter ) {
//qDebug(qPrintable(QString::number(__LINE__)));
    int maxw = chartSize().width();
    int maxh = chartSize().height();
    painter->save();
    painter->translate(4+yTitleSize().height()+4+valueSize().width(), 4+chartTitleSize().height()+maxh);
    const QPalette &pal = palette();
    painter->setPen(pal.color(QPalette::WindowText));
    painter->drawLine(0,0, maxw, 0);
    painter->drawLine(0,0, 0, -maxh);
    painter->restore();
}

uint ColumnChartView::horizontalMargins( ) const {
//qDebug(qPrintable(QString::number(__LINE__)));
    return 2*_canvasHMargin + (_flags & Legend ? _legendwidth : 0);
}

void ColumnChartView::drawLegend( QPainter * painter ) {
//qDebug(qPrintable(QString::number(__LINE__)));
    painter->save();
    QFontMetrics fm(font());
    painter->translate(4+yTitleSize().height()+4+valueSize().width()+chartSize().width()+2, 4+chartTitleSize().height());
#ifdef DEBUG

    painter->save();
    QPen pn(Qt::black);
    pn.setStyle(Qt::DashLine);
    painter->setPen(pn);
    painter->drawRect(QRect(0,0, _legendwidth+3, chartSize().height()));
    painter->restore();
#endif

    painter->drawRect(0, 0, _legendwidth, 6+(model()->columnCount(rootIndex()))*(font().pointSize()+4));
    if(_legend.flags() & ChartLegend::Shadow) {
        painter->save();
        QPen pn = painter->pen();
        pn.setWidth(3);
        pn.setColor(palette().color(QPalette::Shadow));
        painter->setPen(pn);
        int dl = (model()->columnCount(rootIndex()))*(font().pointSize()+4);
        painter->drawLine(5,8+dl, _legendwidth, 8+dl);
        painter->drawLine(2+_legendwidth, 5, 2+_legendwidth, 8+dl);
        painter->restore();
    }
    for(int i=0;i<model()->columnCount();i++) {
        painter->setBrush(qvariant_cast<QColor>(model()->headerData(i, Qt::Horizontal, Qt::DecorationRole)));
        painter->drawRect(5, 6+i*(font().pointSize()+4), font().pointSize(), font().pointSize());
        painter->drawText(5+font().pointSize()+4, font().pointSize()+6+i*(font().pointSize()+4), model()->headerData(i, Qt::Horizontal).toString());
    }
    painter->restore();
}

void ColumnChartView::drawRowNames( QPainter * p ) {
//qDebug(qPrintable(QString::number(__LINE__)));
    int itemwidth = itemWidth();
    int seriesCount = model()->columnCount();
    //    int columnWidth = itemWidth()/seriesCount;
    int maxh = chartSize().height();
    //    int valueSpan = _maxVal - _minVal;
    //QStyleOptionViewItem opt = viewOptions();
    //opt.displayAlignment |= Qt::AlignHCenter;
    uint rowHeight = rowNameSize().height();//(_flags & XTitle) ? xTitleSize().height()+4-QFontMetrics(font()).height()-2 : 4;
    p->save();
    for(int i=0;i<model()->rowCount(rootIndex());i++) {
        QRect rect(qRound(4+yTitleSize().height()+4+valueSize().width()+itemwidth/4+i*itemwidth*3/2), maxh+4+chartTitleSize().height()+2, itemwidth, rowHeight);
        QStyleOptionViewItem opt = getOptionsForRowLabel(i);
        p->setFont(opt.font);
        p->setPen(opt.palette.color(QPalette::Normal, QPalette::Text));
        p->drawText(rect,opt.displayAlignment|Qt::TextWordWrap,QAbstractItemDelegate::elidedText(fontMetrics(), rect.width(),Qt::ElideRight,model()->headerData(i, Qt::Vertical).toString()));
    }
    p->restore();
}

QStyleOptionViewItem ColumnChartView::getOptionsForRowLabel( int row ) {
//qDebug(qPrintable(QString::number(__LINE__)));
    QStyleOptionViewItem opt = viewOptions();
    opt.displayAlignment |= Qt::AlignHCenter;
    opt.displayAlignment &= ~Qt::AlignVCenter;
    opt.displayAlignment |= Qt::AlignTop;
    QVariant value;
    value = model()->headerData(row, Qt::Vertical, Qt::FontRole);
    if(value.isValid())
        opt.font = qvariant_cast<QFont>(value);
    value = model()->headerData(row, Qt::Vertical, Qt::TextColorRole);
    if(value.isValid() && qvariant_cast<QColor>(value).isValid())
        opt.palette.setColor(QPalette::Text, qvariant_cast<QColor>(value));
    return opt;
}

void ColumnChartView::dataChanged( const QModelIndex & topLeft, const QModelIndex & bottomRight ) {
//qDebug(qPrintable(QString::number(__LINE__)));
    QAbstractItemView::dataChanged(topLeft, bottomRight);
    updateGeometries();
    viewport()->update();
}

void ColumnChartView::drawVerticalScale( QPainter * p ) {
//qDebug(qPrintable(QString::number(__LINE__)));
    //    int maxw = chartSize().width();
    int maxh = chartSize().height();
    p->save();
    p->translate(4+yTitleSize().height(), 4+chartTitleSize().height()+maxh);
    QStyleOptionViewItem option = viewOptions();
    QPen nrcolor = palette().color(QPalette::WindowText);
    int lineh;
#ifdef DEBUG

    p->save();
    QPen pn(QColor(50,50,50));
    pn.setStyle(Qt::DashLine);
    p->setPen(pn);
    p->drawRect(QRect(0, 0, valueSize().width(), -maxh));
    p->restore();
#endif

    p->setPen(nrcolor);
    for(uint i=0;i<=_vertGridLines;i++) {
        lineh  = qRound(i*(1.0*maxh/_vertGridLines));
        //QRect textRect(-4-yTitleSize().height(), -lineh-10, 4+yTitleSize().height()-5, 20);
        QRect textRect(0, -lineh-10, valueSize().width(), QFontMetrics(font()).height());
        p->drawText(textRect, Qt::AlignRight|Qt::AlignTop, QString::number( _minVal+i*(_maxVal-_minVal)/_vertGridLines ));
    }
    p->restore();
}

void ColumnChartView::drawChartTitle( QPainter * p ) {
//qDebug(qPrintable(QString::number(__LINE__)));
    QVariant v;
    QFont f = p->font();
    p->save();
    //     p->setRenderHint(QPainter::TextAntialiasing, true);
    v = _chartTitle[Qt::FontRole];
    if(v.isValid()) {
        f = qvariant_cast<QFont>(v);
        p->setFont(f);
    }
    v = chartTitle(Qt::TextColorRole);
    if(v.isValid()) {
        QColor c = qvariant_cast<QColor>(v);
        if(c.isValid()) {
            QPen pe = p->pen();
            pe.setColor(c);
            p->setPen(pe);
        }
    }
    v = chartTitle(Qt::TextAlignmentRole);
    Qt::Alignment align = Qt::AlignCenter;
    if(v.isValid()) {
        align = (Qt::Alignment)v.toInt();
    }
    QRect titleRect(2, 2, chartTitleSize().width(), chartTitleSize().height());
#ifdef DEBUG

    p->save();
    QPen pn(Qt::black);
    pn.setStyle(Qt::DashLine);
    p->setPen(pn);
    p->drawRect(titleRect);
    p->restore();
#endif

    p->drawText(titleRect,
                align,
                itemDelegate()->elidedText(QFontMetrics(f), chartTitleSize().width(), Qt::ElideRight,_chartTitle[Qt::DisplayRole].toString())
               );
    p->restore();
}

void ColumnChartView::drawXTitle( QPainter * p ) {
//qDebug(qPrintable(QString::number(__LINE__)));
    QVariant v;
    QFont f = p->font();
    p->save();
    //     p->setRenderHint(QPainter::TextAntialiasing, true);
    v = xTitle(Qt::FontRole);
    if(v.isValid()) {
        f = qvariant_cast<QFont>(v);
        p->setFont(f);
    }
    v = xTitle(Qt::TextColorRole);
    if(v.isValid()) {
        QColor c = qvariant_cast<QColor>(v);
        if(c.isValid()) {
            QPen pe = p->pen();
            pe.setColor(c);
            p->setPen(pe);
        }
    }
    v = xTitle(Qt::TextAlignmentRole);
    Qt::Alignment align = Qt::AlignCenter;
    if(v.isValid()) {
        align = (Qt::Alignment)v.toInt();
    }
    QRect xRect(4+yTitleSize().height()+4+valueSize().width(), 4+ chartTitleSize().height() + chartSize().height() +2 + rowNameSize().height(), chartSize().width(), + xTitleSize().height());
#ifdef DEBUG

    p->save();
    QPen pn(Qt::black);
    pn.setStyle(Qt::DashLine);
    p->setPen(pn);
    p->drawRect(xRect);
    p->restore();
#endif

    p->drawText(xRect,
                align,
                itemDelegate()->elidedText(QFontMetrics(f), xRect.width(), Qt::ElideRight,xTitle(Qt::DisplayRole).toString())
               );
    p->restore();
}

void ColumnChartView::drawYTitle( QPainter *p  ) {
//qDebug(qPrintable(QString::number(__LINE__)));
    QVariant v;
    QFont f = p->font();
    p->save();
    //     p->setRenderHint(QPainter::TextAntialiasing, true);
    v = yTitle(Qt::FontRole);
    if(v.isValid()) {
        f = qvariant_cast<QFont>(v);
        p->setFont(f);
    }
    v = yTitle(Qt::TextColorRole);
    if(v.isValid()) {
        QColor c = qvariant_cast<QColor>(v);
        if(c.isValid()) {
            QPen pe = p->pen();
            pe.setColor(c);
            p->setPen(pe);
        }
    }
    v = yTitle(Qt::TextAlignmentRole);
    Qt::Alignment align = Qt::AlignCenter;
    if(v.isValid()) {
        align = (Qt::Alignment)v.toInt();
    }
    QRect yRect(-chartTitleSize().height()-4, 1, -chartSize().height(), QFontMetrics(f).height());
    //p->save();
    p->rotate(-90);
    // p->scale(-1,0);
#ifdef DEBUG

    p->save();
    QPen pn(Qt::black);
    pn.setStyle(Qt::DashLine);
    p->setPen(pn);
    p->drawRect(yRect);
    p->restore();
#endif

    p->drawText(yRect,
                align,
                itemDelegate()->elidedText(QFontMetrics(f), chartSize().height(), Qt::ElideRight, yTitle(Qt::DisplayRole).toString())
               );
    p->restore();
}

QVariant ColumnChartView::yTitle( int role ) const {
    return _yTitle[role];
}

void ColumnChartView::setYTitle( const QVariant & theValue, int role ) {
//qDebug(qPrintable(QString::number(__LINE__)));
    if(theValue!=_yTitle[role]) {
        _yTitle[role] = theValue;
        if(_flags & YTitle)
            emit yTitleChanged();
    }
}

QVariant ColumnChartView::xTitle( int role ) const {
    return _xTitle[role];
}

void ColumnChartView::setXTitle( const QVariant & theValue, int role ) {
//qDebug(qPrintable(QString::number(__LINE__)));
    if(theValue!=_xTitle[role]) {
        _xTitle[role] = theValue;
        if(_flags & XTitle)
            emit xTitleChanged();
    }
}

QVariant ColumnChartView::chartTitle( int role ) const {
    return _chartTitle[role];
}

void ColumnChartView::setChartTitle( const QVariant & theValue, int role ) {
//qDebug(qPrintable(QString::number(__LINE__)));
    if(theValue!=_chartTitle[role]) {
        _chartTitle[role] = theValue;
        if(_flags & ChartTitle)
            emit chartTitleChanged();
    }
}

uint ColumnChartView::minorVerticalGridLines( ) const {
    return _minorVertGridLines;
}

void ColumnChartView::setMinorVerticalGridLines( const uint & theValue ) {
//qDebug(qPrintable(QString::number(__LINE__)));
    _minorVertGridLines = theValue;
    if(_flags & VerticalGrid)
        viewport()->update();
}

uint ColumnChartView::verticalGridLines( ) const {
    return _vertGridLines;
}

void ColumnChartView::setVerticalGridLines( const uint & theValue ) {
//qDebug(qPrintable(QString::number(__LINE__)));
    _vertGridLines = theValue;
    if(_flags & VerticalGrid)
        viewport()->update();
}

void ColumnChartView::setMaximumValue( const int & theValue ) {
//qDebug(qPrintable(QString::number(__LINE__)));
    _maxVal = theValue;
    viewport()->update();
}

void ColumnChartView::setMinimumValue( const int & theValue ) {
//qDebug(qPrintable(QString::number(__LINE__)));
    _minVal = theValue;
    viewport()->update();
}

int ColumnChartView::minimumValue( ) const {
    return _minVal;
}

QSize ColumnChartView::canvasMargins( ) {
    return QSize(_canvasHMargin, _canvasVMargin);
}

void ColumnChartView::setCanvasMargins( QSize s ) {
    _canvasHMargin = s.width();
    _canvasVMargin = s.height();
}

int ColumnChartView::maximumValue( ) const {
    return _maxVal;
}

void ColumnChartView::setChartSize( const QSize & theValue ) {
    _chartSize = theValue;
}

QSize ColumnChartView::chartSize( ) const {
    return _chartSize;
}

void ColumnChartView::setFlags( const quint32 & theValue ) {
//qDebug(qPrintable(QString::number(__LINE__)));
    if(theValue!=_flags) {
        _flags = theValue;
        updateGeometries();
        viewport()->update();
    }
}

quint32 ColumnChartView::flags( ) const {
    return _flags;
}

QSize ColumnChartView::chartTitleSize( ) const {
//qDebug(qPrintable(QString::number(__LINE__)));
    QSize s;
    if(!(_flags & ChartTitle) || chartTitle().isNull())
        return QSize(chartSize().width()+yTitleSize().height()+4+_legendwidth+4+valueSize().width(), 0);
    QVariant v = chartTitle(Qt::SizeHintRole);
    if(v.isValid()) {
        s= qvariant_cast<QSize>(v);
        s.setWidth(chartSize().width());
        return s;
    }
    s.setWidth(chartSize().width()+yTitleSize().height()+4+_legendwidth+4+valueSize().width());

    v = chartTitle(Qt::FontRole);
    if(v.isValid()) {
        QFont f = qvariant_cast<QFont>(v);
        s.setHeight(QFontMetrics(f).height());
    } else {
        s.setHeight(_canvasVMargin);
    }
    return s;
}

QSize ColumnChartView::xTitleSize( ) const {
//qDebug("QSize ColumnChartView::xTitleSize( ) const");
    QSize s;
    if(!(_flags & XTitle) || xTitle().isNull())
        return QSize(chartSize().width(), 0);
    QVariant v = xTitle(Qt::SizeHintRole);
    if(v.isValid()) {
        s= qvariant_cast<QSize>(v);
        s.setWidth(chartSize().width());
        return s;
    }
    s.setWidth(chartSize().width());
    v = xTitle(Qt::FontRole);
    if(v.isValid()) {
        QFont f = qvariant_cast<QFont>(v);
        s.setHeight(QFontMetrics(f).height());
    } else {
        s.setHeight(_canvasVMargin);
    }
    return s;
}

QSize ColumnChartView::yTitleSize( ) const {
//    qDebug("ColumnChartView::yTitleSize( ) const");
    QSize s;
    if(!(_flags & YTitle) || yTitle().isNull())
        return QSize(chartSize().width(), 0);
    QVariant v = yTitle(Qt::SizeHintRole);
    if(v.isValid()) {
        s= qvariant_cast<QSize>(v);
        s.setWidth(chartSize().width());
        return s;
    }
    s.setWidth(chartSize().width());
    v = yTitle(Qt::FontRole);
    if(v.isValid()) {
        QFont f = qvariant_cast<QFont>(v);
        s.setHeight(QFontMetrics(f).height());
    } else {
        s.setHeight(_canvasVMargin);
    }
    return s;
}

QSize ColumnChartView::rowNameSize( ) const {
//    qDebug("QSize ColumnChartView::rowNameSize( ) const");
    return QSize(0, QFontMetrics(font()).height());
}

QSize ColumnChartView::valueSize( ) const {
//    qDebug("QSize ColumnChartView::valueSize( ) const");
    int m = QFontMetrics(font()).width(QString::number(maximumValue()));
    return QSize(m+4, 0);
}
