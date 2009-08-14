/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this program (in the file LGPL.txt); if not,         *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at http://kdesvn.alwins-world.de.           *
 ***************************************************************************/
#ifndef REVISIONBUTTONIMPL_H
#define REVISIONBUTTONIMPL_H

#include "ui_revisionbutton.h"
#include "svnqt/revision.hpp"

class RevisionButtonImpl: public QWidget, public Ui::RevisionButton {
    Q_OBJECT

public:
    RevisionButtonImpl(QWidget *parent = 0, const char *name = 0);
    virtual ~RevisionButtonImpl();

    virtual void setRevision(const svn::Revision&aRev);

    virtual void setNoWorking(bool);
    const svn::Revision& revision()const {
        return m_Rev;
    }

protected:
    svn::Revision m_Rev;
    bool m_noWorking;

public slots:
    virtual void askRevision();
signals:
    void revisionChanged();

};

#endif
