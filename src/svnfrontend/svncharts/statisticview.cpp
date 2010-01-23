/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
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

#include "statisticview.h"
#include "dbstatistic.h"

#include <kcolorscheme.h>

#include <QStandardItemModel>

#include <cmath>

StatisticView::StatisticView(QWidget*parent)
:QWidget(parent),m_DbStatistic(0)
{
    init();
}

StatisticView::StatisticView(const QString&repository,QWidget*parent)
    :QWidget(parent),m_DbStatistic(0)
{
    init();
    setRepository(repository);
}

void StatisticView::init()
{
    setupUi(this);
    QStandardItemModel*_model = new QStandardItemModel;
    m_TableView->setModel(_model);
    m_ColumnView->setModel(m_TableView->model());
    QItemSelectionModel *sm = new QItemSelectionModel(_model);
    m_TableView->setSelectionModel(sm);
    m_ColumnView->setSelectionModel(sm);
    m_ColumnView->setCanvasMargins(QSize(50,50));
    m_ColumnView->setMinimumBarWidth(60);
    m_ColumnView->setFlags(ColumnChartView::ChartTitle|ColumnChartView::XTitle|ColumnChartView::YTitle|ColumnChartView::VerticalScale);
}

void StatisticView::setRepository(const QString&repository)
{
    // shared pointer, delete itself so no destructor needed
    m_DbStatistic = new DbStatistic(repository);
    QStandardItemModel*_model = static_cast<QStandardItemModel*>(m_TableView->model());
    _model->clear();

    DbStatistic::Usermap _data;
    if (!m_DbStatistic->getUserCommits(_data)) {
        m_ColumnView->setChartTitle(i18n("Could not get statistic data"));
        return;
    }

    QList<unsigned> values=_data.values();
    QList<QString> keys = _data.keys();

    static int offset = 30;
    static int colbegin = 50;
    QColor a(colbegin,colbegin,colbegin);
    int r=0; int g=0;int b=0;
    uint colinc=0;
    static int maxc=255-offset;
    QColor _bgColor = KColorScheme(QPalette::Active, KColorScheme::Selection).background().color();
    m_ColumnView->setYTitle(i18n("Commits"));
    m_ColumnView->setXTitle(i18n("Author"));

    _model->setColumnCount(1);
    _model->setRowCount(values.size());
    
    _model->setHeaderData(0,Qt::Horizontal,i18n("Commits"));

    unsigned all = 0;
    QColor _block = KColorScheme(QPalette::Active, KColorScheme::Selection).background().color();

    for (int i=0; i<values.size();++i) {
        _model->setData(_model->index(i,0),values[i]);
        all+=values[i];
        if (m_ColumnView->maximumValue()<(int)values[i]) {
            m_ColumnView->setMaximumValue( qRound((double)values[i]/50.0+0.5)*50 );
        }
        if (keys[i].isEmpty()) {
            _model->setHeaderData(i, Qt::Vertical,i18n("No author"),Qt::DisplayRole);
            QFont f = _model->headerData(i,Qt::Vertical,Qt::FontRole).value<QFont>();
            f.setItalic(true);
            _model->setHeaderData(i,Qt::Vertical,f,Qt::FontRole);
        } else {
            _model->setHeaderData(i, Qt::Vertical,keys[i],Qt::DisplayRole);
        }
        

        a.setRgb(a.red()+offset,a.green()+offset,a.blue()+offset);
        _model->setData(_model->index(i,0), a, Qt::DecorationRole);
        if ( a.red()>maxc||a.green()>maxc||a.blue()>maxc ) {
            if (colinc==0) {
                ++colinc;
            } else if (r>=50||g>=50||b>=50) {
                if (++colinc>6) {
                    colinc = 0;
                    r=g=b=0;
                } else {
                    r=g=b=-10;
                }
            }
            if (colinc & 0x1) {
                r+=10;
            }
            if (colinc & 0x2) {
                g+=10;
            }
            if (colinc & 0x4) {
                b+=10;
            }
            a.setRgb(colbegin+r,colbegin+g,colbegin+b);
        }

    }
    m_ColumnView->setChartTitle(i18n("%1 commits overall",all));
}

StatisticView::~StatisticView()
{
}

#include "statisticview.moc"