/***************************************************************************
 *   Copyright (C) 2007 by Rajko Albrecht                                  *
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

#include "diffbrowser.h"
#include "diffbrowserdata.h"
#include "src/settings/kdesvnsettings.h"

#include <kglobalsettings.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <klocale.h>

#include <qfont.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <QKeyEvent>
#include <qtextcodec.h>
#include <KStandardGuiItem>
#include <KFind>

/*!
    \fn DiffBrowser::DiffBrowser(QWidget*parent=0,const char*name=0)
 */
DiffBrowser::DiffBrowser(QWidget*parent,const char*name)
    : KTextBrowser( parent, name)
{
    setTextFormat(Qt::PlainText);
    setFont(KGlobalSettings::fixedFont());
    m_Data = new DiffBrowserData;

//     setWordWrap(Q3TextEdit::NoWrap);
    setLineWrapMode(QTextEdit::NoWrap);
    m_Data->m_Syntax = new DiffSyntax(this);
    QToolTip::add(this,i18n("Ctrl-F for search, F3 or Shift-F3 for search again."));
    Q3WhatsThis::add(this,i18n("<b>Display differences between files</b><p>You may search inside text with Ctrl-F.</p><p>F3 for search forward again, Shift-F3 for search backward again.</p><p>You may save the (original) output with Ctrl-S.</p>"));
    setFocus();
}

/*!
    \fn DiffBrowser::~DiffBrowser()
 */
 DiffBrowser::~DiffBrowser()
{
    delete m_Data;
}

void DiffBrowser::setText(const QString&aText)
{
    m_Data->m_content.setRawData(aText.local8Bit(),aText.local8Bit().size());
    KTextBrowser::setText(aText);
//     setCursorPosition(0,0);
    moveCursor(QTextCursor::Start);
}

void DiffBrowser::setText(const QByteArray&aText)
{
    m_Data->m_content=aText;
    printContent();
//     setCursorPosition(0,0);
    moveCursor(QTextCursor::Start);
}

void DiffBrowser::printContent()
{
    QTextCodec * cc = QTextCodec::codecForName(Kdesvnsettings::locale_for_diff());
    if (!cc) {
        KTextBrowser::setText(QString::fromLocal8Bit(m_Data->m_content,m_Data->m_content.size()));
    } else {
        KTextBrowser::setText(cc->toUnicode(m_Data->m_content,m_Data->m_content.size()));
    }
}

/*!
    \fn DiffBrowser::saveDiff()
 */
void DiffBrowser::saveDiff()
{
//     QString saveTo = KFileDialog::getSaveFileName(QString::null,"text/x-diff");
    QString saveTo = KFileDialog::getSaveFileName(KUrl(),
                                                   i18n("Diff files (*.diff);;All Files(*)"),
                                                   this/*,
                                                   i18n("Save diff to")*/
                                                   );
    if (saveTo.isEmpty()) {
        return;
    }
    QFile tfile(saveTo);
    if (tfile.exists()) {
        if (KMessageBox::warningYesNo(KApplication::activeModalWidget(),
                                     i18n("File %1 exists - overwrite?").arg(saveTo))
            !=KMessageBox::Yes) {
            return;
        }
    }
    tfile.open(QIODevice::Truncate|QIODevice::WriteOnly|QIODevice::Unbuffered);
    QDataStream stream( &tfile );
    stream.writeRawBytes(m_Data->m_content.data(),m_Data->m_content.size());
}

void DiffBrowser::keyPressEvent(QKeyEvent*ev)
{
    if ( ev->key() == Qt::Key_Return) {
        ev->ignore();
        return;
    }
    if (ev->key() == Qt::Key_F3) {
        if (ev->state() == Qt::ShiftModifier) {
            searchagainback_slot();
        } else {
            searchagain_slot();
        }
    } else if (ev->key()==Qt::Key_F && ev->state() == Qt::ControlModifier) {
        startSearch();
    } else if (ev->key()==Qt::Key_S && ev->state() == Qt::ControlModifier) {
        saveDiff();
    } else {
        KTextBrowser::keyPressEvent(ev);
    }
}

void DiffBrowser::startSearch()
{
    if( !m_Data->srchdialog ) {
//         m_Data->srchdialog = new KEdFind( this, "searchdialog", false);
        m_Data->srchdialog = new KFindDialog(this);
        m_Data->srchdialog->setButtons( KDialog::User1|KDialog::Close );
        m_Data->srchdialog->setButtonGuiItem( KDialog::User1, KStandardGuiItem::find() );
        m_Data->srchdialog->setDefaultButton(KDialog::User1);
        m_Data->srchdialog->setSupportsWholeWordsFind(false);
        m_Data->srchdialog->setHasCursor(false);
        m_Data->srchdialog->setHasSelection(false);
        m_Data->srchdialog->setSupportsRegularExpressionFind(false);

        connect(m_Data->srchdialog,SIGNAL(search()),this,SLOT(search_slot()));
        connect(m_Data->srchdialog,SIGNAL(done()),this,SLOT(searchdone_slot()));
    }
//     QString _st = m_Data->srchdialog->getText();
    QString _st = m_Data->srchdialog->pattern();
//     m_Data->srchdialog->setText(_st.isEmpty() ? m_Data->pattern : _st);
    m_Data->srchdialog->setPattern(_st.isEmpty() ? m_Data->pattern : _st);
    m_Data->srchdialog->show();
// KDE4 port - pv - what is was for?     m_Data->srchdialog->result();
}

/*!
    \fn DiffBrowser::search_slot()
 */
void DiffBrowser::search_slot()
{
    if( !m_Data->srchdialog ) {
        return;
    }
//     QString to_find_string = m_Data->srchdialog->getText();
//     doSearch(to_find_string,m_Data->srchdialog->case_sensitive(),m_Data->srchdialog->get_direction());
    QString to_find_string = m_Data->srchdialog->pattern();
    doSearch(to_find_string,
              m_Data->srchdialog->options() & Qt::CaseSensitive,
              m_Data->srchdialog->options() & KFind::FindBackwards ? KFind::FindBackwards : KFind::FindIncremental);
}

void DiffBrowser::doSearch(const QString&to_find_string,bool case_sensitive,bool back)
{
    if( !m_Data->srchdialog ) {
        return;
    }
    int line, col;
//     getCursorPosition(&line,&col);
    line = textCursor().blockNumber()+1;
    col = textCursor().columnNumber()+1;

    if (/*m_Data->last_search != DiffBrowserData::NONE &&*/ !back) {
        col = col+1;
    }
    while (1) {
//         bool result = find(to_find_string,case_sensitive,false,
//                         (!back),&line,&col);
        bool result = find(to_find_string);

        if (result) {
//             m_Data->last_search = back?DiffBrowserData::BACKWARD:DiffBrowserData::FORWARD;
            m_Data->pattern=to_find_string;
            break;
        }
        QWidget * _parent = m_Data->srchdialog->isVisible()?m_Data->srchdialog:parentWidget();
//         if (!m_Data->srchdialog->get_direction()) {
        if (m_Data->srchdialog->options() & KFind::FindIncremental) {
            // forward
            int query = KMessageBox::questionYesNo(
                    _parent,
                    i18n("End of document reached.\n"\
                         "Continue from the beginning?"),
                    i18n("Find"),
                    KStandardGuiItem::yes(),
                    KStandardGuiItem::no());
            if (query == KMessageBox::Yes){
                line = 0;
                col = 0;
//                 m_Data->last_search = DiffBrowserData::FORWARD;
            } else {
                break;
            }
        } else {
            int query = KMessageBox::questionYesNo(
                    _parent,
                    i18n("Beginning of document reached.\n"\
                         "Continue from the end?"),
                    i18n("Find"),
                    KStandardGuiItem::yes(),
                    KStandardGuiItem::no());
            if (query == KMessageBox::Yes){
//                 line = lines()-1;
                line = textCursor().blockNumber()-1;
//                 QString string = text(line);
                QString string = document()->findBlock(line).text();
                col  = string.length();
                if (col>0) {
                    --col;
                }
//                 m_Data->last_search = DiffBrowserData::BACKWARD;
            } else {
                break;
            }
        }
    }
}

/*!
    \fn DiffBrowser::searchdone_slot()
 */
void DiffBrowser::searchdone_slot()
{
    if (!m_Data->srchdialog)
        return;

    m_Data->srchdialog->hide();
    setFocus();
//     m_Data->last_finished_search = m_Data->last_search;
//     m_Data->last_search = DiffBrowserData::NONE;
//     m_Data->cs = m_Data->srchdialog->case_sensitive();
}

void DiffBrowser::searchagain_slot()
{
    doSearchAgain(false);
}

void DiffBrowser::searchagainback_slot()
{
    doSearchAgain(true);
}

void DiffBrowser::doSearchAgain(bool back)
{
    if (!m_Data->srchdialog || m_Data->pattern.isEmpty()) {
        startSearch();
    } else {
        m_Data->last_search = m_Data->last_finished_search;
        doSearch(m_Data->pattern,m_Data->cs,back);
//         m_Data->last_finished_search = m_Data->last_search;
//         m_Data->last_search = DiffBrowserData::NONE;
    }
}

void DiffBrowser::slotTextCodecChanged(const QString&codec)
{
    if (Kdesvnsettings::locale_for_diff()!=codec) {
        Kdesvnsettings::setLocale_for_diff(codec);
        printContent();
        Kdesvnsettings::self()->writeConfig();
    }
}

#include "diffbrowser.h.moc"
