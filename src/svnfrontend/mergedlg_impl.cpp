/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht   *
 *   ral@alwins-world.de   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "mergedlg_impl.h"
#include "rangeinput_impl.h"

#include <kurlrequester.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kdebug.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qvbox.h>

MergeDlg_impl::MergeDlg_impl(QWidget *parent, const char *name,bool src1,bool src2,bool out)
    :MergeDlg(parent, name)
{
    if (!src1) {
        m_SrcOneInput->setEnabled(false);
        m_SrcOneInput->hide();
        m_SrcOneLabel->hide();
    }
    if (!src2) {
        m_SrcTwoInput->setEnabled(false);
        m_SrcTwoInput->hide();
        m_SrcTwoLabel->hide();
    }
    if (!out) {
        m_OutInput->setEnabled(false);
        m_OutInput->hide();
        m_OutLabel->hide();
    }
}

MergeDlg_impl::~MergeDlg_impl()
{
}

bool MergeDlg_impl::recursive()const
{
    return m_RecursiveCheck->isChecked();
}

bool MergeDlg_impl::force()const
{
    return m_ForceCheck->isChecked();
}

bool MergeDlg_impl::ignorerelated()const
{
    return m_RelatedCheck->isChecked();
}

bool MergeDlg_impl::dryrun()const
{
    return m_DryCheck->isChecked();
}

QString MergeDlg_impl::Src1()const
{
    return m_SrcOneInput->url();
}

QString MergeDlg_impl::Src2()const
{
    return m_SrcTwoInput->url();
}

QString MergeDlg_impl::Dest()const
{
    return m_OutInput->url();
}

Rangeinput_impl::revision_range MergeDlg_impl::getRange()const
{
    return m_RangeInput->getRange();
}

#include "mergedlg_impl.moc"


/*!
    \fn MergeDlg_impl::getMergeRange(bool*force,bool*recursive,bool*related,bool*dry)
 */
bool MergeDlg_impl::getMergeRange(Rangeinput_impl::revision_range&range,bool*force,bool*recursive,bool*ignorerelated,bool*dry,
    QWidget*parent,const char*name)
{
    MergeDlg_impl*ptr = 0;
    KDialogBase dlg(parent,name,true,i18n("Enter merge range"),
            KDialogBase::Ok|KDialogBase::Cancel,
            KDialogBase::Ok,true);
    QWidget* Dialog1Layout = dlg.makeVBoxMainWidget();
    ptr = new MergeDlg_impl(Dialog1Layout,"merge_range_dlg",false,false,false);
    dlg.resize( QSize(480,360).expandedTo(dlg.minimumSizeHint()) );
    if (dlg.exec()!=QDialog::Accepted) {
        return false;
    }
    range = ptr->getRange();
    *force = ptr->force();
    *recursive=ptr->recursive();
    *ignorerelated=ptr->ignorerelated();
    *dry = ptr->dryrun();
    return true;
}
