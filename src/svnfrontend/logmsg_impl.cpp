/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht   *
 *   ral@alwins-world.de   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "logmsg_impl.h"
#include <ktextedit.h>
#include <kcombobox.h>
#include <qcheckbox.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <qvbox.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kapp.h>
#include <kconfigbase.h>
#include <kconfig.h>

QValueList<QString> Logmsg_impl::sLogHistory = QValueList<QString>();
const char* Logmsg_impl::groupName = "logmsg_dialog_size";

Logmsg_impl::Logmsg_impl(QWidget *parent, const char *name)
    :LogmessageData(parent, name)
{
}

void Logmsg_impl::slotHistoryActivated(const QString&aMessage)
{
    m_LogEdit->setText(aMessage);
}


#include "logmsg_impl.moc"


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
    /// @todo make static threadsafe
    QValueList<QString>::const_iterator it;
    for (it=sLogHistory.begin();it!=sLogHistory.end();++it) {
        m_LogHistory->insertItem((*it));
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
    if (sLogHistory.size()>10) {
        sLogHistory.erase(sLogHistory.fromLast());
    }
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


/*!
    \fn Logmsg_impl::setRecCheckboxtext(const QString&what)
 */
void Logmsg_impl::setRecCheckboxtext(const QString&what)
{
    m_RecursiveButton->setText(what);
}
