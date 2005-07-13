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

#include <qstring.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <kdialog.h>
#include <ktextbrowser.h>
#include <klocale.h>
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
    try {
        logs = m_ParentList->svnclient()->log(k->fullName().ascii(),start,end,true);
    } catch (svn::ClientException e) {
        QString ex = e.message();
        emit clientException(ex);
        return;
    }
    if (!logs) {
        qDebug("No logs");
        return;
    }
    SvnLogDlgImp disp;
    disp.dispLog(logs);
    disp.exec();
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

template<class T> QDialog* SvnActions::createDialog(T**ptr)
{
    KDialog * dlg = new KDialog();
    if (!dlg) return dlg;
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
    try {
        blame = m_ParentList->svnclient()->annotate(k->fullName().ascii(),start,end);
    } catch (svn::ClientException e) {
        ex = QString::fromLatin1(e.message());
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
    static QString rowc = "<tr bgcolor=\"#DDDDDD\"><td>";
    static QString rows = "</td><td>";
    static QString rowe = "</td></tr>\n";
    QString text = "<html><table>"+rowb+"Rev"+rows+I18N_NOOP("Author")+rows+"Line"+rows+"&nbsp;"+rowe;
    bool second = false;
    for (bit=blame->begin();bit!=blame->end();++bit) {
        text+=(second?rowb:rowc)+QString("%1").arg(bit->revision())+
            rows+QString("%1").arg(bit->author().c_str())+rows+
            QString("%1").arg(bit->lineNumber())+rows+
            QString("<pre>%1</pre>").arg(bit->line().c_str())+rowe;
            second = !second;
    }
    text += "</table></html>";
    KTextBrowser*ptr;
    QDialog*dlg = createDialog(&ptr);
    if (dlg) {
        ptr->setText(text);
        dlg->exec();
        delete dlg;
    }
}
