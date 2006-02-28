/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht                                  *
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
#include "settings.h"

#include <ktextedit.h>
#include <kcombobox.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kapp.h>
#include <kconfigbase.h>
#include <kconfig.h>

#include <qvbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlistview.h>

#define MAX_MESSAGE_HISTORY 10

QValueList<QString> Logmsg_impl::sLogHistory = QValueList<QString>();
const char* Logmsg_impl::groupName = "logmsg_dialog_size";

unsigned int Logmsg_impl::smax_message_history = 0xFFFF;

Logmsg_impl::Logmsg_impl(QWidget *parent, const char *name)
    :LogmessageData(parent, name)
{
    m_LogEdit->setFocus();
    m_Reviewlabel->hide();
    m_ReviewList->hide();
}

Logmsg_impl::Logmsg_impl(const logActionEntries&_activatedList,
        const logActionEntries&_notActivatedList,
        QWidget *parent, const char *name)
    :LogmessageData(parent, name)
{
    m_LogEdit->setFocus();
    /// @todo own listcheckitem class with more information
    for (unsigned j = 0; j<_activatedList.count();++j) {
        QCheckListItem * item = new QCheckListItem(m_ReviewList,_activatedList[j].first,QCheckListItem::CheckBox);
        item->setText(1,_activatedList[j].second);
        item->setTristate(FALSE);
        item->setState(QCheckListItem::On);
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
    return m_LogEdit->text();
}


/*!
    \fn Logmsg_impl::isRecursive()const
 */
bool Logmsg_impl::isRecursive()const
{
    return m_RecursiveButton->isChecked();
}


/*!
    \fn Logmsg_impl::initHistory()
 */
void Logmsg_impl::initHistory()
{
    if (smax_message_history==0xFFFF) {
        smax_message_history = Settings::max_log_messages();
        KConfigGroup cs(Settings::self()->config(),"log_messages");
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
    kdDebug()<<"Max history: " << smax_message_history << endl;
    QValueList<QString>::const_iterator it;
    for (it=sLogHistory.begin();it!=sLogHistory.end();++it) {
        if ((*it).length()<=40) {
            m_LogHistory->insertItem((*it));
        } else {
            m_LogHistory->insertItem((*it).left(37)+"...");
        }
    }
}


/*!
    \fn Logmsg_impl::saveHistory()
 */
void Logmsg_impl::saveHistory()
{
    if (m_LogEdit->text().length()==0) return;
    /// @todo make static threadsafe
    QValueList<QString>::iterator it;
    if ( (it=sLogHistory.find(m_LogEdit->text()))!=sLogHistory.end()) {
        sLogHistory.erase(it);
    }
    sLogHistory.push_front(m_LogEdit->text());
    if (sLogHistory.size()>smax_message_history) {
        sLogHistory.erase(sLogHistory.fromLast());
    }
    KConfigGroup cs(Settings::self()->config(),"log_messages");
    for (unsigned int i = 0; i < sLogHistory.size();++i) {
        cs.writeEntry(QString("log_%0").arg(i),sLogHistory[i]);
    }
    cs.sync();
}

QString Logmsg_impl::getLogmessage(bool*ok,bool*rec,QWidget*parent,const char*name)
{
    bool _ok,_rec;
    QString msg("");

    Logmsg_impl*ptr=0;
    KDialogBase dlg(parent,name,true,i18n("Commit log"),
            KDialogBase::Ok|KDialogBase::Cancel,
            KDialogBase::Ok,true);
    QWidget* Dialog1Layout = dlg.makeVBoxMainWidget();

    ptr = new Logmsg_impl(Dialog1Layout);
    if (!rec) {
        ptr->m_RecursiveButton->hide();
    }
    ptr->initHistory();
    dlg.resize(dlg.configDialogSize(groupName));
    if (dlg.exec()!=QDialog::Accepted) {
        _ok = false;
        /* avoid compiler warnings */
        _rec = false;
    } else {
        _ok = true;
        _rec = ptr->isRecursive();
        msg=ptr->getMessage();
        ptr->saveHistory();
    }
    dlg.saveDialogSize(groupName,false);
    if (ok) *ok = _ok;
    if (rec) *rec = _rec;
    return msg;
}

QString Logmsg_impl::getLogmessage(const logActionEntries&_on,
            const logActionEntries&_off,
            bool*ok,QWidget*parent,const char*name)
{
    bool _ok;
    QString msg("");

    Logmsg_impl*ptr=0;
    KDialogBase dlg(parent,name,true,i18n("Commit log"),
            KDialogBase::Ok|KDialogBase::Cancel,
            KDialogBase::Ok,true);
    QWidget* Dialog1Layout = dlg.makeVBoxMainWidget();
    ptr = new Logmsg_impl(_on,_off,Dialog1Layout);
    ptr->m_RecursiveButton->hide();
    ptr->initHistory();
    dlg.resize(dlg.configDialogSize(groupName));
    if (dlg.exec()!=QDialog::Accepted) {
        _ok = false;
        /* avoid compiler warnings */
    } else {
        _ok = true;
        msg=ptr->getMessage();
        ptr->saveHistory();
    }
    dlg.saveDialogSize(groupName,false);
    if (ok) *ok = _ok;
    return msg;
}

/*!
    \fn Logmsg_impl::setRecCheckboxtext(const QString&what)
 */
void Logmsg_impl::setRecCheckboxtext(const QString&what)
{
    m_RecursiveButton->setText(what);
}

#include "logmsg_impl.moc"
