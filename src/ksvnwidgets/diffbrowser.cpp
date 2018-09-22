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
#include "diffsyntax.h"
#include "settings/kdesvnsettings.h"

#include <QFileDialog>
#include <KMessageBox>
#include <KLocalizedString>
#include <KStandardGuiItem>
#include <KFind>
#include <KFindDialog>

#include <QApplication>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QTextCodec>

/*!
    \fn DiffBrowser::DiffBrowser(QWidget*parent=0)
 */
DiffBrowser::DiffBrowser(QWidget *parent)
    : QTextBrowser(parent)
    , m_srchdialog(nullptr)
{
//     setTextFormat(Qt::PlainText);
    setLineWrapMode(QTextEdit::NoWrap);
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    setLineWrapMode(QTextEdit::NoWrap);
    m_Syntax = new DiffSyntax(document());
    setToolTip(i18n("Ctrl-F for search, F3 or Shift-F3 for search again."));
    setWhatsThis(i18n("<b>Display differences between files</b><p>You may search inside text with Ctrl-F.</p><p>F3 for search forward again, Shift-F3 for search backward again.</p><p>You may save the (original) output with Ctrl-S.</p>"));
    setFocus();
}

/*!
    \fn DiffBrowser::~DiffBrowser()
 */
DiffBrowser::~DiffBrowser()
{
    delete m_Syntax;
    delete m_srchdialog;
}

void DiffBrowser::setText(const QByteArray &aText)
{
    m_content = aText;
    printContent();
    moveCursor(QTextCursor::Start);
}

void DiffBrowser::printContent()
{
    QTextCodec *cc = QTextCodec::codecForName(Kdesvnsettings::locale_for_diff().toUtf8());
    if (!cc) {
        QTextBrowser::setText(QString::fromLocal8Bit(m_content));
    } else {
        QTextBrowser::setText(cc->toUnicode(m_content));
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
    tfile.write(m_content);
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
    if (!m_srchdialog) {
        m_srchdialog = new KFindDialog(this);
        m_srchdialog->setSupportsWholeWordsFind(true);
        m_srchdialog->setHasCursor(false);
        m_srchdialog->setHasSelection(false);
        m_srchdialog->setSupportsRegularExpressionFind(false);
        connect(m_srchdialog, &KFindDialog::okClicked, this, &DiffBrowser::search_slot);
    }
    QString _st = m_srchdialog->pattern();
    m_srchdialog->setPattern(_st.isEmpty() ? m_pattern : _st);
    m_srchdialog->show();
}

/*!
    \fn DiffBrowser::search_slot()
 */
void DiffBrowser::search_slot()
{
    if (!m_srchdialog) {
        return;
    }
    doSearch(m_srchdialog->pattern(),
             (m_srchdialog->options() & KFind::FindBackwards) == KFind::FindBackwards);
}

void DiffBrowser::doSearch(const QString &to_find_string, bool back)
{
    if (!m_srchdialog) {
        return;
    }
    while (true) {
        bool result;
        QTextDocument::FindFlags f;
        if (back) {
            f = QTextDocument::FindBackward;
        }
        if (m_srchdialog->options()&KFind::WholeWordsOnly) {
            f |= QTextDocument::FindWholeWords;
        }
        if (m_srchdialog->options()&KFind::CaseSensitive) {
            f |= QTextDocument::FindCaseSensitively;
        }

        result = find(to_find_string, f);

        if (result) {
            m_pattern = to_find_string;
            break;
        }
        QWidget *_parent = m_srchdialog->isVisible() ? m_srchdialog : parentWidget();
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
    if (!m_srchdialog || m_pattern.isEmpty()) {
        startSearch();
    } else {
        doSearch(m_pattern, back);
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
