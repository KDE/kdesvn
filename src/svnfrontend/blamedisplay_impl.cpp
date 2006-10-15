#include "blamedisplay_impl.h"
#include "src/settings/kdesvnsettings.h"
#include <klistview.h>
#include <kglobalsettings.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include <qpixmap.h>
#include <qpainter.h>
#include <qheader.h>
#include <qmap.h>

#define COL_LINENR 0
#define COL_REV 1
#define COL_DATE 2
#define COL_AUT 3
#define COL_LINE 4

class BlameDisplayItem:public KListViewItem
{
public:
    BlameDisplayItem(KListView*,const svn::AnnotateLine&,bool,BlameDisplay_impl*);
    BlameDisplayItem(KListView*,BlameDisplayItem*,const svn::AnnotateLine&,bool,BlameDisplay_impl*);
    virtual ~BlameDisplayItem(){}
    virtual int compare(QListViewItem *i, int col, bool ascending)const;
    virtual void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment);
    virtual int rtti()const{return 1000;}

    virtual int width( const QFontMetrics & fm, const QListView * lv, int c ) const;

protected:
    svn::AnnotateLine m_Content;

    bool m_disp;

    void display();
    BlameDisplay_impl*cb;
};

BlameDisplayItem::BlameDisplayItem(KListView*lv,const svn::AnnotateLine&al,bool disp,BlameDisplay_impl*_c)
    : KListViewItem(lv),m_Content(al),m_disp(disp),cb(_c)
{
    display();
}

BlameDisplayItem::BlameDisplayItem(KListView*lv,BlameDisplayItem*it,const svn::AnnotateLine&al,bool disp,BlameDisplay_impl*_c)
    : KListViewItem(lv,it),m_Content(al),m_disp(disp),cb(_c)
{
    display();
}

#define BORDER 4

int BlameDisplayItem::width (const QFontMetrics & fm, const QListView * lv, int c ) const
{
    if (c == COL_LINE) {
        return KListViewItem::width(QFontMetrics(KGlobalSettings::fixedFont()),lv,c)+2*BORDER;
    }
    return KListViewItem::width(fm,lv,c)+2*BORDER;
}

void BlameDisplayItem::display()
{
    if (m_disp){
        setText(COL_REV,QString("%1").arg(m_Content.revision()));
        setText(COL_AUT,m_Content.author());
        if (m_Content.date().isValid()) {
            setText(COL_DATE,KGlobal::locale()->formatDateTime(m_Content.date()));
        }
    }

    setText(COL_LINENR,QString("%1").arg(m_Content.lineNumber()+1));
    QString _line = m_Content.line();
    _line.replace("\t","    ");
    setText(COL_LINE,QString("%1").arg(_line));
}

int BlameDisplayItem::compare(QListViewItem *item, int col, bool ascending)const
{
    Q_UNUSED(ascending);
    BlameDisplayItem* k = static_cast<BlameDisplayItem*>(item);
    if (col == COL_REV) {
        return k->m_Content.revision()-m_Content.revision();
    }
    if (col == COL_AUT) {
        if (Kdesvnsettings::locale_is_casesensitive()) {
            return m_Content.author().localeAwareCompare(k->m_Content.author());
        }
        return m_Content.author().compare(k->m_Content.author());
    }
    return k->m_Content.lineNumber()-m_Content.lineNumber();
}

void BlameDisplayItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment)
{
    if (alignment & (AlignTop || AlignBottom) == 0)
        alignment |= AlignVCenter;

    /* don't copy string */
    const QString & str = text(column);;
    if (column == COL_LINE) {
        p->setFont(KGlobalSettings::fixedFont());
    }

    QColorGroup _cg = cg;
    QColor _bgColor;
    if (column==COL_LINENR || isSelected()) {
        _bgColor = KGlobalSettings::highlightColor();
        p->setPen(KGlobalSettings::highlightedTextColor());
    } else {
        if (Kdesvnsettings::self()->colored_blame()) {
            _bgColor = cb->rev2color(m_Content.revision());
        } else {
            _bgColor = listView()->viewport()->colorGroup().base();
        }
    }

    p->fillRect(0, 0, width, height(), _bgColor);
    if (column==COL_AUT) {
        p->drawLine(width-1,0,width-1,height());
    }

    if (str.isEmpty())
        return;
    p->drawText(BORDER, 0, width - 2*BORDER, height(), alignment, str);
}

class BlameDisplayData
{
    public:
        BlameDisplayData(){max=-1;min=INT_MAX-1;rev_count=0;up=false;}
        ~BlameDisplayData(){}
        svn_revnum_t max,min;
        QMap<svn_revnum_t,QColor> m_shadingMap;
        QColor m_lastCalcColor;
        unsigned int rev_count;
        bool up;
};

BlameDisplay_impl::BlameDisplay_impl(QWidget*parent,const char*name)
    : BlameDisplay(parent,name)
{
    m_Data = new BlameDisplayData();
}

BlameDisplay_impl::BlameDisplay_impl(const svn::AnnotatedFile&blame,QWidget*parent,const char*name)
    : BlameDisplay(parent,name)
{
    m_Data = new BlameDisplayData();
    setContent(blame);
}

void BlameDisplay_impl::setContent(const svn::AnnotatedFile&blame)
{
    m_BlameList->setColumnAlignment(COL_REV,Qt::AlignRight);
    m_BlameList->setColumnAlignment(COL_LINENR,Qt::AlignRight);
    m_BlameList->header()->setLabel(COL_LINE,QString(""));

    m_BlameList->clear();
    svn::AnnotatedFile::const_iterator bit;
    m_BlameList->setSorting(COL_LINENR,false);
    m_Data->max = -1;
    svn_revnum_t lastRev(-1);
    for (bit=blame.begin();bit!=blame.end();++bit) {
        bool disp = (*bit).revision()!=lastRev || bit==blame.begin() ;

        if ((*bit).revision()>m_Data->max) {m_Data->max=(*bit).revision();++(m_Data->rev_count);}
        if ((*bit).revision()<m_Data->min) m_Data->min=(*bit).revision();
        new BlameDisplayItem(m_BlameList,(*bit),disp,this);
        if (disp) {
            lastRev = (*bit).revision();
        }
        if (m_Data->m_shadingMap.find((*bit).revision())==m_Data->m_shadingMap.end()) {
            m_Data->m_shadingMap[(*bit).revision()]=QColor();
        }
    }
    kdDebug()<<"Max revision: "<<m_Data->max << endl;
    kdDebug()<<"Min revision: "<<m_Data->min << endl;
    if (Kdesvnsettings::self()->colored_blame()) {
        QColor a(160,160,160);
        int offset = 10;
        int r=0; int g=0;int b=0;
        int colortoggle=0;
        for (svn_revnum_t i = m_Data->min; i<= m_Data->max;++i) {
            if (m_Data->m_shadingMap.find(i)==m_Data->m_shadingMap.end()) {
                continue;
            }
            a.setRgb(a.red()+offset,a.green()+offset,a.blue()+offset);
            m_Data->m_shadingMap[i]=a;
            if ( a.red()>245||a.green()>245||a.blue()>245 ) {
                if (colortoggle==0) {
                    if (r<=40) {
                        r+=10;
                    } else {
                        r = 0;
                        ++colortoggle;
                    }
                }
                if (colortoggle==1) {
                    if (g<=40) {
                        g+=10;
                    } else {
                        g = 0;
                        ++colortoggle;
                    }
                }
                if (colortoggle==2) {
                    if (b<=40) {
                        b+=10;
                    } else {
                        b = 0;
                        colortoggle=0;
                    }
                }
                a.setRgb(160+r,160+g,160+b);
            }
        }
    }
}

const QColor BlameDisplay_impl::rev2color(svn_revnum_t r )const
{
    if (m_Data->m_shadingMap.find(r)!=m_Data->m_shadingMap.end() && m_Data->m_shadingMap[r].isValid())
    {
        return m_Data->m_shadingMap[r];
    } else {
        return m_BlameList->viewport()->colorGroup().base();
    }
}

BlameDisplay_impl::~BlameDisplay_impl()
{
    delete m_Data;
}

#include "blamedisplay_impl.moc"
