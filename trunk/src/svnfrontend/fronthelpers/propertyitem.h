/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
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
#ifndef PROPERTYITEM_H
#define PROPERTYITEM_H

#include <QTreeWidgetItem>

class PropertiesDlg;
class Propertylist;
class QTreeWidget;

class PropertyListViewItem:public QTreeWidgetItem
{
    friend class PropertiesDlg;
    friend class Propertylist;

    public:
        static const int _RTTI_ = QTreeWidgetItem::UserType+2;
        PropertyListViewItem(QTreeWidget *parent,const QString&,const QString&);
        PropertyListViewItem(QTreeWidget *parent);
        virtual ~PropertyListViewItem();

        const QString&startName()const{return m_startName;}
        const QString&startValue()const{return m_startValue;}
        const QString&currentName()const{return m_currentName;}
        const QString&currentValue()const{return m_currentValue;}

        void checkValue();
        void checkName();
        void deleteIt();
        void unDeleteIt();
        bool deleted()const{return m_deleted;}

        bool different()const;

       //! Check if a specific property may just internale
       /*!
        * That means, a property of that may not edit,added or deleted.
        *
        * This moment it just checks for "svn:special"
        * \return true if protected property otherwise false
        */
       static bool protected_Property(const QString&);

    protected:
        QString m_currentName,m_startName,m_currentValue,m_startValue;
        bool m_deleted;
};

#endif
