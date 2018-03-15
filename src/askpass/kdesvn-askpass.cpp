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
#include <KAboutData>
#include <KLocalizedString>
#include <KPasswordDialog>
#include <KWallet>

#include <QPointer>
#include <QTextStream>
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("kdesvn");
    KAboutData aboutData(QStringLiteral("kdesvnaskpass"), i18n("kdesvnaskpass"), QStringLiteral("0.2"),
                         i18n("ssh-askpass for kdesvn"),
                         KAboutLicense::LicenseKey::LGPL,
                         i18n("Copyright (c) 2005-2009 Rajko Albrecht"));
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addPositionalArgument(QStringLiteral("[prompt]"), i18n("Prompt"));
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);
    // no need for session management
    //app.disableSessionManagement();

    QString prompt;
    QString kfile;
    bool error = false;

    if (parser.positionalArguments().isEmpty()) {
        prompt = i18n("Please enter your password below.");
    } else {
        prompt = parser.positionalArguments().at(0);
        if (prompt.contains(QLatin1String("Bad passphrase"), Qt::CaseInsensitive) ||
                prompt.contains(QLatin1String("Permission denied"), Qt::CaseInsensitive)) {
            error = true;
        }
        kfile = prompt.section(QLatin1Char(' '), -2).remove(QLatin1Char(':')).simplified();
    }
    QString pw;
    QString wfolder = aboutData.productName();

    QScopedPointer<KWallet::Wallet> wallet(KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), 0));
    if (!error && wallet && wallet->hasFolder(wfolder)) {
        wallet->setFolder(wfolder);
        wallet->readPassword(kfile, pw);
    }

    if (pw.isEmpty()) {
        QPointer<KPasswordDialog> dlg(new KPasswordDialog(nullptr, (wallet ? KPasswordDialog::ShowKeepPassword : KPasswordDialog::NoFlags)));
        dlg->setPrompt(prompt);
        dlg->setWindowTitle(i18nc("@title:window", "Password"));
        if (dlg->exec() != KPasswordDialog::Accepted) {
            delete dlg;
            return 1;
        }
        pw = dlg->password();
        if (wallet && dlg->keepPassword()) {
            if (!wallet->hasFolder(wfolder)) {
                wallet->createFolder(wfolder);
            }
            wallet->setFolder(wfolder);
            wallet->writePassword(kfile, pw);
        }
        delete dlg;
    }

    QTextStream out(stdout);
    out << pw;
    /* cleanup memory */
    pw.replace(0, pw.length(), QLatin1Char('0'));
    return 0;
}

