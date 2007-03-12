#ifndef BLAMEDISPLAY_IMPL_H
#define BLAMEDISPLAY_IMPL_H

#include "blamedisplay.h"
#include "svnqt/client.hpp"

class BlameDisplayData;
class SimpleLogCb;
class BlameDisplayItem;
class KListViewSearchLineWidget;

class BlameDisplay_impl:public BlameDisplay
{
    Q_OBJECT
public:
    BlameDisplay_impl(const QString&,const svn::AnnotatedFile&,QWidget*parent=0,const char*name=0);
    BlameDisplay_impl(QWidget*parent=0,const char*name=0);
    virtual ~BlameDisplay_impl();

    virtual void setContent(const QString&,const svn::AnnotatedFile&);
    virtual void setCb(SimpleLogCb*);

    const QColor rev2color(svn_revnum_t)const;

    static void displayBlame(SimpleLogCb*,const QString&,const svn::AnnotatedFile&,QWidget*parent=0,const char*name=0);

public slots:
    virtual void slotGoLine();
    virtual void slotShowCurrentCommit();

protected slots:
    virtual void slotContextMenuRequested(KListView*,QListViewItem*, const QPoint&);
    virtual void slotSelectionChanged();

protected:
    virtual void showCommit(BlameDisplayItem*);
    KListViewSearchLineWidget* m_SearchWidget;

private:
    BlameDisplayData*m_Data;
protected slots:
    virtual void slotItemDoubleClicked(QListViewItem*);
};

#endif
