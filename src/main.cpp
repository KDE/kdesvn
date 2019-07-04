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

#include "kdesvn.h"
#include "commandline.h"
#include "kdesvn-config.h"
#include <QApplication>
#include <KAboutData>

#include <QCommandLineParser>
#include <QDir>
#include <klocalizedstring.h>

static const char description[] =
    I18N_NOOP("A Subversion Client by KDE (standalone application)");

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("kdesvn");
    app.setApplicationName(QStringLiteral("kdesvn"));
    app.setApplicationDisplayName(QStringLiteral("kdesvn"));
    app.setOrganizationDomain(QStringLiteral("kde.org"));
    app.setApplicationVersion(QStringLiteral(KDESVN_VERSION));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("kdesvn"), app.windowIcon()));

    KAboutData aboutData(QStringLiteral("kdesvn"), i18n("kdesvn"), QStringLiteral(KDESVN_VERSION), i18n(description),
                         KAboutLicense::GPL, i18n("(C) 2005-2009 Rajko Albrecht,\n(C) 2015-2019 Christian Ehrlicher"));
    aboutData.addAuthor(i18n("Rajko Albrecht"), i18n("Developer"), QStringLiteral("ral@alwins-world.de"));
    aboutData.addAuthor(i18n("Christian Ehrlicher"), i18n("Developer"), QStringLiteral("ch.ehrlicher@gmx.de"));
    aboutData.setHomepage(QStringLiteral("https://commits.kde.org/kdesvn"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption({QStringLiteral("r"), i18n("Execute single Subversion command on specific revision(-range)"), i18n("startrev[:endrev]")});
    parser.addOption({QStringLiteral("R"), i18n("Ask for revision when executing single command")});
    parser.addOption({QStringLiteral("f"), i18n("Force operation")});
    parser.addOption({QStringLiteral("o"), i18n("Save output of Subversion command (eg \"cat\") into file <file>"), i18n("<file>")});
    parser.addOption({QStringLiteral("l"), i18n("Limit log output to <number>"), i18n("<number>")});
    parser.addPositionalArgument(QStringLiteral("+exec <command>"), i18n("Execute Subversion command (\"exec help\" for more information)"));
    parser.addPositionalArgument(QStringLiteral("+[URL]"), i18n("Document to open"));
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    // see if we are starting with session management
    if (app.isSessionRestored()) {
        RESTORE(kdesvn);
    } else {
        // no session.. just start up normally
        if (parser.positionalArguments().isEmpty()) {
            kdesvn *widget = new kdesvn;
            widget->show();
            widget->checkReload();
        } else {
            if (parser.positionalArguments().at(0) == QLatin1String("exec")) {
                CommandLine cl(&parser);
                return cl.exec();
            } else {
                int i = 0;
                for (; i < parser.positionalArguments().count(); i++) {
                    kdesvn *widget = new kdesvn;
                    widget->show();
                    widget->load(QUrl::fromUserInput(parser.positionalArguments().at(i),
                                                     QDir::currentPath()), true);
                }
            }
        }
    }
    return app.exec();
}
