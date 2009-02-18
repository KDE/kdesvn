#include "propertyitem.h"
#include <klocale.h>
#include <kiconloader.h>
#include <kicon.h>
#include <kdebug.h>

PropertyListViewItem::PropertyListViewItem(QTreeWidget *parent,const QString&aName,const QString&aValue)
    : QTreeWidgetItem(parent,_RTTI_),m_currentName(aName),m_startName(aName),m_currentValue(aValue),m_startValue(aValue),m_deleted(false)
{
    setText(0,startName());
    setText(1,startValue());
}

PropertyListViewItem::PropertyListViewItem(QTreeWidget *parent)
    : QTreeWidgetItem(parent,_RTTI_),m_currentName(""),m_startName(""),m_currentValue(""),m_startValue(""),m_deleted(false)
{
}

PropertyListViewItem::~PropertyListViewItem()
{
}

void PropertyListViewItem::checkValue()
{
    m_currentValue=text(1);
}

void PropertyListViewItem::checkName()
{
    m_currentName=text(0);
}

bool PropertyListViewItem::different()const
{
    return m_currentName!=m_startName || m_currentValue!=m_startValue || deleted();
}

void PropertyListViewItem::deleteIt()
{
    m_deleted = true;
    setIcon(0,KIconLoader::global()->loadIcon("cancel",KIconLoader::Desktop,16));
}

void PropertyListViewItem::unDeleteIt()
{
    m_deleted = false;
    setIcon(0,KIcon());
}

bool PropertyListViewItem::protected_Property(const QString&what)
{
    if (
        what.compare("svn:mergeinfo")==0 ||
        what.compare("svn:special")==0
        ) {
        return true;
    }
    return false;
}
