#ifndef BLAMEDISPLAY_IMPL_H
#define BLAMEDISPLAY_IMPL_H

#include "blamedisplay.h"
#include "svnqt/client.hpp"

class BlameDisplayData;

class BlameDisplay_impl:public BlameDisplay
{
    Q_OBJECT
public:
    BlameDisplay_impl(const svn::AnnotatedFile&,QWidget*parent=0,const char*name=0);
    BlameDisplay_impl(QWidget*parent=0,const char*name=0);
    virtual ~BlameDisplay_impl();

    virtual void setContent(const svn::AnnotatedFile&);

    const QColor rev2color(svn_revnum_t)const;

public slots:

private:
    BlameDisplayData*m_Data;
};

#endif
