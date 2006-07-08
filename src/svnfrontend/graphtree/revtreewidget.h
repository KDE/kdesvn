/****************************************************************************
** Form interface generated from reading ui file 'form1.ui'
**
** Created: Mo Apr 3 11:10:16 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.5   edited Aug 31 12:13 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef REVTREEWIDGET_H
#define REVTREEWIDGET_H

#include <svnqt/revision.hpp>

#include <qvariant.h>
#include <qpixmap.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class RevGraphView;
class QSplitter;
class KTextBrowser;
class CContextListener;

namespace svn {
    class LogEntry;
    class Client;
}

class RevTreeWidget : public QWidget
{
    Q_OBJECT

public:
    RevTreeWidget(QObject*,svn::Client*,QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~RevTreeWidget();

    QSplitter* m_Splitter;
    RevGraphView* m_RevGraphView;

    void setBasePath(const QString&);
    void dumpRevtree();

protected:
    QVBoxLayout* RevTreeWidgetLayout;
    KTextBrowser* m_Detailstext;

signals:
    void dispDiff(const QString&);
    void makeCat(const svn::Revision&,const QString&,const QString&,const svn::Revision&,QWidget*);

protected slots:
    virtual void languageChange();
    virtual void setDetailText(const QString&);

private:
    QPixmap image0;

};

#endif // REVTREEWIDGET_H
