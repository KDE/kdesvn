#include "leftpane.h"

#ifndef _LEFTPANE_IMPL
#define _LEFTPANE_IMPL

class leftpane_impl:public leftpane
{
    Q_OBJECT
public:
    leftpane_impl(QWidget*parent=0, WFlags fl=0);
    virtual ~leftpane_impl();
public slots:
    virtual void folderSelected(const QString&);
};

#endif
