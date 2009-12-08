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
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>

static const char description[] =
    I18N_NOOP("A Subversion Client for KDE (standalone application)");

static const char version[] = VERSION;


int main(int argc, char **argv)
{
    KAboutData about(QByteArray("kdesvn"),QByteArray("kdesvn"),ki18n("kdesvn"),QByteArray(version),ki18n(description),
                     KAboutData::License_GPL,ki18n("(C) 2005-2009 Rajko Albrecht"));
    about.addAuthor( ki18n("Rajko Albrecht"),ki18n("Developer"),QByteArray("ral@alwins-world.de"),QByteArray());
    about.setHomepage("http://kdesvn.alwins-world.de/");
    about.setBugAddress("kdesvn-bugs@alwins-world.de");

    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineOptions options;
    options.add("r startrev[:endrev]",ki18n("Execute single subversion command on specific revision(-range)"));
    options.add("R",ki18n("Ask for revision when executing single command"));
    options.add("f",ki18n("Force operation"));
    options.add("o <file>",ki18n("Save output of subversion command (eg \"cat\") into file <file>"));
    options.add("l <number>",ki18n("Limit log output to <number>"));
    options.add("+exec <command>",ki18n("Execute subversion command (\"exec help\" for more information)"));
    options.add("+[URL]", ki18n( "Document to open" ));
    KCmdLineArgs::addCmdLineOptions(options);


    KApplication app;

    // see if we are starting with session management
    if (app.isSessionRestored())
    {
        RESTORE(kdesvn);
    }
    else
    {
        // no session.. just start up normally
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
        if (args->count() == 0)
        {
            kdesvn *widget = new kdesvn;
            widget->show();
            widget->checkReload();
        }
        else
        {
            if (QString(args->arg(0))==QString("exec")) {
                CommandLine cl(args);
                return cl.exec();
            } else {
                int i = 0;
                for (; i < args->count(); i++)
                {
                    kdesvn *widget = new kdesvn;
                    widget->show();
                    widget->load(args->url(i),true);
                }
            }
        }
        args->clear();
    }
    return app.exec();
}
