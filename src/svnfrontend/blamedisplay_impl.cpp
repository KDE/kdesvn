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

#define COL_LINENR 0
#define COL_REV 1
#define COL_DATE 2
#define COL_AUT 3
#define COL_LINE 4

class BlameDisplayItem:public KListViewItem
{
public:
    BlameDisplayItem(KListView*,const svn::AnnotateLine&,bool,bool,bool);
    BlameDisplayItem(KListView*,BlameDisplayItem*,const svn::AnnotateLine&,bool,bool,bool);
    virtual ~BlameDisplayItem(){}
    virtual int compare(QListViewItem *i, int col, bool ascending)const;
    virtual void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment);
    virtual int rtti()const{return 1000;}

    virtual int width( const QFontMetrics & fm, const QListView * lv, int c ) const;

protected:
    svn::AnnotateLine m_Content;

    bool m_author,m_date,m_rev;

    void display();
};

BlameDisplayItem::BlameDisplayItem(KListView*lv,const svn::AnnotateLine&al,bool a,bool d,bool r)
    : KListViewItem(lv),m_Content(al),m_author(a),m_date(d),m_rev(r)
{
    display();
}

BlameDisplayItem::BlameDisplayItem(KListView*lv,BlameDisplayItem*it,const svn::AnnotateLine&al,bool a,bool d,bool r)
    : KListViewItem(lv,it),m_Content(al),m_author(a),m_date(d),m_rev(r)
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
    if (m_rev) setText(COL_REV,QString("%1").arg(m_Content.revision()));
    if (m_author) setText(COL_AUT,m_Content.author());
    if (m_date) setText(COL_DATE,KGlobal::locale()->formatDateTime(m_Content.date()));
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

    if (column == COL_LINE) {
        p->setFont(KGlobalSettings::fixedFont());
        if (isSelected()) {
            p->setPen(KGlobalSettings::highlightedTextColor());
        }
        p->fillRect(0, 0, width, height(), isSelected()?KGlobalSettings::highlightColor():cg.background());
        p->drawText(BORDER, 0, width - 2*BORDER, height(), alignment, m_Content.line());
        return;
    }
    QColorGroup _cg = cg;
    QColor _bgColor;
    if (column==COL_LINENR || isSelected()) {
        _bgColor = KGlobalSettings::highlightColor();
        p->setPen(KGlobalSettings::highlightedTextColor());
    } else if (column==COL_DATE ){
        _bgColor = QColor(210,230,230);
    } else if (column==COL_REV ){
        _bgColor = QColor(230,230,210);
    } else if (column==COL_AUT ){
        _bgColor = QColor(230,230,230);
    } else {
        KListViewItem::paintCell(p,cg,column,width,alignment);
        return;
    }
    const QPixmap *pm = listView()->viewport()->backgroundPixmap();
    if (pm && !pm->isNull()) {
        _cg.setBrush(QColorGroup::Base, QBrush(_bgColor, *pm));
        QPoint o = p->brushOrigin();
        p->setBrushOrigin( o.x()-listView()->contentsX(), o.y()-listView()->contentsY() );
    } else {
        if (listView()->viewport()->backgroundMode()==Qt::FixedColor) {
            _cg.setColor(QColorGroup::Background,_bgColor);
        } else {
            _cg.setColor(QColorGroup::Base,_bgColor);
        }
    }
    p->fillRect(0, 0, width, height(), _bgColor);
    QString str = text(column);
    if (str.isEmpty())
        return;
    p->drawText(BORDER, 0, width - 2*BORDER, height(), alignment, str);
    //QListViewItem::paintCell(p, _cg, column, width, alignment);
}

class BlameDisplayData
{
    public:
        BlameDisplayData(){max=-1;min=INT_MAX-1;}
        ~BlameDisplayData(){}
        svn_revnum_t max,min;
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
    QDateTime lastDate;
    QString lastAuthor;
    svn_revnum_t lastRev(-1);
    for (bit=blame.begin();bit!=blame.end();++bit) {
        bool disp =
                (*bit).author()!=lastAuthor ||
                (*bit).date()!=lastDate ||
                (*bit).revision()!=lastRev;

        if ((*bit).revision()>m_Data->max) m_Data->max=(*bit).revision();
        if ((*bit).revision()<m_Data->min) m_Data->min=(*bit).revision();
        new BlameDisplayItem(m_BlameList,(*bit),disp,disp,disp);
        lastRev = (*bit).revision();
        lastDate = (*bit).date();
        lastAuthor = (*bit).author();
    }
    kdDebug()<<"Max revision: "<<m_Data->max << endl;
    kdDebug()<<"Min revision: "<<m_Data->min << endl;
}

QColor BlameDisplay_impl::rev2color(svn_revnum_t /* r */)
{
    return QColor();
}

BlameDisplay_impl::~BlameDisplay_impl()
{
    delete m_Data;
}

#include "blamedisplay_impl.moc"
