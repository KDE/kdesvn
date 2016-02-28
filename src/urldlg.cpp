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
#include "urldlg.h"
#include <kurlrequester.h>
#include <QVBoxLayout>
#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <KHistoryComboBox>
#include <qlabel.h>

UrlDlg::UrlDlg(QWidget *parent)
    : KDialog(parent)
{
    setButtons(Ok | Cancel | User1);
    setDefaultButton(Ok);
    showButtonSeparator(true);

    m_plainPage = new QWidget(this);
    setMainWidget(m_plainPage);

    QVBoxLayout *topLayout = new QVBoxLayout(m_plainPage);
    QLabel *label = new QLabel(i18n("Open repository or working copy") , m_plainPage);
    topLayout->addWidget(label);

    KHistoryComboBox *combo = new KHistoryComboBox(this);
    combo->setDuplicatesEnabled(false);
    KConfigGroup kc = KSharedConfig::openConfig()->group("Open-repository settings");
    int max = kc.readEntry(QLatin1String("Maximum history"), 15);
    combo->setMaxCount(max);
    QStringList list = kc.readEntry(QLatin1String("History"), QStringList());
    combo->setHistoryItems(list);
    combo->setMinimumWidth(100);
    combo->adjustSize();
    if (combo->width() > 300) {
        combo->resize(300, combo->height());
    }

    m_urlRequester = new KUrlRequester(combo, m_plainPage);
    topLayout->addWidget(m_urlRequester);
    m_urlRequester->setFocus();
    m_urlRequester->setMode(KFile::ExistingOnly | KFile::Directory);
    connect(m_urlRequester->comboBox(), SIGNAL(currentTextChanged(QString)), SLOT(slotTextChanged(QString)));
    enableButtonOk(false);
    enableButton(KDialog::User1, false);
    setButtonGuiItem(KDialog::User1, KGuiItem(i18n("Clear"), QIcon::fromTheme("clear")));
    connect(this, SIGNAL(user1Clicked()), m_urlRequester, SLOT(clear()));
    m_urlRequester->adjustSize();
    resize(QSize(400, sizeHint().height()));
}

/*!
    \fn UrlDlg::accept()
 */
void UrlDlg::accept()
{
    KHistoryComboBox *combo = static_cast<KHistoryComboBox *>(m_urlRequester->comboBox());
    if (combo) {
        combo->addToHistory(m_urlRequester->url().url());
        KConfigGroup kc = KSharedConfig::openConfig()->group("Open-repository settings");
        kc.writeEntry(QLatin1String("History"), combo->historyItems());
        kc.sync();
    }
    KDialog::accept();
}

/*!
    \fn UrlDlg::slotTextChanged(const QString&)
 */
void UrlDlg::slotTextChanged(const QString &text)
{
    bool state = !text.trimmed().isEmpty();
    enableButtonOk(state);
    enableButton(KDialog::User1, state);
}

/*!
    \fn UrlDlg::getUrl(QWidget*parent)
 */
QUrl UrlDlg::getUrl(QWidget *parent)
{
    QUrl ret;
    QPointer<UrlDlg> dlg(new UrlDlg(parent));
    dlg->setCaption(i18n("Open"));
    if (dlg->exec() == KDialog::Accepted) {
        // added by Wellu Mäkinen <wellu@wellu.org>
        //
        // get rid of leading whitespace
        // that is %20 in encoded form
        QString url = dlg->m_urlRequester->url().toString();

        // decodes %20 to normal spaces
        // trims the whitespace from both ends
        // of the URL
        ret = QUrl(url.trimmed());
    }
    delete dlg;
    return ret;
}
