#ifndef _PROPERTYITEM_H
#define _PROPERTYITEM_H

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
