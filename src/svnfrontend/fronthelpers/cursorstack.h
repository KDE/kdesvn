#ifndef __CURSOR_STACK_H
#define __CURSOR_STACK_H

#include <kapplication.h>
#include <qcursor.h>

class CursorStack
{
public:
    CursorStack(Qt::CursorShape c = Qt::WaitCursor)
    {
        KApplication::setOverrideCursor(QCursor(c));
    }
    ~CursorStack()
    {
        KApplication::restoreOverrideCursor();
    }
};

#endif
