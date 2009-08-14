/***************************************************************************
 *   Copyright (C) 2007 by Rajko Albrecht  ral@alwins-world.de             *
 *   http://kdesvn.alwins-world.de/                                        *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this program (in the file LGPL.txt); if not,         *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at http://kdesvn.alwins-world.de.           *
 ***************************************************************************/
#ifndef PROPERTYLIST_H
#define PROPERTYLIST_H

#include <QTreeWidget>
#include <QStringList>
#include "src/svnqt/svnqttypes.hpp"

class SvnItem;
/**
	@author
*/
class Propertylist : public QTreeWidget
{
    Q_OBJECT
public:
    Propertylist(QWidget *parent = 0, const char *name = 0);
    ~Propertylist();

    bool checkExisting(const QString&aName,QTreeWidgetItem*it=0);
    bool commitchanges()const{return m_commitit;}
    void setCommitchanges(bool how){m_commitit=how;}
    void addCallback(QObject*);

public slots:
    virtual void displayList(const svn::PathPropertiesMapListPtr&,bool,bool,const QString&);
    virtual void clear();

protected slots:
    virtual void slotItemChanged(QTreeWidgetItem*item,int col );

signals:
    void sigSetProperty(const svn::PropertiesMap&,const QStringList&,const QString&);
protected:
    bool m_commitit;
    QString m_current;
    bool m_Dir;

    virtual void keyPressEvent(QKeyEvent*);
};

#endif
