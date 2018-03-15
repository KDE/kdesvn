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
#include "svnqt/revision.h"
#include "svnqt/svnqttypes.h"

class CheckoutInfo_impl: public QWidget, public Ui::CheckoutInfo
{
    Q_OBJECT
public:
    explicit CheckoutInfo_impl(QWidget *parent = nullptr);
    ~CheckoutInfo_impl();

    svn::Revision toRevision() const;
    QUrl reposURL() const;
    QString targetDir() const;

    bool overwrite() const;
    svn::Depth getDepth() const;
    void setStartUrl(const QUrl &);

    void disableForce(bool how);
    void disableTargetDir(bool how);
    void disableAppend(bool how);
    void disableOpen(bool how);
    bool openAfterJob() const;
    void disableRange(bool how);
    void setTargetUrl(const QUrl &);
    bool ignoreExternals() const;
    void hideDepth(bool hide);
    void hideOverwrite(bool hide);
    void hideIgnoreKeywords(bool hide);
    bool ignoreKeywords() const;
protected slots:
    void urlChanged(const QString &);
};

#endif
