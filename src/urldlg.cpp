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
#include <kcombobox.h>
#include <kurlrequester.h>
#include <qlayout.h>
#include <QVBoxLayout>
#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <klineedit.h>
#include <kurl.h>
#include <kdebug.h>
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

    init_dlg();
}

UrlDlg::~UrlDlg()
{
}

/*!
    \fn UrlDlg::init_dlg
 */
void UrlDlg::init_dlg()
{
    QVBoxLayout *topLayout = new QVBoxLayout(m_plainPage);   // /* plainPage() */, 0, spacingHint());
    QLabel *label = new QLabel(i18n("Open repository or working copy") , m_plainPage /* plainPage() */);
    topLayout->addWidget(label);

    KHistoryComboBox *combo = new KHistoryComboBox(this);
    combo->setDuplicatesEnabled(false);
    KConfigGroup kc = KGlobal::config()->group("Open-repository settings");
    int max = kc.readEntry(QString::fromLatin1("Maximum history"), 15);
    combo->setMaxCount(max);
    QStringList list = kc.readEntry(QString::fromLatin1("History"), QStringList());
    combo->setHistoryItems(list);
    combo->setMinimumWidth(100);
    combo->adjustSize();
    if (combo->width() > 300) {
        combo->resize(300, combo->height());
    }

    urlRequester_ = new KUrlRequester(combo, m_plainPage);
    topLayout->addWidget(urlRequester_);
    urlRequester_->setFocus();
    urlRequester_->setMode(KFile::ExistingOnly | KFile::Directory);
    connect(urlRequester_->comboBox(), SIGNAL(textChanged(QString)), SLOT(slotTextChanged(QString)));
    enableButtonOk(false);
    enableButton(KDialog::User1, false);
    setButtonGuiItem(KDialog::User1, KGuiItem(i18n("Clear"), KIcon("clear")));
    connect(this, SIGNAL(user1Clicked()), this, SLOT(slotClear()));
    urlRequester_->adjustSize();
    resize(QSize(400, sizeHint().height()));
}

/*!
    \fn UrlDlg::accept()
 */
void UrlDlg::accept()
{
    KHistoryComboBox *combo = static_cast<KHistoryComboBox *>(urlRequester_->comboBox());
    if (combo) {
        combo->addToHistory(urlRequester_->url().url());
        KConfigGroup kc = KGlobal::config()->group("Open-repository settings");
        kc.writeEntry(QString::fromLatin1("History"), combo->historyItems());
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
    \fn UrlDlg::slotClear()
 */
void UrlDlg::slotClear()
{
    urlRequester_->clear();
}

/*!
    \fn UrlDlg::selectedUrl()
 */
KUrl UrlDlg::selectedUrl()
{
    if (result() == QDialog::Accepted) {
        KUrl uri = urlRequester_->url();
        return uri;
    } else {
        return KUrl();
    }
}

/*!
    \fn UrlDlg::getUrl(QWidget*parent)
 */
KUrl UrlDlg::getUrl(QWidget *parent)
{
    KUrl ret;
    QPointer<UrlDlg> dlg(new UrlDlg(parent));
    dlg->setCaption(i18n("Open"));
    if (dlg->exec() == KDialog::Accepted) {
        // added by Wellu Mäkinen <wellu@wellu.org>
        //
        // get rid of leading whitespace
        // that is %20 in encoded form
        QString url = dlg->selectedUrl().prettyUrl();

        // decodes %20 to normal spaces
        // trims the whitespace from both ends
        // of the URL
        ret = KUrl(url.trimmed());
    }
    delete dlg;
    return ret;
}
