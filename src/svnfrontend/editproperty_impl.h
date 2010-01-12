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
#ifndef EDITPROPERTY_IMPL_H
#define EDITPROPERTY_IMPL_H

#include "ui_editpropsdlg.h"

class QStringList;
class QString;

class EditProperty_impl: public QWidget, public Ui::EditPropsWidget {
Q_OBJECT
public:
    EditProperty_impl(QWidget *parent = 0, const char *name = 0);
    ~EditProperty_impl();

    QString propName()const;
    QString propValue()const;
    void setPropName(const QString&);
    void setPropValue(const QString&);
    void setDir(bool dir);

protected slots:
    void updateToolTip(const QString&);

private:
    QStringList fileProperties;
    QStringList fileComments;
    QStringList dirProperties;
    QStringList dirComments;
    QString comment;
    bool isDir;
protected slots:
    virtual void showHelp();
};

#endif
