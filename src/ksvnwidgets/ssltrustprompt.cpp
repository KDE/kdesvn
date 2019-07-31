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
#include "ssltrustprompt.h"
#include "ui_ssltrustprompt.h"

#include <KLocalizedString>
#include <QPointer>
#include <QPushButton>

SslTrustPrompt::SslTrustPrompt(const QString &host, const QString &text, QWidget *parent)
    : KSvnDialog(QLatin1String("trustssldlg"), parent)
    , m_ui(new Ui::SslTrustPrompt)
{
    m_ui->setupUi(this);
    setDefaultButton(m_ui->buttonBox->button(QDialogButtonBox::No));
    m_ui->buttonBox->button(QDialogButtonBox::Yes)->setText(i18n("Accept permanently"));
    m_ui->buttonBox->button(QDialogButtonBox::No)->setText(i18n("Accept temporarily"));
    m_ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(i18n("Reject"));
    connect(m_ui->buttonBox->button(QDialogButtonBox::Yes), &QPushButton::clicked,
            this, [this]() {done(QDialogButtonBox::Yes);});
    connect(m_ui->buttonBox->button(QDialogButtonBox::No), &QPushButton::clicked,
            this, [this]() {done(QDialogButtonBox::No);});
    connect(m_ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
            this, [this]() {done(QDialogButtonBox::Cancel);});

    m_ui->m_MainLabel->setText(QLatin1String("<p align=\"center\"><b>") +
                               i18n("Error validating server certificate for '%1'", host) +
                               QLatin1String("</b></p>"));
    m_ui->m_ContentText->setText(text);
}

SslTrustPrompt::~SslTrustPrompt()
{
    delete m_ui;
}

bool SslTrustPrompt::sslTrust(const QString &host,
                              const QString &fingerprint,
                              const QString &validFrom,
                              const QString &validUntil,
                              const QString &issuerName,
                              const QString &realm,
                              const QStringList &reasons,
                              bool *ok,
                              bool *saveit)
{
    static QLatin1String rb("<tr><td>");
    static QLatin1String rs("</td><td>");
    static QLatin1String re("</td></tr>");
    QString text = QStringLiteral("<html><body>");
    if (!reasons.isEmpty()) {
        text += QStringLiteral("<p align=\"center\"><h2>") + i18n("Failure reasons") + QStringLiteral("</h2><hline>");
        for (const QString &reason : reasons) {
            text += reason + QStringLiteral("<br/><hline>");
        }
        text += QStringLiteral("</p>");
    }

    text += QStringLiteral("<p align=\"center\"><table>");
    text += rb + i18n("Realm") + rs + realm + re;
    text += rb + i18n("Host") + rs + host + re;
    text += rb + i18n("Valid from") + rs + validFrom + re;
    text += rb + i18n("Valid until") + rs + validUntil + re;
    text += rb + i18n("Issuer name") + rs + issuerName + re;
    text += rb + i18n("Fingerprint") + rs + fingerprint + re;
    text += QStringLiteral("</table></p></body></html>");

    QPointer<SslTrustPrompt> dlg(new SslTrustPrompt(host, text, QApplication::activeModalWidget()));
    int i = dlg->exec();
    delete dlg;

    *saveit = i == QDialogButtonBox::Yes;
    *ok = (i == QDialogButtonBox::Yes || i == QDialogButtonBox::No);
    return *ok;
}
