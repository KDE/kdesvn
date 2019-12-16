/***************************************************************************
 *   Copyright (C) 2007 by Rajko Albrecht  ral@alwins-world.de             *
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
#pragma once

#include <QTreeWidget>
#include <QStringList>
#include "svnqt/svnqttypes.h"

class SvnItem;
/**
    @author
*/
class Propertylist : public QTreeWidget
{
    Q_OBJECT
public:
    explicit Propertylist(QWidget *parent = nullptr);
    ~Propertylist();

    bool checkExisting(const QString &aName, QTreeWidgetItem *it = nullptr);
    bool commitchanges() const
    {
        return m_commitit;
    }
    void setCommitchanges(bool how)
    {
        m_commitit = how;
    }

public Q_SLOTS:
    void displayList(const svn::PathPropertiesMapListPtr &, bool, bool, const QString &);
    void clear();

protected Q_SLOTS:
    void init();
    void slotItemChanged(QTreeWidgetItem *item, int col);

Q_SIGNALS:
    void sigSetProperty(const svn::PropertiesMap &, const QStringList &, const QString &);
protected:
    bool m_commitit;
    QString m_current;
    bool m_Dir;
};
