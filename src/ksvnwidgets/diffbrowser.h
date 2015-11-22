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
#ifndef DIFF_BROWSER_H
#define DIFF_BROWSER_H

#include <ktextbrowser.h>
#include <QKeyEvent>

class DiffSyntax;
// class KEdFind;
/* FIXME TODO fix the search */
class DiffBrowserData;

class DiffBrowser : public KTextBrowser
{
    Q_OBJECT

public:
    explicit DiffBrowser(QWidget *parent = 0);
    virtual ~DiffBrowser();

public slots:
    virtual void setText(const QString &aText);
    virtual void setText(const QByteArray &ex);
    virtual void saveDiff();
    virtual void slotTextCodecChanged(const QString &);

protected:
    virtual void keyPressEvent(QKeyEvent *);

    virtual void startSearch();
    virtual void doSearch(const QString &to_find_string, bool back);
    virtual void doSearchAgain(bool back);

protected:
    DiffBrowserData *m_Data;

    void printContent();

protected slots:
    virtual void search_slot();
    virtual void searchagain_slot();
    virtual void searchagainback_slot();
};

#endif

