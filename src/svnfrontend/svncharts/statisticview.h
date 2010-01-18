/***************************************************************************
 *   Copyright (C) 2006-2010 by Rajko Albrecht                             *
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
#ifndef STATISTIC_VIEW_H
#define STATISTIC_VIEW_H

#include "ui_statisticview.h"
#include "svnqt/shared_pointer.h"

class DbStatistic;

class StatisticView:public QWidget,public Ui_StatisticView
{
    Q_OBJECT

public:
    StatisticView(QWidget*parent);
    StatisticView(const QString&repository,QWidget*parent);
    virtual ~StatisticView();

    void setRepository(const QString&repository);

public Q_SLOTS:

private:
    svn::SharedPointer<DbStatistic> m_DbStatistic;

    void init();
};

#endif
