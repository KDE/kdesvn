/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "revtreewidget.h"
#include "revgraphview.h"
#include "src/settings/kdesvnsettings.h"

#include <ktextbrowser.h>

#include <QVariant>
#include <QSplitter>
#include <QList>

#include <QLayout>
#include <QToolTip>
#include <QWhatsThis>
#include <QSizePolicy>

/*
 *  Constructs a RevTreeWidget as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
RevTreeWidget::RevTreeWidget(QObject*lt,svn::Client*cl, QWidget* parent, const char* name)
    : QWidget(parent)
{
    if ( !name )
        setObjectName("RevTreeWidget");
    else
        setObjectName(name);
    RevTreeWidgetLayout = new QVBoxLayout(this);//, 11, 6, "RevTreeWidgetLayout");

    m_Splitter = new QSplitter(Qt::Vertical,this);

    m_RevGraphView = new RevGraphView(lt,cl, m_Splitter, "m_RevGraphView");
    m_RevGraphView->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

    connect(m_RevGraphView,SIGNAL(dispDetails(const QString&)),this,SLOT(setDetailText(const QString&)));
    connect(m_RevGraphView,
                    SIGNAL(makeNorecDiff(const QString&,const svn::Revision&,const QString&,const svn::Revision&,QWidget*)),
                    this,
                    SIGNAL(makeNorecDiff(const QString&,const svn::Revision&,const QString&,const svn::Revision&,QWidget*))
           );
    connect(m_RevGraphView,
            SIGNAL(makeRecDiff(const QString&,const svn::Revision&,const QString&,const svn::Revision&,QWidget*)),
            this,
            SIGNAL(makeRecDiff(const QString&,const svn::Revision&,const QString&,const svn::Revision&,QWidget*))
           );
    connect(m_RevGraphView,
            SIGNAL(makeCat(const svn::Revision&,const QString&,const QString&,const svn::Revision&,QWidget*)),
            this,
            SIGNAL(makeCat(const svn::Revision&,const QString&,const QString&,const svn::Revision&,QWidget*))
           );

    m_Detailstext = new KTextBrowser( m_Splitter, "m_Detailstext" );
    m_Detailstext->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    //m_Detailstext->setResizePolicy( KTextBrowser::Manual );
    RevTreeWidgetLayout->addWidget( m_Splitter );
    resize( QSize(600, 480).expandedTo(minimumSizeHint()) );
    QList<int> list = Kdesvnsettings::tree_detail_height();
    if (list.count()==2 && (list[0]>0||list[1]>0)) {
        m_Splitter->setSizes(list);
    }
}

/*
 *  Destroys the object and frees any allocated resources
 */
RevTreeWidget::~RevTreeWidget()
{
    // no need to delete child widgets, Qt does it all for us
    QList<int> list = m_Splitter->sizes();
    if (list.count()==2) {
        Kdesvnsettings::setTree_detail_height(list);
        Kdesvnsettings::self()->writeConfig();
    }
}

void RevTreeWidget::setBasePath(const QString&_p)
{
    m_RevGraphView->setBasePath(_p);
}

void RevTreeWidget::dumpRevtree()
{
    m_RevGraphView->dumpRevtree();
}

void RevTreeWidget::setDetailText(const QString&_s)
{
    m_Detailstext->setText(_s);
    QList<int> list = m_Splitter->sizes();
    if (list.count()!=2) return;
    if (list[1]==0) {
        int h = height();
        int th = h/10;
        list[0]=h-th;
        list[1]=th;
        m_Splitter->setSizes(list);
    }
}

#include "revtreewidget.moc"

