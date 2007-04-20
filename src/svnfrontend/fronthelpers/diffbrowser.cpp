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

#include <kglobalsettings.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <klocale.h>

#include <qfont.h>
/*!
    \fn DiffBrowser::DiffBrowser(QWidget*parent=0,const char*name=0)
 */
DiffBrowser::DiffBrowser(QWidget*parent,const char*name)
    : QTextBrowser( parent, name)
{
    setTextFormat(Qt::PlainText);
    setFont(KGlobalSettings::fixedFont());
    //setTabStopWidth(4);
    setWordWrap(QTextEdit::NoWrap);
    m_Syntax = new DiffSyntax(this);
    srchdialog=0;
    setFocus();
}

/*!
    \fn DiffBrowser::~DiffBrowser()
 */
 DiffBrowser::~DiffBrowser()
{
    delete m_Syntax;
}

void DiffBrowser::setText(const QString&aText)
{
    m_content.setRawData(aText.local8Bit(),aText.local8Bit().size());
    QTextBrowser::setText(aText);
    setCursorPosition(0,0);
}

void DiffBrowser::setText(const QByteArray&aText)
{
    m_content=aText;
    QTextBrowser::setText(QString::fromLocal8Bit(aText,aText.size()));
    setCursorPosition(0,0);
}

/*!
    \fn DiffBrowser::saveDiff()
 */
void DiffBrowser::saveDiff()
{
    QString saveTo = KFileDialog::getSaveFileName(QString::null,"text/x-diff");
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
    tfile.open(IO_Truncate|IO_WriteOnly|IO_Raw);
    QDataStream stream( &tfile );
    stream.writeRawBytes(m_content.data(),m_content.size());
}

void DiffBrowser::keyPressEvent(QKeyEvent*ev)
{
    if ( ev->key() == Key_Return && ev->state() == ControlButton ) {
        ev->ignore();
        return;
    }
    if (ev->key() == Key_F3) {
        kdDebug()<<"Weitersuchen..."<<endl;
    } else if (ev->key()==Key_F && ev->state() == ControlButton) {
        kdDebug()<<"Suchen..."<<endl;
        startSearch();
    } else {
        QTextBrowser::keyPressEvent(ev);
    }
}

void DiffBrowser::startSearch()
{
    int line, col;
    getCursorPosition(&line,&col);
    //kdDebug()<<"Line: "<<line << " Col: "<<col << endl;
}

#include "diffbrowser.h.moc"
