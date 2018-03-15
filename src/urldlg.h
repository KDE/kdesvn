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
#pragma once

#include <QDialog>

namespace Ui
{
class UrlDlg;
}
class KUrlRequester;

/**
@author Rajko Albrecht
*/
class UrlDlg : public QDialog
{
    Q_OBJECT
public:
    static QUrl getUrl(QWidget *parent = nullptr);
private:
    explicit UrlDlg(QWidget *parent = nullptr);
    ~UrlDlg();

private slots:
    void accept() final;
    void slotTextChanged(const QString &);
private:
    KUrlRequester *m_urlRequester;
    Ui::UrlDlg *m_ui;
};
