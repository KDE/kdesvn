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
#include "svnactions.h"
#include "kdesvnfilelist.h"
#include "filelistviewitem.h"
#include "rangeinput_impl.h"
#include "svnlogdlgimp.h"
#include "svncpp/client.hpp"
#include "svncpp/annotate_line.hpp"
#include "svncpp/context_listener.hpp"

#include <qstring.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <kdialog.h>
#include <ktextbrowser.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <qvaluelist.h>

#if 0
#include <khtml_part.h>
#include <khtmlview.h>
#endif

SvnActions::SvnActions(QObject *parent, const char *name)
 : QObject(parent, name),m_ParentList(0)
{
}

SvnActions::SvnActions(kdesvnfilelist *parent, const char *name)
 : QObject(parent, name),m_ParentList(parent)
{
}

SvnActions::~SvnActions()
{
}


#include "svnactions.moc"


/*!
    \fn SvnActions::slotMakeRangeLog()
 */
void SvnActions::slotMakeRangeLog()
{
    if (!m_ParentList) return;
    FileListViewItem*k = m_ParentList->singleSelected();
    if (!k) return;
    Rangeinput_impl rdlg;
    if (rdlg.exec()==QDialog::Accepted) {
        Rangeinput_impl::revision_range r = rdlg.getRange();
        makeLog(r.first,r.second,k);
    }
}

/*!
    \fn SvnActions::slotMakeLog()
 */
void SvnActions::slotMakeLog()
{
    if (!m_ParentList) return;
    FileListViewItem*k = m_ParentList->singleSelected();
    if (!k) return;
    svn::Revision start(svn::Revision::START);
    svn::Revision end(svn::Revision::HEAD);
    makeLog(start,end,k);
}

/*!
    \fn SvnActions::makeLog(svn::Revision start,svn::Revision end,FileListViewItem*k)
 */
void SvnActions::makeLog(svn::Revision start,svn::Revision end,FileListViewItem*k)
{
    const svn::LogEntries * logs;
    QString ex;
    try {
        logs = m_ParentList->svnclient()->log(k->fullName().local8Bit(),start,end,true);
    } catch (svn::ClientException e) {
        ex = QString::fromLocal8Bit(e.message());
        emit clientException(ex);
        return;
    }
    if (!logs) {
        ex = I18N_NOOP("Got no logs");
        emit clientException(ex);
        return;
    }
    SvnLogDlgImp disp;
    disp.dispLog(logs);
    disp.exec();
    delete logs;
}


/*!
    \fn SvnActions::slotBlame()
 */
void SvnActions::slotBlame()
{
    if (!m_ParentList) return;
    FileListViewItem*k = m_ParentList->singleSelected();
    if (!k) return;
    svn::Revision start(svn::Revision::START);
    svn::Revision end(svn::Revision::HEAD);
    makeBlame(start,end,k);
}

template<class T> QDialog* SvnActions::createDialog(T**ptr,const QString&_head)
{
    KDialog * dlg = new KDialog();
    if (!dlg) return dlg;
    dlg->setCaption(_head);
    QHBoxLayout* Dialog1Layout;

    QVBoxLayout* blayout;
    QSpacerItem* Spacer1;
    Dialog1Layout = new QHBoxLayout( dlg, 2, 2, "Dialog1Layout");
    *ptr = new T(dlg);

    Dialog1Layout->addWidget( *ptr );

    /* button */
    blayout = new QVBoxLayout( 0, 0, 6, "blayout");
    QPushButton*buttonOk = new QPushButton( dlg, "buttonOk" );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    blayout->addWidget( buttonOk );
    buttonOk->setText( i18n( "&Close" ) );
    Spacer1 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    blayout->addItem( Spacer1 );
    Dialog1Layout->addLayout(blayout);
    connect( buttonOk, SIGNAL( clicked() ), dlg, SLOT( accept() ) );

    dlg->resize( QSize(640,480).expandedTo(dlg->minimumSizeHint()) );
    return dlg;
}

/*!
    \fn SvnActions::makeBlame(svn::Revision start, svn::Revision end, FileListViewItem*k)
 */
void SvnActions::makeBlame(svn::Revision start, svn::Revision end, FileListViewItem*k)
{
    svn::AnnotatedFile * blame;
    QString ex;
    svn::Path p(k->fullName().local8Bit());

    try {
        blame = m_ParentList->svnclient()->annotate(p,start,end);
    } catch (svn::ClientException e) {
        ex = QString::fromLocal8Bit(e.message());
        emit clientException(ex);
        return;
    } catch (...) {
        ex = I18N_NOOP("Error getting blame");
        emit clientException(ex);
        return;
    }
    if (!blame) {
        ex = I18N_NOOP("Got no annotate");
        emit clientException(ex);
        return;
    }
    svn::AnnotatedFile::const_iterator bit;
    static QString rowb = "<tr><td>";
    static QString rowc = "<tr bgcolor=\"#EEEEEE\"><td>";
    static QString rows = "</td><td>";
    static QString rowe = "</td></tr>\n";
    QString text = "<html><table>"+rowb+"Rev"+rows+I18N_NOOP("Author")+rows+I18N_NOOP("Line")+rows+"&nbsp;"+rowe;
    bool second = false;
    QString codetext = "";
    for (bit=blame->begin();bit!=blame->end();++bit) {
        codetext = bit->line().c_str();
        codetext.replace(" ","&nbsp;");
        codetext.replace("\"","&quot;");
        codetext.replace("<","&lt;");
        codetext.replace(">","&gt;");
        codetext.replace("\t","&nbsp;&nbsp;&nbsp;&nbsp;");
        text+=(second?rowb:rowc)+QString("%1").arg(bit->revision())+
            rows+QString("%1").arg(bit->author().c_str())+rows+
            QString("%1").arg(bit->lineNumber())+rows+
            QString("<code>%1</code>").arg(codetext)+rowe;
            second = !second;
    }
    text += "</table></html>";
    KTextBrowser*ptr;
    QDialog*dlg = createDialog(&ptr,QString(I18N_NOOP("Blame %1")).arg(k->text(0)));
    if (dlg) {
        ptr->setText(text);
        dlg->exec();
        delete dlg;
    }
    delete blame;
}

QDateTime SvnActions::apr2qttime(apr_time_t atime)
{
    QDateTime t;t.setTime_t(atime/(1000*1000),Qt::UTC);
    return t;
}

/*!
    \fn SvnActions::slotMakeRangeBlame()
 */
void SvnActions::slotRangeBlame()
{
    return;
    if (!m_ParentList) return;
    FileListViewItem*k = m_ParentList->singleSelected();
    if (!k) return;
    Rangeinput_impl rdlg;
    if (rdlg.exec()==QDialog::Accepted) {
        Rangeinput_impl::revision_range r = rdlg.getRange();
        makeBlame(r.first,r.second,k);
    }
}

void SvnActions::makeCat(svn::Revision start, FileListViewItem*k)
{
    std::string content;
    QString ex;
    svn::Path p(k->fullName().local8Bit());
    try {
        content = m_ParentList->svnclient()->cat(p,start);
    } catch (svn::ClientException e) {
        ex = QString::fromLocal8Bit(e.message());
        emit clientException(ex);
        return;
    } catch (...) {
        ex = I18N_NOOP("Error getting content");
        emit clientException(ex);
        return;
    }
    if (content.size()==0) {
        emit clientException(I18N_NOOP("Got no content"));
        return;
    }
    KTextBrowser*ptr;
    QDialog*dlg = createDialog(&ptr,QString(I18N_NOOP("Content of %1")).arg(k->text(0)));
    if (dlg) {
        ptr->setFont(KGlobalSettings::fixedFont());
        ptr->setWordWrap(QTextEdit::NoWrap);
        ptr->setText(QString::fromLocal8Bit(content.c_str()));
        dlg->exec();
        delete dlg;
    }
}

void SvnActions::slotCat()
{
    if (!m_ParentList) return;
    FileListViewItem*k = m_ParentList->singleSelected();
    if (!k) return;
    makeCat(svn::Revision::HEAD, k);
}

void SvnActions::slotMkdir()
{
    if (!m_ParentList) return;
    FileListViewItem*k = m_ParentList->singleSelected();
    QString parentDir,ex;
    if (k) {
        if (!k->isDir()) {
            KMessageBox::sorry(0,I18N_NOOP("May not make subdirs of a file"));
            return;
        }
        parentDir=k->fullName();
    } else {
        parentDir=m_ParentList->baseUri();
    }
    bool isOk=false;
    ex = KInputDialog::getText(I18N_NOOP("New Dir"),I18N_NOOP("Enter (sub-)directory name:"),QString::null,&isOk);
    if (!isOk) {
        return;
    }
    svn::Path target(parentDir.local8Bit());
    target.addComponent(ex.local8Bit());
    ex = "";
    std::string message;

    QString logMessage="";
    if (!m_ParentList->isLocal()) {
        isOk = false;
        logMessage = KInputDialog::getMultiLineText(I18N_NOOP("Logmessage"),I18N_NOOP("Enter a logmessage"),QString::null,&isOk);
        if (!isOk) {
            return;
        }
    }

    try {
        m_ParentList->svnclient()->mkdir(target,logMessage.local8Bit());
    }catch (svn::ClientException e) {
        ex = QString::fromLocal8Bit(e.message());
        emit clientException(ex);
        return;
    }

    ex = target.path().c_str();
    emit dirAdded(ex,k);
}

void SvnActions::slotInfo()
{
    if (!m_ParentList) return;
    QPtrList<FileListViewItem> lst = m_ParentList->allSelected();
    QValueList<svn::Entry> entries;
    QString ex;
    try {
        if (lst.count()==0) {
            entries.push_back(m_ParentList->svnclient()->info(m_ParentList->baseUri().local8Bit()));
        } else {
            FileListViewItem*item;
            for (item=lst.first();item;item=lst.next()) {
                entries.push_back(m_ParentList->svnclient()->info(item->fullName().local8Bit()));
            }
        }
    } catch (svn::ClientException e) {
        ex = QString::fromLocal8Bit(e.message());
        emit clientException(ex);
        return;
    }
    QString text = "<html>";
    QValueList<svn::Entry>::const_iterator it;
    QString rb = "<tr><td align=\"right\"><b>";
    QString re = "</td></tr>\n";
    QString cs = "</b>:</td><td>";
    for (it=entries.begin();it!=entries.end();++it) {
        text+="<p><table>";
        if ((*it).name()&&strlen((*it).name())) {
            text+=rb+I18N_NOOP("Name")+cs+QString((*it).name())+re;
        }
        text+=rb+I18N_NOOP("URL")+cs+QString((*it).url())+re;
        if ((*it).repos()&&strlen((*it).repos())) {
            text+=rb+I18N_NOOP("Canonical repository url")+cs+QString((*it).repos())+re;
        }
        text+=rb+I18N_NOOP("Type")+cs;
        switch ((*it).kind()) {
        case svn_node_none:
            text+=i18n("Absent");
            break;
        case svn_node_file:
            text+=i18n("File");
            break;
        case svn_node_dir:
            text+=i18n("Directory");
            break;
        case svn_node_unknown:
        default:
            text+=i18n("Unknown");
            break;
        }
        text+=re;
        text+=rb+I18N_NOOP("Schedule")+cs;
        switch ((*it).schedule()) {
        case svn_wc_schedule_normal:
            text+=i18n("Normal");
            break;
        case svn_wc_schedule_add:
            text+=i18n("Addition");
            break;
        case svn_wc_schedule_delete:
            text+=i18n("Deletion");
            break;
        case svn_wc_schedule_replace:
            text+=i18n("Replace");
            break;
        default:
            text+=i18n("Unknow");
            break;
        }
        text+=re;
        text+=rb+I18N_NOOP("UUID")+cs+QString((*it).uuid())+re;
        text+=rb+I18N_NOOP("Last author")+cs+QString((*it).cmtAuthor())+re;
        text+=rb+I18N_NOOP("Last changed")+cs+apr2qttime((*it).cmtDate()).toString(Qt::LocalDate)+re;
        text+=rb+I18N_NOOP("Property last changed")+cs+apr2qttime((*it).propTime()).toString(Qt::LocalDate)+re;
        text+=rb+I18N_NOOP("Last revision")+cs+QString("%1").arg((*it).cmtRev())+re;
        if ((*it).conflictNew()&&strlen((*it).conflictNew())) {
            text+=rb+I18N_NOOP("New version of conflicted file")+cs+QString((*it).conflictNew())+re;
        }
        if ((*it).conflictOld()&&strlen((*it).conflictOld())) {
            text+=rb+I18N_NOOP("Old version of conflicted file")+cs+QString((*it).conflictOld())+re;
        }
        if ((*it).conflictWrk()&&strlen((*it).conflictWrk())) {
            text+=rb+I18N_NOOP("Working version of conflicted file")+cs+QString((*it).conflictWrk())+re;
        }
        if ((*it).copyfromUrl()&&strlen((*it).copyfromUrl())) {
            text+=rb+I18N_NOOP("Copy from URL")+cs+QString((*it).copyfromUrl())+re;
        }

        text+="</table></p>\n";
    }
    text+="</html>";
    KTextBrowser*ptr;
    QDialog*dlg = createDialog(&ptr,QString(I18N_NOOP("Infolist")));
    if (dlg) {
        ptr->setText(text);
        dlg->exec();
        delete dlg;
    }
}
