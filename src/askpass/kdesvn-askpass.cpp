/***************************************************************************
 *   Copyright (C) 2005-2007 by Rajko Albrecht                             *
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
#include <klocale.h>
#include <kpassworddialog.h>

#include <iostream>

int main(int argc, char** argv)
{
    KAboutData about(QByteArray("kdesvnaskpass"),QByteArray("kdesvnaskpass"),ki18n("kdesvnaskpass"),QByteArray("0.1"),
                    ki18n("ssh-askpass for kdesvn"),
                    KAboutData::License_LGPL,
                    ki18n("Copyright (c) 2005 Rajko Albrecht"));
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineOptions options;
    options.add("+[prompt]",ki18n("Prompt"));
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;
    // no need for session management
    app.disableSessionManagement();
    QString prompt;

    if( !KCmdLineArgs::parsedArgs()->count() ) {
        prompt = i18n("Please enter your password below.");
    } else {
        prompt = KCmdLineArgs::parsedArgs()->arg(0);
    }
    QString pw;
    KPasswordDialog dlg;
    dlg.setPrompt(prompt);
    dlg.setCaption(i18n("SSH password"));
    if (dlg.exec()==KPasswordDialog::Accepted) {
        pw = dlg.password();
        std::cout << pw.toUtf8().data() << std::endl;
        /* cleanup memory */
        pw.replace(0,pw.length(),"0");
        return 0;
    }
    return 1;
}

