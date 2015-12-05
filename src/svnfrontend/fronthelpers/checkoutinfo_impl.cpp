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
#include "checkoutinfo_impl.h"
#include "rangeinput_impl.h"
#include "src/ksvnwidgets/depthselector.h"
#include "src/svnqt/url.h"
#include "helpers/ktranslateurl.h"
#include <kurlrequester.h>
#include <qlabel.h>
#include <qtooltip.h>

#include <klineedit.h>
#include <qcheckbox.h>
#include <klocale.h>
#include <kdebug.h>

CheckoutInfo_impl::CheckoutInfo_impl(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    m_RangeInput->setStartOnly(true);
    m_RangeInput->setHeadDefault();
    m_TargetSelector->setMode(KFile::LocalOnly | KFile::Directory);
    m_UrlEdit->setMode(KFile::Directory);
}

CheckoutInfo_impl::~CheckoutInfo_impl()
{
}

svn::Revision CheckoutInfo_impl::toRevision() const
{
    return m_RangeInput->getRange().first;
}

QString CheckoutInfo_impl::reposURL() const
{
    KUrl uri(m_UrlEdit->url());
    QString proto = svn::Url::transformProtokoll(uri.protocol());
    // uri.protocol() is the internal (or kioslave?) protocol
    // proto is the protocol given to the svn api
    // why ksvn+file needs a different handling than svn+file is
    // currently unknown but this logic is used in other places too
    if (proto == QLatin1String("file") &&
            uri.protocol() != QLatin1String("ksvn+file")) {
        uri.setProtocol(QString());
    } else {
        uri.setProtocol(proto);
    }
    return uri.prettyUrl(KUrl::RemoveTrailingSlash);
}

QString CheckoutInfo_impl::targetDir() const
{
    if (!m_CreateDirButton->isChecked()) {
        return  m_TargetSelector->url().url();
    }
    QString _uri = reposURL();
    QStringList l = _uri.split(QLatin1Char('/'), QString::SkipEmptyParts);
    if (l.isEmpty()) {
        return m_TargetSelector->url().url();
    }
    return  m_TargetSelector->url().path() + QLatin1Char('/') + l[l.count() - 1];
}

bool CheckoutInfo_impl::overwrite() const
{
    return m_overwriteButton->isChecked();
}

/*!
    \fn CheckoutInfo_impl::setTargetUrl(const QString&)
 */
void CheckoutInfo_impl::setTargetUrl(const QString &what)
{
    m_TargetSelector->setUrl(what);
}

void CheckoutInfo_impl::setStartUrl(const QString &what)
{
    KUrl uri(helpers::KTranslateUrl::string2Uri(what));
    m_UrlEdit->setUrl(uri);
}

void CheckoutInfo_impl::hideDepth(bool how, bool overwriteAsRecurse)
{
    if (how) {
        m_DepthSelector->setEnabled(false);
        m_DepthSelector->hide();
        if (overwriteAsRecurse) {
            m_overwriteButton->setToolTip(i18n("Make operation recursive"));
            m_overwriteButton->setText(i18n("Recursive"));
        }
    } else if (!how) {
        m_DepthSelector->setEnabled(false);
        m_DepthSelector->show();
        m_overwriteButton->setText(i18n("Overwrite existing"));
        m_overwriteButton->setToolTip(i18n("May existing unversioned items overwritten"));
    }
    adjustSize();
}

svn::Depth CheckoutInfo_impl::getDepth() const
{
    if (m_DepthSelector->isEnabled()) {
        return m_DepthSelector->getDepth();
    }
    return svn::DepthUnknown;
}

void CheckoutInfo_impl::disableTargetDir(bool how)
{
    if (how) {
        m_TargetSelector->setEnabled(false);
        m_TargetSelector->hide();
        m_TargetLabel->hide();
    } else if (!how) {
        m_TargetSelector->setEnabled(true);
        m_TargetSelector->show();
        m_TargetLabel->show();
    }
}

void CheckoutInfo_impl::disableOpen(bool how)
{
    if (how) {
        m_ShowExplorer->setEnabled(false);
        m_ShowExplorer->hide();
    } else if (!how) {
        m_ShowExplorer->setEnabled(true);
        m_ShowExplorer->show();
    }
}

/*!
    \fn CheckoutInfo_impl::openAfterJob()
 */
bool CheckoutInfo_impl::openAfterJob() const
{
    return m_ShowExplorer->isChecked();
}

/*!
    \fn CheckoutInfo_impl::disableRange(bool how)
 */
void CheckoutInfo_impl::disableRange(bool how)
{
    if (how) {
        m_RangeInput->setEnabled(false);
        m_RangeInput->hide();
    } else {
        m_RangeInput->setEnabled(true);
        m_RangeInput->show();
    }
}

void CheckoutInfo_impl::urlChanged(const QString &)
{
}

void CheckoutInfo_impl::disableAppend(bool how)
{
    m_CreateDirButton->setChecked(!how);
    if (how) {
        m_CreateDirButton->hide();
    } else {
        m_CreateDirButton->show();
    }
}

/*!
    \fn CheckoutInfo_impl::ignoreExternals()
 */
bool CheckoutInfo_impl::ignoreExternals() const
{
    return m_ignoreExternals->isChecked();
}

void CheckoutInfo_impl::disableExternals(bool how)
{
    m_ignoreExternals->setChecked(!how);
    if (how) {
        m_ignoreExternals->hide();
    } else {
        m_ignoreExternals->show();
    }
}
