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
#include "revisionbuttonimpl.h"
#include "src/svnfrontend/fronthelpers/rangeinput_impl.h"
#include "src/settings/kdesvnsettings.h"

#include <kpushbutton.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kapplication.h>
#include <klocale.h>

#include <KVBox>
#include <QPointer>

RevisionButtonImpl::RevisionButtonImpl(QWidget *parent)
    : QWidget(parent),
      m_Rev(svn::Revision::UNDEFINED), m_noWorking(false)
{
    setupUi(this);
}

RevisionButtonImpl::~RevisionButtonImpl()
{
}

void RevisionButtonImpl::setRevision(const svn::Revision &aRev)
{
    m_Rev = aRev;
    m_RevisionButton->setText(m_Rev.toString());
    emit revisionChanged();
}

void RevisionButtonImpl::askRevision()
{
    Rangeinput_impl *rdlg;
    QPointer<KDialog> dlg(new KDialog());
    dlg->setCaption(i18n("Select revision"));
    dlg->setButtons(KDialog::Ok | KDialog::Cancel);
    dlg->showButtonSeparator(false);

    KVBox *Dialog1Layout = new KVBox(dlg);
    dlg->setMainWidget(Dialog1Layout);

    rdlg = new Rangeinput_impl(Dialog1Layout);
    rdlg->setStartOnly(true);
    rdlg->setNoWorking(m_noWorking);

    KConfigGroup _k(Kdesvnsettings::self()->config(), "log_revisions_dlg");
    dlg->restoreDialogSize(_k);
    if (dlg->exec() == QDialog::Accepted) {
        setRevision(rdlg->getRange().first);
    }
    if (dlg) {
        dlg->saveDialogSize(_k);
        delete dlg;
    }
}

void RevisionButtonImpl::setNoWorking(bool how)
{
    m_noWorking = how;
}
