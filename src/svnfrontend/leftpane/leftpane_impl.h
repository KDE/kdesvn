#ifndef _LEFTPANE_IMPL
#define _LEFTPANE_IMPL

#include "ui_leftpane.h"
#include <QWidget>

class leftpane_impl: public QWidget, public Ui::leftpane
{
    Q_OBJECT
public:
    leftpane_impl(QWidget*parent=0, Qt::WFlags fl=0);
    virtual ~leftpane_impl();
public slots:
    virtual void folderSelected(const QString&);
};

#endif
