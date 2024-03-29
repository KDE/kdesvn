/***************************************************************************
 *   Copyright (C) 2106 by Christian Ehrlicher <ch.ehrlicher@gmx.de>       *
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
#include <ksvndialog.h>

#include <QtContainerFwd>

namespace Ui
{
class RevertForm;
}

class RevertForm : public KSvnDialog
{
    Q_OBJECT
public:
    explicit RevertForm(const QStringList &files, QWidget *parent = nullptr);
    ~RevertForm() override;

    svn::Depth getDepth() const;

private:
    Ui::RevertForm *m_ui;
};
