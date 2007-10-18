/***************************************************************************
 *   Copyright (C) 2006-2007 by Rajko Albrecht                             *
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
#include "blamedisplay_impl.h"
#include "simple_logcb.h"
#include "src/settings/kdesvnsettings.h"
#include "src/svnqt/log_entry.hpp"
#include "fronthelpers/cursorstack.h"
#include "fronthelpers/widgetblockstack.h"
#include "src/ksvnwidgets/encodingselector_impl.h"

#include <k3listview.h>
#include <kglobalsettings.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kdialogbase.h>
#include <kapplication.h>
#include <ktextbrowser.h>
#include <k3listviewsearchline.h>

#include <qpixmap.h>
#include <qpainter.h>
#include <q3header.h>
#include <qmap.h>
#include <q3popupmenu.h>
#include <q3vbox.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <qlayout.h>
#include <qtextcodec.h>

#define COL_LINENR 0
#define COL_REV 1
#define COL_DATE 2
#define COL_AUT 3
#define COL_LINE 4

class LocalizedAnnotatedLine:public svn::AnnotateLine
{
public:
    LocalizedAnnotatedLine(const svn::AnnotateLine&al)
        :svn::AnnotateLine(al)
    {
        localeChanged();
    }

    void localeChanged()
    {
        if (!codec_searched) {
            cc = QTextCodec::codecForName(Kdesvnsettings::locale_for_blame());
            codec_searched = true;
        }
        if (cc) {
            m_tLine=cc->toUnicode(line().data(),line().size());
            m_tAuthor=cc->toUnicode(author().data(),author().size());
        } else {
            m_tLine=QString::FROMUTF8(line().data(),line().size());
            m_tAuthor=QString::FROMUTF8(author().data(),author().size());
        }
    }

    const QString& tAuthor()const{return m_tAuthor;}
    const QString& tLine()const{return m_tLine;}

    static void reset_codec(){codec_searched = false; cc=0;}

protected:
    QString m_tAuthor,m_tLine;

    static bool codec_searched;
    static QTextCodec * cc;
};

QTextCodec* LocalizedAnnotatedLine::cc = 0;
bool LocalizedAnnotatedLine::codec_searched = false;

class BlameDisplayItem:public K3ListViewItem
{
public:
    BlameDisplayItem(K3ListView*,const svn::AnnotateLine&,bool,BlameDisplay_impl*);
    BlameDisplayItem(K3ListView*,BlameDisplayItem*,const svn::AnnotateLine&,bool,BlameDisplay_impl*);
    virtual ~BlameDisplayItem(){}
    virtual int compare(Q3ListViewItem *i, int col, bool ascending)const;
    virtual void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment);
    virtual int rtti()const{return 1000;}

    virtual int width( const QFontMetrics & fm, const Q3ListView * lv, int c ) const;

    apr_int64_t lineNumber(){return m_Content.lineNumber();}
    svn_revnum_t rev(){return m_Content.revision();}

    void localeChanged()
    {
        m_Content.localeChanged();
        if (m_disp){
            setText(COL_AUT,m_Content.tAuthor());
        }
        QString _line = m_Content.tLine();
        _line.replace("\t","    ");
        setText(COL_LINE,QString("%1").arg(_line));
    }

protected:
    LocalizedAnnotatedLine m_Content;

    bool m_disp;

    void display();
    BlameDisplay_impl*cb;
};

BlameDisplayItem::BlameDisplayItem(K3ListView*lv,const svn::AnnotateLine&al,bool disp,BlameDisplay_impl*_c)
    : K3ListViewItem(lv),m_Content(al),m_disp(disp),cb(_c)
{
    display();
}

BlameDisplayItem::BlameDisplayItem(K3ListView*lv,BlameDisplayItem*it,const svn::AnnotateLine&al,bool disp,BlameDisplay_impl*_c)
    : K3ListViewItem(lv,it),m_Content(al),m_disp(disp),cb(_c)
{
    display();
}

#define BORDER 4

int BlameDisplayItem::width (const QFontMetrics & fm, const Q3ListView * lv, int c ) const
{
    if (c == COL_LINE) {
        return K3ListViewItem::width(QFontMetrics(KGlobalSettings::fixedFont()),lv,c)+2*BORDER;
    }
    return K3ListViewItem::width(fm,lv,c)+2*BORDER;
}

void BlameDisplayItem::display()
{
    if (m_disp){
        setText(COL_REV,QString("%1").arg(m_Content.revision()));
        setText(COL_AUT,m_Content.tAuthor());
        if (m_Content.date().isValid()) {
            setText(COL_DATE,KGlobal::locale()->formatDateTime(m_Content.date()));
        }
    }

    setText(COL_LINENR,QString("%1").arg(m_Content.lineNumber()+1));
    QString _line = m_Content.tLine();
    _line.replace("\t","    ");
    setText(COL_LINE,QString("%1").arg(_line));
}

int BlameDisplayItem::compare(Q3ListViewItem *item, int col, bool ascending)const
{
    Q_UNUSED(ascending);
    BlameDisplayItem* k = static_cast<BlameDisplayItem*>(item);
    if (col == COL_REV) {
        return k->m_Content.revision()-m_Content.revision();
    }
    if (col == COL_AUT) {
        if (Kdesvnsettings::locale_is_casesensitive()) {
            return m_Content.tAuthor().localeAwareCompare(k->m_Content.author());
        }
        return m_Content.tAuthor().compare(k->m_Content.author());
    }
    return k->m_Content.lineNumber()-m_Content.lineNumber();
}

void BlameDisplayItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment)
{
    if (alignment & (Qt::AlignTop || Qt::AlignBottom) == 0)
        alignment |= Qt::AlignVCenter;

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
        BlameDisplayData()
        {
            max=-1;
            min=INT_MAX-1;
            rev_count=0;
            up=false;
            m_cb=0;m_File="";
            m_dlg = 0;
        }
        ~BlameDisplayData(){}
        svn_revnum_t max,min;
        QMap<svn_revnum_t,QColor> m_shadingMap;
        QMap<svn_revnum_t,svn::LogEntry> m_logCache;

        QColor m_lastCalcColor;
        unsigned int rev_count;
        bool up;
        SimpleLogCb*m_cb;
        QString m_File;
        KDialogBase*m_dlg;

        QString reposRoot;
};

BlameDisplay_impl::BlameDisplay_impl(QWidget*parent,const char*name)
    : BlameDisplay(parent,name)
{
    m_Data = new BlameDisplayData();
    connect(m_BlameList,SIGNAL(selectionChanged()),this,SLOT(slotSelectionChanged()));
}

BlameDisplay_impl::BlameDisplay_impl(const QString&what,const svn::AnnotatedFile&blame,QWidget*parent,const char*name)
    : BlameDisplay(parent,name)
{
    m_Data = new BlameDisplayData();
    connect(m_BlameList,SIGNAL(selectionChanged()),this,SLOT(slotSelectionChanged()));
    setContent(what,blame);
}

void BlameDisplay_impl::setCb(SimpleLogCb*_cb)
{
    m_Data->m_cb = _cb;
}

void BlameDisplay_impl::setContent(const QString&what,const svn::AnnotatedFile&blame)
{
    m_Data->m_File = what;
    m_SearchWidget = new KListViewSearchLineWidget(m_BlameList,this);
    EncodingSelector_impl*m_Ls = new EncodingSelector_impl(Kdesvnsettings::locale_for_blame(),this);
    connect(m_Ls,SIGNAL(TextCodecChanged(const QString&)),
            this,SLOT(slotTextCodecChanged(const QString&)));

    BlameDisplayLayout->remove(m_BlameList);
    BlameDisplayLayout->addWidget(m_Ls);
    BlameDisplayLayout->addWidget(m_SearchWidget);
    BlameDisplayLayout->addWidget(m_BlameList);

    m_BlameList->setColumnAlignment(COL_REV,Qt::AlignRight);
    m_BlameList->setColumnAlignment(COL_LINENR,Qt::AlignRight);

    m_BlameList->clear();
    if (m_Data->m_dlg) {
        m_Data->m_dlg->enableButton(KDialogBase::User2,false);
    }
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
    if (Kdesvnsettings::self()->colored_blame()) {
        QColor a(160,160,160);
        int offset = 10;
        int r=0; int g=0;int b=0;
        uint colinc=0;

        for (svn_revnum_t i = m_Data->min; i<= m_Data->max;++i) {
            if (m_Data->m_shadingMap.find(i)==m_Data->m_shadingMap.end()) {
                continue;
            }
            a.setRgb(a.red()+offset,a.green()+offset,a.blue()+offset);
            m_Data->m_shadingMap[i]=a;
            if ( a.red()>245||a.green()>245||a.blue()>245 ) {
                if (colinc==0) {
                    ++colinc;
                } else if (r>=50||g>=50||b>=50) {
                    if (++colinc>6) {
                        colinc = 0;
                        r=g=b=0;
                    } else {
                        r=g=b=-10;
                    }
                }
                if (colinc & 0x1) {
                    r+=10;
                }
                if (colinc & 0x2) {
                    g+=10;
                }
                if (colinc & 0x4) {
                    b+=10;
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

void BlameDisplay_impl::slotGoLine()
{
    bool ok = true;
    int line = KInputDialog::getInteger(i18n("Show line"),i18n("Show line number"),
                                        1,1,m_BlameList->childCount(),1,&ok,this);
    if (!ok) {
        return;
    }
    Q3ListViewItem*item = m_BlameList->firstChild();
    --line;
    while (item) {
        if (item->rtti()==1000) {
            BlameDisplayItem*bit = static_cast<BlameDisplayItem*>(item);
            if (bit->lineNumber()==line) {
                m_BlameList->ensureItemVisible(bit);
                m_BlameList->setSelected(bit,true);
                return;
            }
        }
        item = item->nextSibling();
    }
}

void BlameDisplay_impl::slotContextMenuRequested(K3ListView*,Q3ListViewItem*item, const QPoint&pos)
{
    if (item==0||item->rtti()!=1000) return;
    BlameDisplayItem*bit = static_cast<BlameDisplayItem*>(item);
    Q3PopupMenu popup;
    popup.insertItem(i18n("Log message for revision"),101);
    int r = popup.exec(pos);

    switch (r)
    {
        case 101:
            showCommit(bit);
            break;
        default:
            break;
    }
}

void BlameDisplay_impl::showCommit(BlameDisplayItem*bit)
{
    if (!bit) return;
    WidgetBlockStack a(m_BlameList);
    QString text;
    if (m_Data->m_logCache.find(bit->rev())!=m_Data->m_logCache.end()) {
        text = m_Data->m_logCache[bit->rev()].message;
    } else {
        CursorStack a(Qt::BusyCursor);
        svn::LogEntry t;
        if (m_Data->m_cb && m_Data->m_cb->getSingleLog(t,bit->rev(),m_Data->m_File,m_Data->max,m_Data->reposRoot)) {
            m_Data->m_logCache[bit->rev()] = t;
            text = m_Data->m_logCache[bit->rev()].message;
        }
    }
    KDialogBase* dlg = new KDialogBase(
            KApplication::activeModalWidget(),
    "simplelog",true,QString(i18n("Logmessage for revision %1").arg(bit->rev())),
    KDialogBase::Close);
    QWidget* Dialog1Layout = dlg->makeVBoxMainWidget();
    KTextBrowser*ptr = new KTextBrowser(Dialog1Layout);
    ptr->setFont(KGlobalSettings::fixedFont());
    ptr->setWordWrap(Q3TextEdit::NoWrap);
    ptr->setText(text);
    dlg->resize(dlg->configDialogSize(*(Kdesvnsettings::self()->config()),"simplelog_display"));
    dlg->exec();
    dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"simplelog_display",false);
}

void BlameDisplay_impl::slotShowCurrentCommit()
{
    Q3ListViewItem*item = m_BlameList->selectedItem();
    if (item==0||item->rtti()!=1000) return;
    BlameDisplayItem*bit = static_cast<BlameDisplayItem*>(item);
    showCommit(bit);
}

void BlameDisplay_impl::slotSelectionChanged()
{
    if (!m_Data->m_dlg) return;
    Q3ListViewItem*item = m_BlameList->selectedItem();
    if (item==0||item->rtti()!=1000) {
        m_Data->m_dlg->enableButton(KDialogBase::User2,false);
    } else {
        m_Data->m_dlg->enableButton(KDialogBase::User2,true);
    }
}

void BlameDisplay_impl::displayBlame(SimpleLogCb*_cb,const QString&item,const svn::AnnotatedFile&blame,QWidget*,const char*name)
{
    int buttons = KDialogBase::Close|KDialogBase::User1|KDialogBase::User2;
    KDialogBase * dlg = new KDialogBase(
            KApplication::activeModalWidget(),
            name,true,QString(i18n("Blame %1")).arg(item),buttons,KDialogBase::Close,false,
            KGuiItem(i18n("Goto line")),KGuiItem(i18n("Log message for revision"),"kdesvnlog"));

    QWidget* Dialog1Layout = dlg->makeVBoxMainWidget();
    BlameDisplay_impl*ptr = new BlameDisplay_impl(Dialog1Layout);
    dlg->resize(dlg->configDialogSize(*(Kdesvnsettings::self()->config()),"blame_dlg"));
    ptr->setContent(item,blame);
    ptr->setCb(_cb);
    ptr->m_Data->m_dlg = dlg;
    dlg->enableButton(KDialogBase::User2,false);
    connect(dlg,SIGNAL(user1Clicked()),ptr,SLOT(slotGoLine()));
    connect(dlg,SIGNAL(user2Clicked()),ptr,SLOT(slotShowCurrentCommit()));
    Dialog1Layout->adjustSize();
    dlg->exec();

    dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"blame_dlg",false);
}

void BlameDisplay_impl::slotItemDoubleClicked(Q3ListViewItem*item)
{
    if (item==0||item->rtti()!=1000) return;
    BlameDisplayItem*bit = static_cast<BlameDisplayItem*>(item);
    showCommit(bit);
}

void BlameDisplay_impl::slotTextCodecChanged(const QString&what)
{
    if (Kdesvnsettings::locale_for_blame()!=what) {
        Kdesvnsettings::setLocale_for_blame(what);
        Kdesvnsettings::self()->writeConfig();
        LocalizedAnnotatedLine::reset_codec();
        QListViewItemIterator it(m_BlameList);
        while ( it.current() ) {
            BlameDisplayItem*_it = static_cast<BlameDisplayItem*>(it.current());
            _it->localeChanged();
            ++it;
        }
    }
}

#include "blamedisplay_impl.moc"
