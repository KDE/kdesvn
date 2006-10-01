/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht                                  *
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
#include "mergedlg_impl.h"
#include "rangeinput_impl.h"
#include "src/svnqt/url.hpp"
#include "helpers/ktranslateurl.h"
#include "src/settings/kdesvnsettings.h"

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
    m_SrcOneInput->setMode(KFile::Directory|KFile::File);
    if (!src1) {
        m_SrcOneInput->setEnabled(false);
        m_SrcOneInput->hide();
        m_SrcOneLabel->hide();
    }
    m_SrcTwoInput->setMode(KFile::Directory|KFile::File);
    if (!src2) {
        m_SrcTwoInput->setEnabled(false);
        m_SrcTwoInput->hide();
        m_SrcTwoLabel->hide();
    }
    m_OutInput->setMode(KFile::LocalOnly|KFile::Directory|KFile::File);
    if (!out) {
        m_OutInput->setEnabled(false);
        m_OutInput->hide();
        m_OutLabel->hide();
    }
    adjustSize();
    setMinimumSize(minimumSizeHint());
    m_useExternMerge->setChecked(Kdesvnsettings::extern_merge_default());
}

MergeDlg_impl::~MergeDlg_impl()
{
}

void MergeDlg_impl::setSrc1(const QString&what)
{
    if (what.isEmpty()) {
        m_SrcOneInput->setURL("");
        return;
    }
    KURL uri(what);
    kdDebug()<<"What: "<<what << " URL: "<<uri<<endl;
    if (uri.protocol()=="file") {
        if (what.startsWith("file:")) {
            uri.setProtocol("ksvn+file");
        } else {
            uri.setProtocol("");
        }
    } else {
        uri.setProtocol(helpers::KTranslateUrl::makeKdeUrl(uri.protocol()));
    }
    m_SrcOneInput->setURL(uri.url());
}

void MergeDlg_impl::setSrc2(const QString&what)
{
    if (what.isEmpty()) {
        m_SrcTwoInput->setURL("");
        return;
    }
    KURL uri(what);
    if (uri.protocol()=="file") {
        if (what.startsWith("file:")) {
            uri.setProtocol("ksvn+file");
        } else {
            uri.setProtocol("");
        }
    } else {
        uri.setProtocol(helpers::KTranslateUrl::makeKdeUrl(uri.protocol()));
    }
    m_SrcTwoInput->setURL(uri.url());
}

void MergeDlg_impl::setDest(const QString&what)
{
    if (what.isEmpty()) {
        m_OutInput->setURL("");
        return;
    }
    KURL uri(what);
    uri.setProtocol("");
    m_OutInput->setURL(uri.url());
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

bool MergeDlg_impl::useExtern()const
{
    return m_useExternMerge->isChecked();
}

QString MergeDlg_impl::Src1()const
{
    KURL uri(m_SrcOneInput->url());
    QString proto = svn::Url::transformProtokoll(uri.protocol());
    if (proto=="file"&&!m_SrcOneInput->url().startsWith("ksvn+file:")) {
        uri.setProtocol("");
    } else {
        uri.setProtocol(proto);
    }
    return uri.url();
}

QString MergeDlg_impl::Src2()const
{
    if (m_SrcTwoInput->url().isEmpty()) {
        return "";
    }
    KURL uri(m_SrcTwoInput->url());
    QString proto = svn::Url::transformProtokoll(uri.protocol());
    if (proto=="file"&&!m_SrcTwoInput->url().startsWith("ksvn+file:")) {
        uri.setProtocol("");
    } else {
        uri.setProtocol(proto);
    }
    return uri.url();
}

QString MergeDlg_impl::Dest()const
{
    KURL uri(m_OutInput->url());
    uri.setProtocol("");
    return uri.url();
}

Rangeinput_impl::revision_range MergeDlg_impl::getRange()const
{
    return m_RangeInput->getRange();
}


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

void MergeDlg_impl::externDisplayToggled(bool how)
{
    m_DryCheck->setEnabled(!how);
    m_RelatedCheck->setEnabled(!how);
    m_ForceCheck->setEnabled(!how);
}

#include "mergedlg_impl.moc"
