/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
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
#include "commitmsg_impl.h"
#include "models/commitmodelhelper.h"
#include "models/commitmodel.h"
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
#include <kurlrequesterdialog.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <kfile.h>
#include <kurlrequester.h>
#include <KVBox>

#include <QStringList>
#include <QSortFilterProxyModel>
#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QWidget>
#include <QPushButton>
#include <QFile>

#define MAX_MESSAGE_HISTORY 10

QStringList Commitmsg_impl::sLogHistory = QStringList();
QString Commitmsg_impl::sLastMessage;
const QString Commitmsg_impl::groupName("logmsg_dlg_size");

int Commitmsg_impl::smax_message_history = 0xFFFF;

Commitmsg_impl::Commitmsg_impl(QWidget *parent)
    :QWidget(parent),CommitMessage()
{
    setupUi(this);
    m_CurrentModel=0;
    m_SortModel=0;
    m_LogEdit->setFocus();
    m_Reviewlabel->hide();
    m_hidden=true;
    hideButtons(true);
    m_MainSplitter->insertWidget(0,m_EditFrame);
    delete m_ReviewFrame;
    m_Reviewlabel=0;
    m_MarkUnversioned=0;
    m_UnmarkUnversioned=0;
    m_DiffItem=0;
}

Commitmsg_impl::Commitmsg_impl(const svn::CommitItemList&_items,QWidget *parent)
    :QWidget(parent),CommitMessage()
{
    setupUi(this);
    m_CurrentModel=0;
    m_SortModel=0;
    m_LogEdit->setFocus();
    hideButtons(true);
    if (_items.count()>0) {
        m_CurrentModel = new CommitModel(_items);
        setupModel();
        m_hidden=false;
    } else {
        m_Reviewlabel->hide();
        m_CommitItemTree->hide();
        m_hidden=true;
    }
    checkSplitterSize();
}

Commitmsg_impl::Commitmsg_impl(const QMap<QString,QString>&_items,QWidget *parent)
    :QWidget(parent),CommitMessage()
{
    setupUi(this);
    m_CurrentModel=0;
    m_SortModel=0;
    m_LogEdit->setFocus();
    hideButtons(true);
    if (_items.count()>0) {
        m_CurrentModel = new CommitModel(_items);
        setupModel();
        m_hidden=false;
    } else {
        m_Reviewlabel->hide();
        m_CommitItemTree->hide();
        m_hidden=true;
    }
    checkSplitterSize();
}

Commitmsg_impl::Commitmsg_impl(const CommitActionEntries&_activatedList,
        const CommitActionEntries&_notActivatedList,
        QWidget *parent)
    :QWidget(parent),CommitMessage()
{
    setupUi(this);
    m_CurrentModel=0;
    m_SortModel=0;
    m_LogEdit->setFocus();
    m_hidden=false;

    m_CurrentModel = new CommitModelCheckitem(_activatedList,_notActivatedList);
    setupModel();

    m_HideNewItems->setChecked(Kdesvnsettings::commit_hide_new());
    checkSplitterSize();
}

Commitmsg_impl::~Commitmsg_impl()
{
    QList<int> list = m_MainSplitter->sizes();
    if (!m_hidden && list.count()==2) {
        Kdesvnsettings::setCommit_splitter_height(list);
        Kdesvnsettings::self()->writeConfig();
    }
    delete m_CurrentModel;
    delete m_SortModel;
}

void Commitmsg_impl::setupModel()
{
    m_SortModel = new QSortFilterProxyModel(m_CommitItemTree);
    m_CommitItemTree->setModel(m_SortModel);
    m_SortModel->setSourceModel(m_CurrentModel);

    m_CommitItemTree->resizeColumnToContents(m_CurrentModel->ItemColumn());
    m_CommitItemTree->resizeColumnToContents(m_CurrentModel->ActionColumn());

    m_SortModel->setSortCaseSensitivity(Kdesvnsettings::case_sensitive_sort()?Qt::CaseSensitive:Qt::CaseInsensitive);
}

void Commitmsg_impl::checkSplitterSize()
{
    QList<int> list = Kdesvnsettings::commit_splitter_height();
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

void Commitmsg_impl::slotHistoryActivated(int number)
{
    if (number < 1||number>sLogHistory.size()) {
        m_LogEdit->setText("");
    } else {
        m_LogEdit->setText(sLogHistory[number-1]);
    }
}

/*!
    \fn Commitmsg_impl::getMessage()const
 */
QString Commitmsg_impl::getMessage()const
{
    return m_LogEdit->toPlainText();
}


/*!
    \fn Commitmsg_impl::isRecursive()const
 */
svn::Depth Commitmsg_impl::getDepth()const
{
    return m_DepthSelector->getDepth();
}

void Commitmsg_impl::keepsLocks(bool keeps_lock)
{
    if (keeps_lock) {
        m_keepLocksButton->show();
    } else {
        m_keepLocksButton->hide();
    }
}

/*!
    \fn Commitmsg_impl::isRecursive()const
 */
bool Commitmsg_impl::isKeeplocks()const
{
    return m_keepLocksButton->isChecked();
}


/*!
    \fn Commitmsg_impl::initHistory()
 */
void Commitmsg_impl::initHistory()
{
    if (smax_message_history==0xFFFF) {
        smax_message_history = Kdesvnsettings::max_log_messages();
        KConfigGroup cs(Kdesvnsettings::self()->config(),"log_messages");
        QString s;
        int current = 0;
        QString key = QString("log_%0").arg(current);
        s = cs.readEntry(key,QString());
        while (!s.isNull()) {
            if (current<smax_message_history) {
                sLogHistory.push_back(s);
            } else {
                cs.deleteEntry(key);
            }
            ++current;
            key = QString("log_%0").arg(current);
            s = cs.readEntry(key,QString());
        }
    }
    QStringList::const_iterator it;
    for (it=sLogHistory.begin();it!=sLogHistory.end();++it) {
        if ((*it).length()<=40) {
            m_LogHistory->addItem((*it));
        } else {
            m_LogHistory->addItem((*it).left(37)+"...");
        }
    }
    if (sLastMessage.length()>0) {
        m_LogEdit->setText(sLastMessage);
        sLastMessage.clear();
    }
}


/*!
    \fn Commitmsg_impl::saveHistory()
 */
void Commitmsg_impl::saveHistory(bool canceld)
{
    QString _text = m_LogEdit->toPlainText();
    if (_text.length()==0||_text.length()>512) return;
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
        for (int i = 0; i < sLogHistory.size();++i) {
            cs.writeEntry(QString("log_%0").arg(i),sLogHistory[i]);
        }
        cs.sync();
    } else {
        sLastMessage=_text;
    }
}

QString Commitmsg_impl::getLogmessage(bool*ok,svn::Depth*rec,bool*keep_locks,QWidget*parent)
{
    bool _ok,_keep_locks;
    svn::Depth _depth = svn::DepthUnknown;
    QString msg("");

    Commitmsg_impl*ptr=0;
    KDialog dlg(parent);
    dlg.setCaption(i18n("Commit log"));
    dlg.setModal(true);
    dlg.setButtons(KDialog::Ok | KDialog::Cancel);
    dlg.setDefaultButton(KDialog::Ok);
    dlg.showButtonSeparator(true);

    KVBox *Dialog1Layout = new KVBox(&dlg);
    dlg.setMainWidget(Dialog1Layout);

    ptr = new Commitmsg_impl(Dialog1Layout);
    if (!rec) {
        ptr->m_DepthSelector->hide();
    }
    if (!keep_locks) {
        ptr->m_keepLocksButton->hide();
    }
    ptr->initHistory();
    KConfigGroup _k(Kdesvnsettings::self()->config(),groupName);
    dlg.restoreDialogSize(_k);
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
    dlg.saveDialogSize(_k);

    if (ok) *ok = _ok;
    if (rec) *rec = _depth;
    return msg;
}

QString Commitmsg_impl::getLogmessage(const svn::CommitItemList&items,bool*ok,svn::Depth*rec,bool*keep_locks,QWidget*parent)
{
    bool _ok,_keep_locks;
    svn::Depth _depth = svn::DepthUnknown;
    QString msg("");

    Commitmsg_impl*ptr=0;
    KDialog dlg(parent);
    dlg.setCaption(i18n("Commit log"));
    dlg.setModal(true);
    dlg.setButtons(KDialog::Ok | KDialog::Cancel);
    dlg.setDefaultButton(KDialog::Ok);
    dlg.showButtonSeparator(true);

    KVBox *Dialog1Layout = new KVBox(&dlg);
    dlg.setMainWidget(Dialog1Layout);

    ptr = new Commitmsg_impl(items,Dialog1Layout);
    if (!rec) {
        ptr->m_DepthSelector->hide();
    }
    if (!keep_locks) {
        ptr->m_keepLocksButton->hide();
    }

    ptr->initHistory();
    KConfigGroup _k(Kdesvnsettings::self()->config(),groupName);
    dlg.restoreDialogSize(_k);
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
    dlg.saveDialogSize(_k);

    if (ok) *ok = _ok;
    if (rec) *rec = _depth;
    if (keep_locks) *keep_locks = _keep_locks;
    return msg;
}

QString Commitmsg_impl::getLogmessage(const QMap<QString,QString>&items,
    bool*ok,svn::Depth*rec,bool*keep_locks,QWidget*parent)
{
    bool _ok,_rec,_keep_locks;
    svn::Depth _depth = svn::DepthUnknown;
    QString msg("");

    Commitmsg_impl*ptr=0;
    KDialog dlg(parent);
    dlg.setCaption(i18n("Commit log"));
    dlg.setModal(true);
    dlg.setButtons(KDialog::Ok | KDialog::Cancel);
    dlg.setDefaultButton(KDialog::Ok);
    dlg.showButtonSeparator(true);

    KVBox *Dialog1Layout = new KVBox(&dlg);
    dlg.setMainWidget(Dialog1Layout);

    ptr = new Commitmsg_impl(items,Dialog1Layout);
    if (!rec) {
        ptr->m_DepthSelector->hide();
    }
    if (!keep_locks) {
        ptr->m_keepLocksButton->hide();
    }
    ptr->initHistory();
    KConfigGroup _k(Kdesvnsettings::self()->config(),groupName);
    dlg.restoreDialogSize(_k);

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
    dlg.saveDialogSize(_k);

    if (ok) *ok = _ok;
    if (rec) *rec = _depth;
    if (keep_locks) *keep_locks = _keep_locks;
    return msg;
}

QString Commitmsg_impl::getLogmessage(const CommitActionEntries&_on,
            const CommitActionEntries&_off,
            QObject*callback,
            CommitActionEntries&_result,
            bool*ok,bool*keep_locks,QWidget*parent)
{
    bool _ok,_keep_locks;
    QString msg("");

    Commitmsg_impl*ptr=0;
    KDialog dlg(parent);
    dlg.setCaption(i18n("Commit log"));
    dlg.setModal(true);
    dlg.setButtons(KDialog::Ok | KDialog::Cancel);
    dlg.setDefaultButton(KDialog::Ok);
    dlg.showButtonSeparator(true);

    KVBox *Dialog1Layout = new KVBox(&dlg);
    dlg.setMainWidget(Dialog1Layout);

    ptr = new Commitmsg_impl(_on,_off,Dialog1Layout);
    ptr->m_DepthSelector->hide();
    if (!keep_locks) {
        ptr->m_keepLocksButton->hide();
    }
    ptr->initHistory();
    if (callback)
    {
        connect(ptr,SIGNAL(makeDiff(const QString&,const svn::Revision&,const QString&,const svn::Revision&,QWidget*)),
                callback,SLOT(makeDiff(const QString&,const svn::Revision&,const QString&,const svn::Revision&,QWidget*)));
        connect(ptr,SIGNAL(sigRevertItem(const QStringList&,bool)),
                callback,SLOT(slotRevertItems(const QStringList&,bool)));
        connect(callback,SIGNAL(sigItemsReverted(const QStringList&)),
                ptr,SLOT(slotItemReverted(const QStringList&)));
    }
    KConfigGroup _k(Kdesvnsettings::self()->config(),groupName);
    dlg.restoreDialogSize(_k);
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
    dlg.saveDialogSize(_k);
    if (ok) *ok = _ok;
    _result = ptr->checkedEntries();
    if (keep_locks) *keep_locks = _keep_locks;
    return msg;
}

/*!
    \fn Commitmsg_impl::setRecCheckboxtext(const QString&what)
 */
void Commitmsg_impl::addItemWidget(QWidget*aWidget)
{
    m_DepthSelector->addItemWidget(aWidget);
}

CommitActionEntries Commitmsg_impl::checkedEntries()
{
    if (m_CurrentModel) {
        return m_CurrentModel->checkedEntries();
    }
    return CommitActionEntries();
}

void Commitmsg_impl::slotUnmarkUnversioned()
{
    markUnversioned(false);
}

void Commitmsg_impl::slotMarkUnversioned()
{
    markUnversioned(true);
}

void Commitmsg_impl::slotDiffSelected()
{
    CommitModelNodePtr ptr = currentCommitItem();
    if (!ptr) {
        return;
    }
    QString what=ptr->actionEntry().name();
    emit makeDiff(what,svn::Revision::BASE,what,svn::Revision::WORKING,parentWidget());
}

void Commitmsg_impl::slotRevertSelected()
{
    CommitModelNodePtr ptr = currentCommitItem();
    if (!ptr) {
        return;
    }
    QStringList what(ptr->actionEntry().name());
    emit sigRevertItem(what,false);
}

CommitModelNodePtr Commitmsg_impl::currentCommitItem(int column)
{
    CommitModelNodePtr res;
    if (!m_CurrentModel) {
        return res;
    }
    QModelIndexList _mi = m_CommitItemTree->selectionModel()->selectedRows(column);
    if (_mi.count()<1) {
        return res;
    }
    QModelIndex ind = m_SortModel->mapToSource(_mi[0]);
    if (ind.isValid()) {
        res = m_CurrentModel->node(ind);
    }
    return res;
}

void Commitmsg_impl::hideKeepsLock(bool how)
{
    m_keepLocksButton->setVisible(!how);
}

void Commitmsg_impl::hideButtons(bool how)
{
    if (!m_MarkUnversioned)return;
    if (how)
    {
        m_MarkUnversioned->hide();
        m_UnmarkUnversioned->hide();
        m_DiffItem->hide();
        m_HideNewItems->hide();
        m_SelectAllButton->hide();
    }
    else
    {
        m_MarkUnversioned->show();
        m_UnmarkUnversioned->show();
        m_DiffItem->show();
        m_HideNewItems->show();
        m_SelectAllButton->show();
    }
}

/*!
    \fn Commitmsg_impl::markUnversioned(bool mark)
 */
void Commitmsg_impl::markUnversioned(bool mark)
{
    if (!m_CurrentModel) {
        return;
    }
    m_CurrentModel->markItems(mark,CommitActionEntry::ADD_COMMIT);
}

void Commitmsg_impl::slotSelectAll()
{
    if (!m_CurrentModel) {
        return;
    }
    m_CurrentModel->markItems(true,CommitActionEntry::ALL);
}

void Commitmsg_impl::hideNewItems(bool how)
{
    if (!m_CurrentModel) {
        return;
    }
    Kdesvnsettings::setCommit_hide_new(how);
    m_CurrentModel->hideItems(how,CommitActionEntry::ADD_COMMIT);
}

/*!
    \fn Commitmsg_impl::hideDepth(bool hide)
 */
void Commitmsg_impl::hideDepth(bool ahide)
{
    m_DepthSelector->hideDepth(ahide);
}

void Commitmsg_impl::insertFile(const QString&fname)
{
    QFile ifs(fname);
    if (ifs.open(QIODevice::ReadOnly)) {
        QTextStream ts(&ifs);
        QString _content = ts.readAll();
        m_LogEdit->textCursor ().insertText(_content);
    }
}

void Commitmsg_impl::insertFile()
{
    QString head = i18n("Select textfile for insert");
    KUrlRequesterDialog dlg(QString(),head,this);
    dlg.setCaption(head);
    KFile::Mode mode = static_cast<KFile::Mode>(KFile::File);
    dlg.urlRequester()->setMode(mode);
    dlg.urlRequester()->setWindowTitle(head);

    if (dlg.exec()!=QDialog::Accepted) {
        return;
    }
    KUrl _url = dlg.selectedUrl();
    if (_url.isEmpty() || !_url.isValid()) {
        return;
    }
    if (_url.isLocalFile()) {
        insertFile(_url.path());
    } else {
        QString tmpFile;
        if( KIO::NetAccess::download(_url, tmpFile, this) ) {
            insertFile( tmpFile );
            KIO::NetAccess::removeTempFile( tmpFile );
        } else {
            KMessageBox::error(this, KIO::NetAccess::lastErrorString() );
        }
    }
}


/*!
    \fn Commitmsg_impl::slotItemReverted(const QStringList&)
 */
void Commitmsg_impl::slotItemReverted(const QStringList&items)
{
    if (!m_CurrentModel) {
        return;
    }
    m_CurrentModel->removeEntries(items);
}

void Commitmsg_impl::slotItemDoubleClicked(const QModelIndex&index)
{
    Q_UNUSED(index);
    slotDiffSelected();
}

#include "commitmsg_impl.moc"
