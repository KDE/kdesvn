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

#ifndef SIMPLE_CHART_MODEL_H
#define SIMPLE_CHART_MODEL_H

#include "src/svnqt/shared_pointer.h"
#include <QSqlQueryModel>

class SimpleChartModelData;

class SimpleChartModel:public QSqlQueryModel
{
    Q_OBJECT
    public:
        SimpleChartModel(QObject*parent=0);
        virtual ~SimpleChartModel();

        virtual bool setData(const QModelIndex&,const QVariant&,int);
        virtual QVariant data(const QModelIndex &item, int role=Qt::DisplayRole) const;

    private:
        svn::SharedPointer<SimpleChartModelData> m_Data;

};
#endif
