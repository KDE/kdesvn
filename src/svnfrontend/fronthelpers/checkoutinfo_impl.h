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
#ifndef CHECKOUTINFO_IMPL_H
#define CHECKOUTINFO_IMPL_H

#include "ui_checkoutinfo.h"
#include "src/svnqt/revision.h"
#include "src/svnqt/svnqttypes.h"
#include "kurl.h"

class CheckoutInfo_impl: public QWidget, public Ui::CheckoutInfo {
Q_OBJECT
public:
    CheckoutInfo_impl(QWidget *parent = 0, const char *name = 0);
    virtual ~CheckoutInfo_impl();

    svn::Revision toRevision();
    QString reposURL();
    QString targetDir();

    bool overwrite();
    svn::Depth getDepth();
    void setStartUrl(const QString&);

    void disableForce(bool how);
    void disableTargetDir(bool how);
    void disableAppend(bool how);
    void disableOpen(bool how);
    void disableExternals(bool how);
    bool openAfterJob();
    virtual void disableRange(bool how);
    void setTargetUrl(const QString&);
    bool ignoreExternals();
    void hideDepth(bool hide,bool overwriteAsRecurse);
protected slots:
    virtual void urlChanged(const QString&);
};

#endif
