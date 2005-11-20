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
#include "checkoutinfo_impl.h"
#include "rangeinput_impl.h"
#include <kurlrequester.h>
#include <qlabel.h>
#include <klineedit.h>
#include <qcheckbox.h>
#include <klocale.h>

CheckoutInfo_impl::CheckoutInfo_impl(QWidget *parent, const char *name)
    :CheckoutInfo(parent, name)
{
    m_RangeInput->setStartOnly(true);
    m_RangeInput->setHeadDefault();
}

CheckoutInfo_impl::~CheckoutInfo_impl()
{
}

svn::Revision CheckoutInfo_impl::toRevision()
{
    return m_RangeInput->getRange().first;
}

QString CheckoutInfo_impl::reposURL()
{
    return m_UrlEdit->text();
}

QString CheckoutInfo_impl::targetDir()
{
    if (!m_CreateDirButton->isChecked()) {
        return  m_TargetSelector->url();
    }
    QString _uri = reposURL();
    while (_uri.endsWith("/")) {
        _uri.truncate(_uri.length()-1);
    }
    QStringList l = QStringList::split('/',_uri);
    if (l.count()==0) {
        return m_TargetSelector->url();
    }
    return  m_TargetSelector->url()+"/"+l[l.count()-1];
}

bool CheckoutInfo_impl::forceIt()
{
    return m_forceButton->isChecked();
}

void CheckoutInfo_impl::setStartUrl(const QString&what)
{
    m_UrlEdit->setText(what);
}

void CheckoutInfo_impl::disableForce(bool how)
{
    if (how) {
        m_forceButton->setEnabled(false);
        m_forceButton->hide();
    } else if (!how) {
        m_forceButton->setEnabled(false);
        m_forceButton->show();
    }
}

void CheckoutInfo_impl::forceAsRecursive(bool how)
{
    if (how) {
        m_forceButton->setText(i18n("Recursive"));
        m_forceButton->setChecked(true);
    } else  {
        m_forceButton->setText(i18n("Force"));
        m_forceButton->setChecked(false);
    }
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
bool CheckoutInfo_impl::openAfterJob()
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

void CheckoutInfo_impl::urlChanged(const QString&)
{
}

#include "checkoutinfo_impl.moc"
