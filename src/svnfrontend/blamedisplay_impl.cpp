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
#include "blamedisplay_impl.h"
#include "simple_logcb.h"
#include "src/settings/kdesvnsettings.h"
#include "src/svnqt/log_entry.h"
#include "fronthelpers/cursorstack.h"
#include "fronthelpers/widgetblockstack.h"
#include "src/ksvnwidgets/encodingselector_impl.h"

#include <kglobalsettings.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kdialog.h>
#include <kapplication.h>
#include <ktextbrowser.h>
#include <kvbox.h>
#include <kcolorscheme.h>
#include <ktextedit.h>
#include <kmenu.h>
#include <kaction.h>

#include <QPixmap>
#include <QPainter>
#include <QMap>
#include <QToolTip>
#include <QLayout>
#include <QTextCodec>
#include <QBrush>
#include <QTime>

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>

#define COL_LINENR 0
#define COL_REV 1
#define COL_DATE 2
#define COL_AUT 3
#define COL_LINE 4

#define TREE_ITEM_TYPE QTreeWidgetItem::UserType+1

#define BORDER 4

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
            cc = QTextCodec::codecForName(Kdesvnsettings::locale_for_blame().toLocal8Bit());
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

class BlameTreeItem:public QTreeWidgetItem
{
public:
    BlameTreeItem(QTreeWidget*,const svn::AnnotateLine&,bool,BlameDisplay_impl*);
    BlameTreeItem(const svn::AnnotateLine&,bool,BlameDisplay_impl*);
    BlameTreeItem(QTreeWidget*,BlameTreeItem*,const svn::AnnotateLine&,bool,BlameDisplay_impl*);
    virtual ~BlameTreeItem(){}
    apr_int64_t lineNumber(){return m_Content.lineNumber();}
    svn_revnum_t rev(){return m_Content.revision();}
    void localeChanged()
    {
        m_Content.localeChanged();
        if (m_disp){
            setText(COL_AUT,m_Content.tAuthor());
        }
        QString _line = m_Content.tLine();
        _line.replace('\t',"    ");
        setText(COL_LINE,QString("%1").arg(_line));
    }

protected:
    LocalizedAnnotatedLine m_Content;

    bool m_disp;
    void display();
    BlameDisplay_impl*cb;
};

BlameTreeItem::BlameTreeItem(const svn::AnnotateLine&al,bool disp,BlameDisplay_impl*_c)
    : QTreeWidgetItem(TREE_ITEM_TYPE),m_Content(al),m_disp(disp),cb(_c)
{
    display();
}

BlameTreeItem::BlameTreeItem(QTreeWidget*tv,const svn::AnnotateLine&al,bool disp,BlameDisplay_impl*_c)
    : QTreeWidgetItem(tv,TREE_ITEM_TYPE),m_Content(al),m_disp(disp),cb(_c)
{
    display();
}

BlameTreeItem::BlameTreeItem(QTreeWidget*tv,BlameTreeItem*it,const svn::AnnotateLine&al,bool disp,BlameDisplay_impl*_c)
    : QTreeWidgetItem(tv,it,TREE_ITEM_TYPE),m_Content(al),m_disp(disp),cb(_c)
{
    display();
}

void BlameTreeItem::display()
{
    setTextAlignment(COL_LINENR,Qt::AlignRight);
    setFont(COL_LINENR,KGlobalSettings::self()->fixedFont());
    setFont(COL_LINE,KGlobalSettings::self()->fixedFont());

    if (m_disp){
        setTextAlignment(COL_REV,Qt::AlignRight);
        setFont(COL_REV,KGlobalSettings::self()->fixedFont());
        setFont(COL_AUT,KGlobalSettings::self()->fixedFont());

        setText(COL_REV,QString("%1").arg(m_Content.revision()));
        setText(COL_AUT,m_Content.tAuthor());
        if (m_Content.date().isValid()) {
            setFont(COL_DATE,KGlobalSettings::self()->fixedFont());
            setText(COL_DATE,KGlobal::locale()->formatDateTime(m_Content.date()));
        }
    }
    setText(COL_LINENR,QString("%1").arg(m_Content.lineNumber()+1));
    QString _line = m_Content.tLine();
    _line.replace('\t',"    ");
    setText(COL_LINE,QString("%1").arg(_line));
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
        KDialog*m_dlg;

        QString reposRoot;
};

BlameDisplay_impl::BlameDisplay_impl(QWidget*parent)
    : QWidget(parent),Ui::BlameDisplay()
{
    setupUi(this);
    KAction*ac = new KAction(KIcon("kdesvnlog"),i18n("Log message for revision"),this);
    connect(ac,SIGNAL(triggered()), this,SLOT(slotShowCurrentCommit()));
    m_BlameTree->addAction(ac);
    m_Data = new BlameDisplayData();
    KTreeWidgetSearchLine * searchLine = m_TreeSearch->searchLine();
    searchLine->addTreeWidget(m_BlameTree);
}

void BlameDisplay_impl::setCb(SimpleLogCb*_cb)
{
    m_Data->m_cb = _cb;
}

void BlameDisplay_impl::setContent(const QString&what,const svn::AnnotatedFile&blame)
{
    m_Data->m_File = what;
    connect(m_encodingSel,SIGNAL(TextCodecChanged(const QString&)),
            this,SLOT(slotTextCodecChanged(const QString&)));

    if (m_Data->m_dlg) {
        m_Data->m_dlg->enableButton(KDialog::User2,false);
    }
    svn::AnnotatedFile::const_iterator bit;
    //m_BlameList->setSorting(COL_LINENR,false);
    m_Data->max = -1;
    svn_revnum_t lastRev(-1);
    QColor a(160,160,160);
    int offset = 10;
    int r=0; int g=0;int b=0;
    uint colinc=0;

    QTime t,s;
    t.start();
    QList<QTreeWidgetItem*> _list;
    QBrush _b,_bt,_bb;

    bool _b_init=false,_bt_init=false;


    for (bit=blame.begin();bit!=blame.end();++bit) {
        bool disp = (*bit).revision()!=lastRev || bit==blame.begin() ;

        if ((*bit).revision()>m_Data->max) {m_Data->max=(*bit).revision();++(m_Data->rev_count);}
        if ((*bit).revision()<m_Data->min) m_Data->min=(*bit).revision();
        s.start();
        BlameTreeItem*item = new BlameTreeItem((*bit),disp,this);
        _list.append(item);

        if (disp) {
            lastRev = (*bit).revision();
        }
        if (Kdesvnsettings::self()->colored_blame()) {
            if (m_Data->m_shadingMap.find((*bit).revision())==m_Data->m_shadingMap.end()) {
                m_Data->m_shadingMap[(*bit).revision()]=QColor();
                a.setRgb(a.red()+offset,a.green()+offset,a.blue()+offset);
                m_Data->m_shadingMap[(*bit).revision()]=a;
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
            if (!_b_init) {
                _b_init= true;
                _b = item->foreground(COL_LINENR);
                _b.setColor(KColorScheme(QPalette::Active, KColorScheme::Selection).foreground().color());
                _bb = item->background(COL_LINENR);
                _b.setStyle(Qt::SolidPattern);
                _bb.setStyle(Qt::SolidPattern);
                _bb.setColor(KColorScheme(QPalette::Active, KColorScheme::Selection).background().color());
            }
            item->setForeground(COL_LINENR,_b);
            item->setBackground(COL_LINENR,_bb);

            if(!_bt_init) {
                _bt_init = true;
                _bt = item->background(COL_REV);
                _bt.setStyle(Qt::SolidPattern);
            }
            _bt.setColor(m_Data->m_shadingMap[(*bit).revision()]);
            item->setBackground(COL_REV,_bt);
            item->setBackground(COL_DATE,_bt);
            item->setBackground(COL_AUT,_bt);
            item->setBackground(COL_LINE,_bt);
        } else {
            m_Data->m_shadingMap[(*bit).revision()]=QColor();
        }
    }
    m_BlameTree->addTopLevelItems(_list);
    qDebug("Time elapsed: %d ms", t.elapsed());
    m_BlameTree->resizeColumnToContents(COL_REV);
    m_BlameTree->resizeColumnToContents(COL_DATE);
    m_BlameTree->resizeColumnToContents(COL_AUT);
    m_BlameTree->resizeColumnToContents(COL_LINENR);
    m_BlameTree->resizeColumnToContents(COL_LINE);
}

const QColor BlameDisplay_impl::rev2color(svn_revnum_t r )const
{
    if (m_Data->m_shadingMap.find(r)!=m_Data->m_shadingMap.end() && m_Data->m_shadingMap[r].isValid())
    {
        return m_Data->m_shadingMap[r];
    } else {
        return m_BlameTree->viewport()->palette().base().color();
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
                                        1,1,m_BlameTree->topLevelItemCount(),1,&ok,this);
    if (!ok) {
        return;
    }
    QTreeWidgetItemIterator it(m_BlameTree);
    --line;
    while(*it) {
        BlameTreeItem*_it = static_cast<BlameTreeItem*>((*it));
        if (_it->lineNumber()==line) {
            m_BlameTree->scrollToItem(*it);
            m_BlameTree->setCurrentItem(*it);
            return;
        }
        ++it;
    }
}

void BlameDisplay_impl::showCommit(BlameTreeItem*bit)
{
    if (!bit) return;
    WidgetBlockStack a(m_BlameTree);
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
    KDialog* dlg = new KDialog(KApplication::activeModalWidget());
    dlg->setButtons(KDialog::Close);
    dlg->setCaption(i18n("Logmessage for revision %1",bit->rev()));
    QWidget* Dialog1Layout = new KVBox(dlg);
    dlg->setMainWidget(Dialog1Layout);
    KTextEdit*ptr = new KTextEdit(Dialog1Layout);
    ptr->setFont(KGlobalSettings::fixedFont());
    ptr->setReadOnly(true);
    ptr->setWordWrapMode(QTextOption::NoWrap);
    ptr->setPlainText(text);
    KConfigGroup k(Kdesvnsettings::self()->config(),"simplelog_display");
    dlg->restoreDialogSize(k);
    dlg->exec();
    dlg->saveDialogSize(k);
}

void BlameDisplay_impl::slotShowCurrentCommit()
{
    QTreeWidgetItem*item = m_BlameTree->currentItem();
    if (item==0||item->type()!=TREE_ITEM_TYPE) return;
    BlameTreeItem*bit = static_cast<BlameTreeItem*>(item);
    showCommit(bit);
}

void BlameDisplay_impl::slotCurrentItemChanged(QTreeWidgetItem*item,QTreeWidgetItem*)
{
    if (!m_Data->m_dlg) return;
    if (item==0||item->type()!=TREE_ITEM_TYPE) {
        m_Data->m_dlg->enableButton(KDialog::User2,false);
    } else {
        m_Data->m_dlg->enableButton(KDialog::User2,true);
    }
}

void BlameDisplay_impl::displayBlame(SimpleLogCb*_cb,const QString&item,const svn::AnnotatedFile&blame,QWidget*)
{
    KDialog * dlg = new KDialog(KApplication::activeModalWidget());
    dlg->setButtons(KDialog::Close|KDialog::User1|KDialog::User2);
    dlg->setButtonGuiItem(KDialog::User1,KGuiItem(i18n("Goto line")));
    dlg->setButtonGuiItem(KDialog::User2,KGuiItem(i18n("Log message for revision"),"kdesvnlog"));
    QWidget* Dialog1Layout = new KVBox(dlg);
    dlg->setMainWidget(Dialog1Layout);

    BlameDisplay_impl*ptr = new BlameDisplay_impl(Dialog1Layout);

    KConfigGroup k(Kdesvnsettings::self()->config(),"blame_dlg");
    dlg->restoreDialogSize(k);

    ptr->setContent(item,blame);
    ptr->setCb(_cb);
    ptr->m_Data->m_dlg = dlg;
    dlg->enableButton(KDialog::User2,false);
    connect(dlg,SIGNAL(user1Clicked()),ptr,SLOT(slotGoLine()));
    connect(dlg,SIGNAL(user2Clicked()),ptr,SLOT(slotShowCurrentCommit()));
    Dialog1Layout->adjustSize();
    dlg->exec();

    dlg->saveDialogSize(k);
}

void BlameDisplay_impl::slotItemDoubleClicked (QTreeWidgetItem*item,int)
{
    if (item==0||item->type()!=TREE_ITEM_TYPE) return;
    BlameTreeItem*bit = static_cast<BlameTreeItem*>(item);
    showCommit(bit);
}

void BlameDisplay_impl::slotTextCodecChanged(const QString&what)
{
    if (Kdesvnsettings::locale_for_blame()!=what) {
        Kdesvnsettings::setLocale_for_blame(what);
        Kdesvnsettings::self()->writeConfig();
        LocalizedAnnotatedLine::reset_codec();

        QTreeWidgetItemIterator it(m_BlameTree);

        while(*it) {
            BlameTreeItem*_it = static_cast<BlameTreeItem*>((*it));
            _it->localeChanged();
            ++it;
        }
    }
}

#include "blamedisplay_impl.moc"
