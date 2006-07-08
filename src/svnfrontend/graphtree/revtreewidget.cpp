#include "revtreewidget.h"
#include "src/settings/kdesvnsettings.h"

#include <qvariant.h>
#include <qsplitter.h>
#include <ktextbrowser.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include "revgraphview.h"
#include "ktextbrowser.h"

/*
 *  Constructs a RevTreeWidget as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
RevTreeWidget::RevTreeWidget(QObject*lt,svn::Client*cl, QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
        setName( "RevTreeWidget" );
    RevTreeWidgetLayout = new QVBoxLayout( this, 11, 6, "RevTreeWidgetLayout");

    m_Splitter = new QSplitter( this, "m_Splitter" );
    m_Splitter->setOrientation( QSplitter::Vertical );

    m_RevGraphView = new RevGraphView(lt,cl, m_Splitter, "m_RevGraphView" );
    m_RevGraphView->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 2, m_RevGraphView->sizePolicy().hasHeightForWidth() ) );
    connect(m_RevGraphView,SIGNAL(dispDetails(const QString&)),this,SLOT(setDetailText(const QString&)));
    connect(m_RevGraphView,SIGNAL(dispDiff(const QString&)),this,SIGNAL(dispDiff(const QString&)));
    connect(m_RevGraphView,SIGNAL(makeCat(const svn::Revision&,const QString&,const QString&,const svn::Revision&,QWidget*))
        ,this,SIGNAL(makeCat(const svn::Revision&,const QString&,const QString&,const svn::Revision&,QWidget*)));

    m_Detailstext = new KTextBrowser( m_Splitter, "m_Detailstext" );
    m_Detailstext->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, m_Detailstext->sizePolicy().hasHeightForWidth() ) );
    m_Detailstext->setResizePolicy( KTextBrowser::Manual );
    RevTreeWidgetLayout->addWidget( m_Splitter );
    languageChange();
    resize( QSize(600, 480).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
    QValueList<int> list = Kdesvnsettings::tree_detail_height();
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
    QValueList<int> list = m_Splitter->sizes();
    if (list.count()==2) {
        Kdesvnsettings::setTree_detail_height(list);
        Kdesvnsettings::writeConfig();
    }
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void RevTreeWidget::languageChange()
{
    setCaption( tr( "RevTreeWidget" ) );
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
    QValueList<int> list = m_Splitter->sizes();
    if (list.count()!=2) return;
    if (list[1]==0) {
        int h = height();
        int th = h/10;
        list[0]=h-th;
        list[1]=th;
        m_Splitter->setSizes(list);
    }
}
