/***************************************************************************
 *   Copyright (C) 2007 by Rajko Albrecht  ral@alwins-world.de             *
 *   http://kdesvn.alwins-world.de/                                        *
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
#ifndef PROPERTYLIST_H
#define PROPERTYLIST_H

#include <QTreeWidget>
#include <QStringList>
#include "src/svnqt/svnqttypes.h"

class SvnItem;
/**
    @author
*/
class Propertylist : public QTreeWidget
{
    Q_OBJECT
public:
    explicit Propertylist(QWidget *parent = 0);
    ~Propertylist();

    bool checkExisting(const QString &aName, QTreeWidgetItem *it = 0);
    bool commitchanges()const
    {
        return m_commitit;
    }
    void setCommitchanges(bool how)
    {
        m_commitit = how;
    }
    void addCallback(QObject *);

public slots:
    virtual void displayList(const svn::PathPropertiesMapListPtr &, bool, bool, const QString &);
    virtual void clear();

protected slots:
    virtual void slotItemChanged(QTreeWidgetItem *item, int col);

signals:
    void sigSetProperty(const svn::PropertiesMap &, const QStringList &, const QString &);
protected:
    bool m_commitit;
    QString m_current;
    bool m_Dir;

    virtual void keyPressEvent(QKeyEvent *);
};

#endif
