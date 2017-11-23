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
#include "mergedlg_impl.h"
#include "rangeinput_impl.h"
#include <svnqt/url.h>
#include <svnqt/version_check.h>
#include "helpers/ktranslateurl.h"
#include "ksvnwidgets/ksvndialog.h"
#include "settings/kdesvnsettings.h"

#include <KLocalizedString>


MergeDlg_impl::MergeDlg_impl(QWidget *parent, bool src1, bool src2, bool out, bool record_only, bool reintegrate)
    : QWidget(parent), Ui::MergeDlg()
{
    setupUi(this);
    m_SrcOneInput->setMode(KFile::Directory | KFile::File);
    if (!src1) {
        m_SrcOneInput->setEnabled(false);
        m_SrcOneInput->hide();
        m_SrcOneLabel->hide();
    }
    m_SrcTwoInput->setMode(KFile::Directory | KFile::File);
    if (!src2) {
        m_SrcTwoInput->setEnabled(false);
        m_SrcTwoInput->hide();
        m_SrcTwoLabel->hide();
    }
    m_OutInput->setMode(KFile::LocalOnly | KFile::Directory | KFile::File);
    if (!out) {
        m_OutInput->setEnabled(false);
        m_OutInput->hide();
        m_OutLabel->hide();
    }
    if (!record_only) {
        m_RecordOnly->setEnabled(false);
        m_RecordOnly->hide();
    }
    if (!reintegrate) {
        m_Reintegrate->setEnabled(false);
        m_Reintegrate->hide();
    }
    if (svn::Version::version_major() == 1 && svn::Version::version_minor() < 7) {
        m_AllowMixedRev->setEnabled(false);
        m_AllowMixedRev->hide();
    }
    adjustSize();
    setMinimumSize(minimumSizeHint());
    m_useExternMerge->setChecked(Kdesvnsettings::extern_merge_default());
}

MergeDlg_impl::~MergeDlg_impl()
{
}

void MergeDlg_impl::setSrc1(const QString &what)
{
    if (what.isEmpty()) {
        m_SrcOneInput->clear();
        return;
    }
    m_SrcOneInput->setUrl(helpers::KTranslateUrl::string2Uri(what));
}

void MergeDlg_impl::setSrc2(const QString &what)
{
    if (what.isEmpty()) {
        m_SrcTwoInput->clear();
        return;
    }
    m_SrcTwoInput->setUrl(helpers::KTranslateUrl::string2Uri(what));
}

void MergeDlg_impl::setDest(const QString &what)
{
    if (what.isEmpty()) {
        m_OutInput->clear();
        return;
    }
    // only local destination
    m_OutInput->setUrl(QUrl::fromLocalFile(what));
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

bool MergeDlg_impl::recordOnly()const
{
    return m_RecordOnly->isChecked();
}

bool MergeDlg_impl::reintegrate()const
{
    return m_Reintegrate->isChecked();
}

bool MergeDlg_impl::allowmixedrevs()const
{
    return m_AllowMixedRev->isChecked();
}

QString MergeDlg_impl::Src1()const
{
    QUrl uri(m_SrcOneInput->url());
    const QString proto = svn::Url::transformProtokoll(uri.scheme());
    if (proto == QLatin1String("file") && !m_SrcOneInput->url().scheme().startsWith("ksvn+file:")) {
        return uri.toLocalFile();
    }

    uri.setScheme(proto);
    return uri.url();
}

QString MergeDlg_impl::Src2()const
{
    if (m_SrcTwoInput->url().isEmpty()) {
        return QString();
    }
    QUrl uri(m_SrcTwoInput->url());
    const QString proto = svn::Url::transformProtokoll(uri.scheme());
    if (proto == QLatin1String("file") && !m_SrcTwoInput->url().scheme().startsWith("ksvn+file:")) {
        return uri.toLocalFile();
    }

    uri.setScheme(proto);
    return uri.url();
}

QString MergeDlg_impl::Dest()const
{
    return m_OutInput->url().toLocalFile();
}

Rangeinput_impl::revision_range MergeDlg_impl::getRange()const
{
    return m_RangeInput->getRange();
}


/*!
    \fn MergeDlg_impl::getMergeRange(bool*force,bool*recursive,bool*related,bool*dry)
 */
bool MergeDlg_impl::getMergeRange(Rangeinput_impl::revision_range &range, bool *force, bool *recursive, bool *ignorerelated, bool *dry,
                                  bool *useExternal, bool *allowmixedrevs,
                                  QWidget *parent)
{
    QPointer<KSvnSimpleOkDialog> dlg(new KSvnSimpleOkDialog(QStringLiteral("merge_range"), parent));
    dlg->setWithCancelButton();
    dlg->setHelp(QLatin1String("merging-items"));
    dlg->setWindowTitle(i18nc("@title:window", "Enter Merge Range"));

    MergeDlg_impl *ptr = new MergeDlg_impl(dlg, false, false, false, false, false);
    dlg->addWidget(ptr);
    bool ret = false;
    if (dlg->exec() == QDialog::Accepted) {
        range = ptr->getRange();
        *force = ptr->force();
        *recursive = ptr->recursive();
        *ignorerelated = ptr->ignorerelated();
        *dry = ptr->dryrun();
        *useExternal = ptr->useExtern();
        *allowmixedrevs = ptr->allowmixedrevs();
        ret = true;
    }
    delete dlg;

    return ret;
}

void MergeDlg_impl::externDisplayToggled(bool how)
{
    m_DryCheck->setEnabled(!how);
    m_RelatedCheck->setEnabled(!how);
    m_ForceCheck->setEnabled(!how);
    m_RecordOnly->setEnabled(!how);
    m_Reintegrate->setEnabled(!how);
}

void MergeDlg_impl::recordOnlyToggled(bool)
{
}

void MergeDlg_impl::reintegrateToggled(bool how)
{
    m_RelatedCheck->setEnabled(!how);
    m_ForceCheck->setEnabled(!how);
    m_RecordOnly->setEnabled(!how);
    m_useExternMerge->setEnabled(!how);
    m_RecursiveCheck->setEnabled(!how);
    m_SrcTwoInput->setEnabled(!how);
    m_RangeInput->setStartOnly(how);
    if (how) {
        m_RecursiveCheck->setChecked(true);
        m_useExternMerge->setChecked(false);
        m_RecordOnly->setChecked(false);
        m_ForceCheck->setChecked(false);
        m_RelatedCheck->setChecked(false);
    }
}
