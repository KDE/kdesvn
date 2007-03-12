#ifndef __WIDGET_BLOCK_STACK_H
#define __WIDGET_BLOCK_STACK_H

class QWidget;

class WidgetBlockStack
{
    QWidget*_w;
public:
    WidgetBlockStack(QWidget*);
    ~WidgetBlockStack();
};

#endif
