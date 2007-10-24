/***************************************************************************
 *   Copyright (C) 2005-2007 by Rajko Albrecht                             *
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
#include "revisionbuttonimpl.h"
#include "src/svnfrontend/fronthelpers/rangeinput_impl.h"
#include "src/settings/kdesvnsettings.h"

#include <kpushbutton.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kapplication.h>
#include <klocale.h>

#include <qvbox.h>

RevisionButtonImpl::RevisionButtonImpl(QWidget *parent, const char *name)
    :RevisionButton(parent, name),m_Rev(svn::Revision::UNDEFINED)
{
}

RevisionButtonImpl::~RevisionButtonImpl()
{
}

void RevisionButtonImpl::setRevision(const svn::Revision&aRev)
{
    m_Rev = aRev;
    m_RevisionButton->setText(m_Rev.toString());
}

void RevisionButtonImpl::askRevision()
{
    Rangeinput_impl*rdlg;
    int buttons = KDialogBase::Ok|KDialogBase::Cancel;

    KDialogBase * dlg = new KDialogBase(KApplication::activeModalWidget(),"Revinput",true,"Input revision",buttons);

    if (!dlg) {
        return;
    }

    QWidget* Dialog1Layout = dlg->makeVBoxMainWidget();
    rdlg = new Rangeinput_impl(Dialog1Layout);
    dlg->resize(dlg->configDialogSize(*(Kdesvnsettings::self()->config()),"log_revisions_dlg"));

    rdlg->setStartOnly(true);
    if (dlg->exec()==QDialog::Accepted) {
        setRevision(rdlg->getRange().first);
    }
    dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"log_revisions_dlg",false);
    delete dlg;
}

#include "revisionbuttonimpl.moc"
