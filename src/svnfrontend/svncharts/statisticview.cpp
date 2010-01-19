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
}

void StatisticView::setRepository(const QString&repository)
{
    // shared pointer, delete itself so no destructor needed
    m_DbStatistic = new DbStatistic(repository);
    QStandardItemModel*_model = static_cast<QStandardItemModel*>(m_TableView->model());

    DbStatistic::Usermap _data;
    m_DbStatistic->getUserCommits(_data);
    QList<QStandardItem*> iList;
    _model->appendColumn(iList);

    QList<unsigned> values=_data.values();
    QList<QString> keys = _data.keys();

    QColor a(160,160,160);
    int offset = 10;
    int r=0; int g=0;int b=0;
    uint colinc=0;
    QColor _bgColor = KColorScheme(QPalette::Active, KColorScheme::Selection).background().color();

    for (int i=0; i<values.size();++i) {
        _model->appendRow(new QStandardItem(values[i]));
        _model->setData(_model->index(i,0),values[i]);
        if (m_ColumnView->maximumValue()<(int)values[i]) {
            m_ColumnView->setMaximumValue( qRound((double)values[i]/50.0+0.5)*50 );
        }
        _model->setHeaderData(i, Qt::Vertical,keys[i]);

        a.setRgb(a.red()+offset,a.green()+offset,a.blue()+offset);
        _model->setData(_model->index(i,0), a, Qt::DecorationRole);
        if ( a.red()>245||a.green()>245||a.blue()>245 ) {
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
            a.setRgb(160+r,160+g,160+b);
        }

    }
}

StatisticView::~StatisticView()
{
}

#include "statisticview.moc"