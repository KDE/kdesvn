#include "widgetblockstack.h"

#include <qwidget.h>

WidgetBlockStack::WidgetBlockStack(QWidget*w)
{
    if ( (_w=w))
    {
        _w->setEnabled(false);
    }
}

WidgetBlockStack::~WidgetBlockStack()
{
    if (_w)
    {
        _w->setEnabled(true);
    }
}
