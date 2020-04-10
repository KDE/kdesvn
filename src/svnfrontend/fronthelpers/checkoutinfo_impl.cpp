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
#include "ksvnwidgets/depthselector.h"
#include "svnqt/url.h"
#include "helpers/ktranslateurl.h"

CheckoutInfo_impl::CheckoutInfo_impl(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    m_RangeInput->setStartOnly(true);
    m_RangeInput->setHeadDefault();
    m_TargetSelector->setMode(KFile::LocalOnly | KFile::Directory);
    m_UrlEdit->setMode(KFile::Directory);
    hideIgnoreKeywords(true);
}

CheckoutInfo_impl::~CheckoutInfo_impl()
{
}

svn::Revision CheckoutInfo_impl::toRevision() const
{
    return m_RangeInput->getRange().first;
}

QUrl CheckoutInfo_impl::reposURL() const
{
    QUrl uri(m_UrlEdit->url());
    uri.setScheme(svn::Url::transformProtokoll(uri.scheme()));
    return uri;
}

QString CheckoutInfo_impl::targetDir() const
{
    const QString tgt = m_TargetSelector->url().toLocalFile();
    if (tgt.isEmpty() || !m_CreateDirButton->isChecked()) {
        return tgt;
    }
    // append last source url path to the target directory
    const QString _uri = reposURL().path();
    const QVector<QStringRef> l = _uri.splitRef(QLatin1Char('/'), QString::SkipEmptyParts);
    if (l.isEmpty()) {
        return tgt;
    }
    return tgt + QLatin1Char('/') + l.last().toString();
}

bool CheckoutInfo_impl::overwrite() const
{
    return m_overwriteButton->isChecked();
}

/*!
    \fn CheckoutInfo_impl::setTargetUrl(const QUrl&)
 */
void CheckoutInfo_impl::setTargetUrl(const QUrl &what)
{
    m_TargetSelector->setUrl(what);
}

void CheckoutInfo_impl::setStartUrl(const QUrl &what)
{
    m_UrlEdit->setUrl(what);
}

void CheckoutInfo_impl::hideDepth(bool how)
{
    if (how) {
        m_DepthSelector->setEnabled(false);
        m_DepthSelector->hide();
    } else if (!how) {
        m_DepthSelector->setEnabled(false);
        m_DepthSelector->show();
    }
    adjustSize();
}

void CheckoutInfo_impl::hideOverwrite(bool hide)
{
    m_overwriteButton->setHidden(hide);
}

void CheckoutInfo_impl::hideIgnoreKeywords(bool hide)
{
    m_IgnoreKeywords->setHidden(hide);
}

bool CheckoutInfo_impl::ignoreKeywords() const
{
    return m_IgnoreKeywords->isChecked();
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
