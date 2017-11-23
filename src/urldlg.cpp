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
#include "ui_urldlg.h"

#include <KUrlRequester>
#include <KConfigGroup>
#include <KHistoryComboBox>
#include <KLocalizedString>
#include <KSharedConfig>
#include <QLabel>
#include <QVBoxLayout>

UrlDlg::UrlDlg(QWidget *parent)
    : QDialog(parent)
    , m_urlRequester(nullptr)
    , m_ui(new Ui::UrlDlg)
{
    m_ui->setupUi(this);
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setShortcut(Qt::CTRL | Qt::Key_Return);

    KHistoryComboBox *combo = new KHistoryComboBox(this);
    combo->setDuplicatesEnabled(false);
    KConfigGroup kc = KSharedConfig::openConfig()->group("Open-repository settings");
    int max = kc.readEntry(QLatin1String("Maximum history"), 15);
    combo->setMaxCount(max);
    const QStringList list = kc.readEntry(QLatin1String("History"), QStringList());
    combo->setHistoryItems(list);
    combo->setMinimumWidth(100);
    combo->adjustSize();
    if (combo->width() > 300) {
        combo->resize(300, combo->height());
    }

    m_urlRequester = new KUrlRequester(combo, this);
    m_ui->topLayout->insertWidget(1, m_urlRequester);
    m_urlRequester->setFocus();
    m_urlRequester->setMode(KFile::ExistingOnly | KFile::Directory);
    connect(m_urlRequester->comboBox(), SIGNAL(currentTextChanged(QString)),
            this, SLOT(slotTextChanged(QString)));

    slotTextChanged(QString());
    m_urlRequester->adjustSize();

    connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
}

UrlDlg::~UrlDlg()
{
    delete m_ui;
}

void UrlDlg::accept()
{
    KHistoryComboBox *combo = static_cast<KHistoryComboBox *>(m_urlRequester->comboBox());
    if (combo) {
        combo->addToHistory(m_urlRequester->url().url());
        KConfigGroup kc = KSharedConfig::openConfig()->group("Open-repository settings");
        kc.writeEntry(QLatin1String("History"), combo->historyItems());
        kc.sync();
    }
    QDialog::accept();
}

/*!
    \fn UrlDlg::slotTextChanged(const QString&)
 */
void UrlDlg::slotTextChanged(const QString &text)
{
    bool state = !text.trimmed().isEmpty();
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(state);
}

/*!
    \fn UrlDlg::getUrl(QWidget*parent)
 */
QUrl UrlDlg::getUrl(QWidget *parent)
{
    QUrl ret;
    QPointer<UrlDlg> dlg(new UrlDlg(parent));
    dlg->setWindowTitle(i18nc("@title:window", "Open"));
    if (dlg->exec() == QDialog::Accepted) {
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
