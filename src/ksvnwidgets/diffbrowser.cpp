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
#include "settings/kdesvnsettings.h"

#include <QFileDialog>
#include <KMessageBox>
#include <KLocalizedString>
#include <KStandardGuiItem>
#include <KFind>

#include <QApplication>
#include <QKeyEvent>
#include <QTextCodec>

/*!
    \fn DiffBrowser::DiffBrowser(QWidget*parent=0)
 */
DiffBrowser::DiffBrowser(QWidget *parent)
    : QTextBrowser(parent)
{
//     setTextFormat(Qt::PlainText);
    setLineWrapMode(QTextEdit::NoWrap);
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_Data = new DiffBrowserData;

    setLineWrapMode(QTextEdit::NoWrap);
    m_Data->m_Syntax = new DiffSyntax(document());
    setToolTip(i18n("Ctrl-F for search, F3 or Shift-F3 for search again."));
    setWhatsThis(i18n("<b>Display differences between files</b><p>You may search inside text with Ctrl-F.</p><p>F3 for search forward again, Shift-F3 for search backward again.</p><p>You may save the (original) output with Ctrl-S.</p>"));
    setFocus();
}

/*!
    \fn DiffBrowser::~DiffBrowser()
 */
DiffBrowser::~DiffBrowser()
{
    delete m_Data;
}

void DiffBrowser::setText(const QByteArray &aText)
{
    m_Data->m_content = aText;
    printContent();
    moveCursor(QTextCursor::Start);
}

void DiffBrowser::printContent()
{
    QTextCodec *cc = QTextCodec::codecForName(Kdesvnsettings::locale_for_diff().toLocal8Bit());
    if (!cc) {
        QTextBrowser::setText(QString::fromLocal8Bit(m_Data->m_content, m_Data->m_content.size()));
    } else {
        QTextBrowser::setText(cc->toUnicode(m_Data->m_content));
    }
}

/*!
    \fn DiffBrowser::saveDiff()
 */
void DiffBrowser::saveDiff()
{
    QString saveTo = QFileDialog::getSaveFileName(this, i18n("Save diff"), QString(), i18n("Patch file (*.diff *.patch)"));
    if (saveTo.isEmpty()) {
        return;
    }
    QFile tfile(saveTo);
    if (tfile.exists()) {
        if (KMessageBox::warningYesNo(QApplication::activeModalWidget(),
                                      i18n("File %1 exists - overwrite?", saveTo))
                != KMessageBox::Yes) {
            return;
        }
    }
    tfile.open(QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Unbuffered);
    QDataStream stream(&tfile);
    stream.writeRawData(m_Data->m_content.data(), m_Data->m_content.size());
}

void DiffBrowser::keyPressEvent(QKeyEvent *ev)
{
    if (ev->key() == Qt::Key_Return) {
        ev->ignore();
        return;
    }
    if (ev->key() == Qt::Key_F3) {
        if (ev->modifiers() == Qt::ShiftModifier) {
            searchagainback_slot();
        } else {
            searchagain_slot();
        }
    } else if (ev->key() == Qt::Key_F && ev->modifiers() == Qt::ControlModifier) {
        startSearch();
    } else if (ev->key() == Qt::Key_S && ev->modifiers() == Qt::ControlModifier) {
        saveDiff();
    } else {
        QTextBrowser::keyPressEvent(ev);
    }
}

void DiffBrowser::startSearch()
{
    if (!m_Data->srchdialog) {
        m_Data->srchdialog = new KFindDialog(this);
        m_Data->srchdialog->setSupportsWholeWordsFind(true);
        m_Data->srchdialog->setHasCursor(false);
        m_Data->srchdialog->setHasSelection(false);
        m_Data->srchdialog->setSupportsRegularExpressionFind(false);
        connect(m_Data->srchdialog, SIGNAL(okClicked()), this, SLOT(search_slot()));
    }
    QString _st = m_Data->srchdialog->pattern();
    m_Data->srchdialog->setPattern(_st.isEmpty() ? m_Data->pattern : _st);
    m_Data->srchdialog->show();
}

/*!
    \fn DiffBrowser::search_slot()
 */
void DiffBrowser::search_slot()
{
    if (!m_Data->srchdialog) {
        return;
    }
    doSearch(m_Data->srchdialog->pattern(),
             (m_Data->srchdialog->options() & KFind::FindBackwards) == KFind::FindBackwards);
}

void DiffBrowser::doSearch(const QString &to_find_string, bool back)
{
    if (!m_Data->srchdialog) {
        return;
    }
    while (1) {
        bool result;
        QTextDocument::FindFlags f;
        if (back) {
            f = QTextDocument::FindBackward;
        }
        if (m_Data->srchdialog->options()&KFind::WholeWordsOnly) {
            f |= QTextDocument::FindWholeWords;
        }
        if (m_Data->srchdialog->options()&KFind::CaseSensitive) {
            f |= QTextDocument::FindCaseSensitively;
        }

        result = find(to_find_string, f);

        if (result) {
            m_Data->pattern = to_find_string;
            break;
        }
        QWidget *_parent = m_Data->srchdialog->isVisible() ? m_Data->srchdialog : parentWidget();
        QTextCursor tc = textCursor();
        if (!back) {
            KMessageBox::ButtonCode query = KMessageBox::questionYesNo(_parent,
                                                                       i18n("End of document reached.\n"\
                                                                            "Continue from the beginning?"),
                                                                       i18n("Find"),
                                                                       KStandardGuiItem::yes(),
                                                                       KStandardGuiItem::no());
            if (query == KMessageBox::Yes) {
                moveCursor(QTextCursor::Start);
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
            if (query == KMessageBox::Yes) {
                moveCursor(QTextCursor::End);
            } else {
                break;
            }
        }
    }
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
        doSearch(m_Data->pattern, back);
    }
}

void DiffBrowser::slotTextCodecChanged(const QString &codec)
{
    if (Kdesvnsettings::locale_for_diff() != codec) {
        Kdesvnsettings::setLocale_for_diff(codec);
        printContent();
        Kdesvnsettings::self()->save();
    }
}
