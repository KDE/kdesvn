/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
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


#include "svnqt/svnqttypes.h"

#include <ksvnwidgets/ksvndialog.h>
#include <svnqt/repoparameter.h>

namespace Ui
{
class CreateRepoDlg;
}

class CreaterepoDlg: public KSvnDialog
{
    Q_OBJECT
public:
    explicit CreaterepoDlg(QWidget *parent = nullptr);
    ~CreaterepoDlg();
    svn::repository::CreateRepoParameter parameter() const;
    bool createMain() const;
    QString targetDir() const;

protected Q_SLOTS:
    void fsTypeChanged(int);
    void compatChanged15();
    void compatChanged16();
    void compatChanged18();

private:
    bool m_inChangeCompat;
    Ui::CreateRepoDlg *m_ui;
};
