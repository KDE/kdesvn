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
#include <qregexp.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kpassworddialog.h>
#include <kdebug.h>
#include <kwallet.h>
#include <KLocalizedString>
#include <QPointer>
#include <QTextStream>

int main(int argc, char **argv)
{
    KAboutData about(QByteArray("kdesvnaskpass"), QByteArray("kdesvnaskpass"), ki18n("kdesvnaskpass"), QByteArray("0.2"),
                     ki18n("ssh-askpass for kdesvn"),
                     KAboutData::License_LGPL,
                     ki18n("Copyright (c) 2005-2009 Rajko Albrecht"));
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineOptions options;
    options.add("+[prompt]", ki18n("Prompt"));
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;
    // no need for session management
    app.disableSessionManagement();
    KGlobal::locale()->insertCatalog("kdesvn");

    QString prompt;
    QString kfile;
    bool error = false;

    if (!KCmdLineArgs::parsedArgs()->count()) {
        prompt = i18n("Please enter your password below.");
    } else {
        prompt = KCmdLineArgs::parsedArgs()->arg(0);
        if (prompt.contains("Bad passphrase", Qt::CaseInsensitive) ||
                prompt.contains("Permission denied", Qt::CaseInsensitive)) {
            error = true;
        }
        kfile = prompt.section(QLatin1Char(' '), -2).remove(QLatin1Char(':')).simplified();
    }
    QString pw;
    QString wfolder = about.appName();

    QScopedPointer<KWallet::Wallet> wallet(KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), 0));
    if (!error && wallet && wallet->hasFolder(wfolder)) {
        wallet->setFolder(wfolder);
        wallet->readPassword(kfile, pw);
    }

    if (pw.isEmpty()) {
        QPointer<KPasswordDialog> dlg(new KPasswordDialog(0, (wallet ? KPasswordDialog::ShowKeepPassword : KPasswordDialog::NoFlags)));
        dlg->setPrompt(prompt);
        dlg->setCaption(i18n("Password"));
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
    pw.replace(0, pw.length(), "0");
    return 0;
}

