#include "propertyitem.h"
#include <klocale.h>
#include <kiconloader.h>

PropertyListViewItem::PropertyListViewItem(KListView *parent,const QString&aName,const QString&aValue)
    : KListViewItem(parent),m_currentName(aName),m_startName(aName),m_currentValue(aValue),m_startValue(aValue),m_deleted(false)
{
    setText(0,startName());
    setText(1,startValue());
}

PropertyListViewItem::PropertyListViewItem(KListView *parent)
    : KListViewItem(parent),m_currentName(""),m_startName(""),m_currentValue(""),m_startValue(""),m_deleted(false)
{
    setText(0,startName());
    setText(1,startValue());
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
    setPixmap(0,KGlobal::iconLoader()->loadIcon("cancel",KIcon::Desktop,16));
}

void PropertyListViewItem::unDeleteIt()
{
    m_deleted = false;
    setPixmap(0,QPixmap());
}

bool PropertyListViewItem::protected_Property(const QString&what)
{
    if (what.compare("svn:special")!=0) return false;
    return true;
}
