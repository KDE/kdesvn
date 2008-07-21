/***************************************************************************
 *   Copyright (C) 2005-2007 by Rajko Albrecht                             *
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
#include "logmsg_impl.h"
#include "src/settings/kdesvnsettings.h"
#include "depthselector.h"

#include <ktextedit.h>
#include <kcombobox.h>
#include <kdialog.h>
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kapplication.h>
#include <kconfigbase.h>
#include <kconfig.h>

#include <QStringList>

#include <q3vbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <q3listview.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <Q3ValueList>

#define MAX_MESSAGE_HISTORY 10

QStringList Logmsg_impl::sLogHistory = QStringList();
QString Logmsg_impl::sLastMessage=QString();
const QString Logmsg_impl::groupName("logmsg_dlg_size");

unsigned int Logmsg_impl::smax_message_history = 0xFFFF;

class SvnCheckListItem:public Q3CheckListItem
{
protected:
    Logmsg_impl::logActionEntry m_Content;
public:
    SvnCheckListItem(Q3ListView*,const Logmsg_impl::logActionEntry&);
    const Logmsg_impl::logActionEntry&data(){return m_Content;}
    virtual int rtti()const{return 1000;}
protected:
    virtual int compare( Q3ListViewItem* item, int col, bool ascending ) const;
};

Logmsg_impl::Logmsg_impl(QWidget *parent)
    :QWidget(parent),LogmessageData()
{
    setupUi(this);
    m_LogEdit->setFocus();
    m_Reviewlabel->hide();
    m_ReviewList->hide();
    m_hidden=true;
    hideButtons(true);
    m_MainSplitter->insertWidget(0,m_EditFrame);
    delete m_ReviewFrame;
    m_Reviewlabel=0;
    m_ReviewList=0;
    m_MarkUnversioned=0;
    m_UnmarkUnversioned=0;
    m_DiffItem=0;
}

Logmsg_impl::Logmsg_impl(const svn::CommitItemList&_items,QWidget *parent)
    :QWidget(parent),LogmessageData()
{
    setupUi(this);
    m_LogEdit->setFocus();
    m_ReviewList->setColumnText(1,i18n("Items to commit"));
    m_ReviewList->setColumnText(0,i18n("Action"));
    m_ReviewList->setSortColumn(1);
    hideButtons(true);
    if (_items.count()>0) {
        for (unsigned i = 0;i<_items.count();++i) {
            Q3ListViewItem*item = new Q3ListViewItem(m_ReviewList);
            if (_items[i].path().isEmpty()) {
                item->setText(1,_items[i].url());
            } else {
                item->setText(1,_items[i].path());
            }
            item->setText(0,QChar(_items[i].actionType()));
        }
        m_hidden=false;
    } else {
        m_Reviewlabel->hide();
        m_ReviewList->hide();
        m_hidden=true;
    }
    checkSplitterSize();
}

Logmsg_impl::Logmsg_impl(const QMap<QString,QString>&_items,QWidget *parent)
    :QWidget(parent),LogmessageData()
{
    setupUi(this);
    m_LogEdit->setFocus();
    m_ReviewList->setColumnText(1,i18n("Items to commit"));
    m_ReviewList->setColumnText(0,i18n("Action"));
    m_ReviewList->setSortColumn(1);
    hideButtons(true);
    if (_items.count()>0) {
        QMap<QString,QString>::ConstIterator it = _items.begin();
        for (;it!=_items.end();++it) {
            Q3ListViewItem*item = new Q3ListViewItem(m_ReviewList);
            item->setText(1,it.key());
            item->setText(0,it.value());
        }
        m_hidden=false;
    } else {
        m_Reviewlabel->hide();
        m_ReviewList->hide();
        m_hidden=true;
    }
    checkSplitterSize();
}

Logmsg_impl::Logmsg_impl(const logActionEntries&_activatedList,
        const logActionEntries&_notActivatedList,
        QWidget *parent)
    :QWidget(parent),LogmessageData()
{
    setupUi(this);
    m_LogEdit->setFocus();
    m_hidden=false;
    for (unsigned j = 0; j<_activatedList.count();++j) {
        SvnCheckListItem * item = new SvnCheckListItem(m_ReviewList,_activatedList[j]);
        item->setState(Q3CheckListItem::On);
    }
    for (unsigned j = 0; j<_notActivatedList.count();++j) {
        SvnCheckListItem * item = new SvnCheckListItem(m_ReviewList,_notActivatedList[j]);
        item->setState(Q3CheckListItem::Off);
    }
    m_HideNewItems->setChecked(Kdesvnsettings::commit_hide_new());
    checkSplitterSize();
}

Logmsg_impl::~Logmsg_impl()
{
    Q3ValueList<int> list = m_MainSplitter->sizes();
    if (!m_hidden && list.count()==2) {
        Kdesvnsettings::setCommit_splitter_height(list);
        Kdesvnsettings::self()->writeConfig();
    }
    for (unsigned int j=0; j<m_Hidden.size();++j) {
        delete m_Hidden[j];
    }
    Kdesvnsettings::setCommit_hide_new(m_HideNewItems->isChecked());
}

void Logmsg_impl::checkSplitterSize()
{
    Q3ValueList<int> list = Kdesvnsettings::commit_splitter_height();
    if (list.count()!=2) {
        return;
    }
    if (m_hidden) {
        list[1]=list[0]+list[1];
        list[0]=0;
    }
    if (m_hidden || (list[0]>0||list[1]>0)) {
        m_MainSplitter->setSizes(list);
    }
}

void Logmsg_impl::slotHistoryActivated(int number)
{
    if (number < 1||(unsigned)number>sLogHistory.size()) {
        m_LogEdit->setText("");
    } else {
        m_LogEdit->setText(sLogHistory[number-1]);
    }
}

/*!
    \fn Logmsg_impl::getMessage()const
 */
QString Logmsg_impl::getMessage()const
{
    return m_LogEdit->toPlainText();
}


/*!
    \fn Logmsg_impl::isRecursive()const
 */
svn::Depth Logmsg_impl::getDepth()const
{
    return m_DepthSelector->getDepth();
}

/*!
    \fn Logmsg_impl::isRecursive()const
 */
bool Logmsg_impl::isKeeplocks()const
{
    return m_keepLocksButton->isChecked();
}


/*!
    \fn Logmsg_impl::initHistory()
 */
void Logmsg_impl::initHistory()
{
    if (smax_message_history==0xFFFF) {
        smax_message_history = Kdesvnsettings::max_log_messages();
        KConfigGroup cs(Kdesvnsettings::self()->config(),"log_messages");
        QString s = QString::null;
        unsigned int current = 0;
        QString key = QString("log_%0").arg(current);
        s = cs.readEntry(key,QString::null);
        while (s!=QString::null) {
            if (current<smax_message_history) {
                sLogHistory.push_back(s);
            } else {
                cs.deleteEntry(key);
            }
            ++current;
            key = QString("log_%0").arg(current);
            s = cs.readEntry(key,QString::null);
        }
    }
    QStringList::const_iterator it;
    for (it=sLogHistory.begin();it!=sLogHistory.end();++it) {
        if ((*it).length()<=40) {
            m_LogHistory->insertItem((*it));
        } else {
            m_LogHistory->insertItem((*it).left(37)+"...");
        }
    }
    if (sLastMessage.length()>0) {
        m_LogEdit->setText(sLastMessage);
        sLastMessage=QString();
    }
}


/*!
    \fn Logmsg_impl::saveHistory()
 */
void Logmsg_impl::saveHistory(bool canceld)
{
    QString _text = m_LogEdit->toPlainText();
    if (_text.length()==0) return;
    /// @todo make static threadsafe
    if (!canceld) {
        int it;
        if ( (it=sLogHistory.indexOf(_text))!=-1) {
            sLogHistory.removeAt(it);
        }
        sLogHistory.push_front(_text);
        if (sLogHistory.size()>smax_message_history) {
            sLogHistory.removeLast();
        }
        KConfigGroup cs(Kdesvnsettings::self()->config(),"log_messages");
        for (unsigned int i = 0; i < sLogHistory.size();++i) {
            cs.writeEntry(QString("log_%0").arg(i),sLogHistory[i]);
        }
        cs.sync();
    } else {
        sLastMessage=_text;
    }
}

QString Logmsg_impl::getLogmessage(bool*ok,svn::Depth*rec,bool*keep_locks,QWidget*parent)
{
    bool _ok,_keep_locks;
    svn::Depth _depth = svn::DepthUnknown;
    QString msg("");

    Logmsg_impl*ptr=0;
    KDialogBase dlg(parent,name,true,i18n("Commit log"),
            KDialogBase::Ok|KDialogBase::Cancel,
            KDialogBase::Ok,true);
    QWidget* Dialog1Layout = dlg.makeVBoxMainWidget();

    ptr = new Logmsg_impl(Dialog1Layout);
    if (!rec) {
        ptr->m_DepthSelector->hide();
    }
    if (!keep_locks) {
        ptr->m_keepLocksButton->hide();
    }
    ptr->initHistory();
    dlg.resize(dlg.configDialogSize(*(Kdesvnsettings::self()->config()),groupName));
    if (dlg.exec()!=QDialog::Accepted) {
        _ok = false;
        /* avoid compiler warnings */
        _keep_locks = false;
    } else {
        _ok = true;
        _depth = ptr->getDepth();
        _keep_locks = ptr->isKeeplocks();
        msg=ptr->getMessage();
    }
    ptr->saveHistory(!_ok);

    dlg.saveDialogSize(*(Kdesvnsettings::self()->config()),groupName,false);
    if (ok) *ok = _ok;
    if (rec) *rec = _depth;
    return msg;
}

QString Logmsg_impl::getLogmessage(const svn::CommitItemList&items,bool*ok,svn::Depth*rec,bool*keep_locks,QWidget*parent)
{
    bool _ok,_keep_locks;
    svn::Depth _depth = svn::DepthUnknown;
    QString msg("");

    Logmsg_impl*ptr=0;
    KDialogBase dlg(parent,name,true,i18n("Commit log"),
            KDialogBase::Ok|KDialogBase::Cancel,
            KDialogBase::Ok,true);
    QWidget* Dialog1Layout = dlg.makeVBoxMainWidget();

    ptr = new Logmsg_impl(items,Dialog1Layout);
    if (!rec) {
        ptr->m_DepthSelector->hide();
    }
    if (!keep_locks) {
        ptr->m_keepLocksButton->hide();
    }

    ptr->initHistory();
    dlg.resize(dlg.configDialogSize(*(Kdesvnsettings::self()->config()),groupName));
    if (dlg.exec()!=QDialog::Accepted) {
        _ok = false;
        /* avoid compiler warnings */
        _keep_locks = false;
    } else {
        _ok = true;
        _depth = ptr->getDepth();
        _keep_locks = ptr->isKeeplocks();
        msg=ptr->getMessage();
    }
    ptr->saveHistory(!_ok);

    dlg.saveDialogSize(*(Kdesvnsettings::self()->config()),groupName,false);
    if (ok) *ok = _ok;
    if (rec) *rec = _depth;
    if (keep_locks) *keep_locks = _keep_locks;
    return msg;
}

QString Logmsg_impl::getLogmessage(const QMap<QString,QString>&items,
    bool*ok,svn::Depth*rec,bool*keep_locks,QWidget*parent)
{
    bool _ok,_rec,_keep_locks;
    svn::Depth _depth = svn::DepthUnknown;
    QString msg("");

    Logmsg_impl*ptr=0;
    KDialogBase dlg(parent,name,true,i18n("Commit log"),
            KDialogBase::Ok|KDialogBase::Cancel,
            KDialogBase::Ok,true);
    QWidget* Dialog1Layout = dlg.makeVBoxMainWidget();

    ptr = new Logmsg_impl(items,Dialog1Layout);
    if (!rec) {
        ptr->m_DepthSelector->hide();
    }
    if (!keep_locks) {
        ptr->m_keepLocksButton->hide();
    }
    ptr->initHistory();
    dlg.resize(dlg.configDialogSize(*(Kdesvnsettings::self()->config()),groupName));
    if (dlg.exec()!=QDialog::Accepted) {
        _ok = false;
        /* avoid compiler warnings */
        _rec = false;
        _keep_locks=false;
    } else {
        _ok = true;
        _depth = ptr->getDepth();
        msg=ptr->getMessage();
        _keep_locks = ptr->isKeeplocks();
    }
    ptr->saveHistory(!_ok);

    dlg.saveDialogSize(*(Kdesvnsettings::self()->config()),groupName,false);
    if (ok) *ok = _ok;
    if (rec) *rec = _depth;
    if (keep_locks) *keep_locks = _keep_locks;
    return msg;
}

QString Logmsg_impl::getLogmessage(const logActionEntries&_on,
            const logActionEntries&_off,
            QObject*callback,
            logActionEntries&_result,
            bool*ok,bool*keep_locks,QWidget*parent)
{
    bool _ok,_keep_locks;
    QString msg("");

    Logmsg_impl*ptr=0;
    KDialogBase dlg(parent,name,true,i18n("Commit log"),
            KDialogBase::Ok|KDialogBase::Cancel,
            KDialogBase::Ok,true);
    QWidget* Dialog1Layout = dlg.makeVBoxMainWidget();
    ptr = new Logmsg_impl(_on,_off,Dialog1Layout);
    ptr->m_DepthSelector->hide();
    if (!keep_locks) {
        ptr->m_keepLocksButton->hide();
    }
    ptr->initHistory();
    if (callback)
    {
        connect(ptr,SIGNAL(makeDiff(const QString&,const svn::Revision&,const QString&,const svn::Revision&,QWidget*)),
                callback,SLOT(makeDiff(const QString&,const svn::Revision&,const QString&,const svn::Revision&,QWidget*)));
    }
    dlg.resize(dlg.configDialogSize(*(Kdesvnsettings::self()->config()),groupName));
    if (dlg.exec()!=QDialog::Accepted) {
        _ok = false;
        /* avoid compiler warnings */
        _keep_locks=false;
    } else {
        _ok = true;
        msg=ptr->getMessage();
        _keep_locks = ptr->isKeeplocks();
    }
    ptr->saveHistory(!_ok);
    dlg.saveDialogSize(*(Kdesvnsettings::self()->config()),groupName,false);
    if (ok) *ok = _ok;
    _result = ptr->selectedEntries();
    if (keep_locks) *keep_locks = _keep_locks;
    return msg;
}

/*!
    \fn Logmsg_impl::setRecCheckboxtext(const QString&what)
 */
void Logmsg_impl::addItemWidget(QWidget*aWidget)
{
    m_DepthSelector->addItemWidget(aWidget);
/*    aWidget->reparent(this,geometry().topLeft());
    m_ItemsLayout->addWidget(aWidget);
    kDebug()<<"SizeHint: "<<aWidget->minimumSizeHint()<< endl;
    aWidget->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    setMinimumHeight(minimumSizeHint().height());*/
}

Logmsg_impl::logActionEntries Logmsg_impl::selectedEntries()
{
    logActionEntries _result;
    if (m_ReviewList) {
        Q3ListViewItemIterator it( m_ReviewList );
        while ( it.current() ) {
            if (it.current()->rtti()==1000) {
                SvnCheckListItem *item = static_cast<SvnCheckListItem*>(it.current());
                if (item->isOn()) {
                    _result.append(item->data());
                }
            }
            ++it;
        }
    }
    return _result;
}

Logmsg_impl::logActionEntry::logActionEntry(const QString&name,const QString&action,ACTION_TYPE kind)
    : _name(name),_actionDesc(action),_kind(kind)
{
}

Logmsg_impl::logActionEntry::logActionEntry()
    : _name(""),_actionDesc(""),_kind(COMMIT)
{
}

SvnCheckListItem::SvnCheckListItem(Q3ListView*parent,const Logmsg_impl::logActionEntry&content)
    :Q3CheckListItem(parent,content._name,Q3CheckListItem::CheckBox),m_Content(content)
{
    setTristate(FALSE);
    setText(1,m_Content._actionDesc);
    if (content._name.isEmpty()) {
        setText(0,"...");
    }
}

int SvnCheckListItem::compare( Q3ListViewItem* item, int col, bool ascending ) const
{
    if (item->rtti()!=1000 || col>0) {
        return Q3CheckListItem::compare(item,col,ascending);
    }
    SvnCheckListItem* k = static_cast<SvnCheckListItem*>( item );
    if (Kdesvnsettings::case_sensitive_sort()) {
        if (Kdesvnsettings::locale_is_casesensitive()) {
            return m_Content._name.toLower().localeAwareCompare(k->m_Content._name.toLower());
        }
        return m_Content._name.compare(k->m_Content._name);
    } else {
        return m_Content._name.toLower().localeAwareCompare(k->m_Content._name.toLower());
    }
}

void Logmsg_impl::slotUnmarkUnversioned()
{
    markUnversioned(false);
}

void Logmsg_impl::slotMarkUnversioned()
{
    markUnversioned(true);
}

void Logmsg_impl::slotDiffSelected()
{
    Q3ListViewItem * it=0;
    if (!m_ReviewList || !(it=m_ReviewList->selectedItem()))
    {
        return;
    }
    if (it->rtti()==1000)
    {
        SvnCheckListItem *item = static_cast<SvnCheckListItem*>(it);
        QString what = item->data()._name;
        emit makeDiff(what,svn::Revision::BASE,what,svn::Revision::WORKING,parentWidget());
    }
}

void Logmsg_impl::hideButtons(bool how)
{
    if (!m_MarkUnversioned)return;
    if (how)
    {
        m_MarkUnversioned->hide();
        m_UnmarkUnversioned->hide();
        m_DiffItem->hide();
        m_HideNewItems->hide();
    }
    else
    {
        m_MarkUnversioned->show();
        m_UnmarkUnversioned->show();
        m_DiffItem->show();
        m_HideNewItems->show();
    }
}

/*!
    \fn Logmsg_impl::markUnversioned(bool mark)
 */
void Logmsg_impl::markUnversioned(bool mark)
{
    if (!m_ReviewList)return;
    Q3ListViewItemIterator it( m_ReviewList );
    while ( it.current() ) {
        if (it.current()->rtti()==1000) {
            SvnCheckListItem *item = static_cast<SvnCheckListItem*>(it.current());
            if (item->data()._kind==logActionEntry::ADD_COMMIT) {
                item->setOn(mark);
            }
        }
        ++it;
    }
}

void Logmsg_impl::hideNewItems(bool how)
{
    if (!m_ReviewList)return;

    if (how) {
        Q3ListViewItemIterator it( m_ReviewList );
        while ( it.current() ) {
            if (it.current()->rtti()==1000) {
                SvnCheckListItem *item = static_cast<SvnCheckListItem*>(it.current());
                if (item->data()._kind==logActionEntry::ADD_COMMIT) {
                    item->setOn(false);
                    m_Hidden.push_back(item);
                }
            }
            ++it;
        }
        for (unsigned j=0;j<m_Hidden.size();++j) {
            m_ReviewList->takeItem(m_Hidden[j]);
        }
    } else {
        for (unsigned j=0;j<m_Hidden.size();++j) {
            m_ReviewList->insertItem(m_Hidden[j]);
        }
        m_Hidden.clear();
    }
}

/*!
    \fn Logmsg_impl::hideDepth(bool hide)
 */
void Logmsg_impl::hideDepth(bool ahide)
{
    m_DepthSelector->hideDepth(ahide);
}

#include "logmsg_impl.moc"
