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

static KCmdLineOptions options[] =
{
    { "r startrev[:endrev]",I18N_NOOP("Execute single subversion command on specific revision(-range)"),0},
    {"R",I18N_NOOP("Ask for revision when executing single command"),0},
    {"f",I18N_NOOP("Force operation"),0},
    {"o <file>",I18N_NOOP("Save output of subversion command (eg \"cat\") into file <file>"),0},
    {"l <number>",I18N_NOOP("Limit log output to <number>"),0},
    { "+exec <command>",I18N_NOOP("Execute subversion command (\"exec help\" for more information)"),0},
    { "+[URL]", I18N_NOOP( "Document to open" ), 0 },
    KCmdLineLastOption
};

int main(int argc, char **argv)
{
    KAboutData about("kdesvn", I18N_NOOP("kdesvn"), version, description,
                     KAboutData::License_GPL, "(C) 2005-2007 Rajko Albrecht",0,
                         0, "ral@alwins-world.de");
    about.addAuthor( "Rajko Albrecht", 0, "ral@alwins-world.de" );
    about.setHomepage("http://www.alwins-world.de/wiki/programs/kdesvn/");
    about.setBugAddress("kdesvn-bugs@alwins-world.de");

    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);


    KApplication app;

    // see if we are starting with session management
    if (app.isRestored())
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
                kdDebug()<<"Execute a command" << endl;
                CommandLine cl(args);
                return cl.exec();
            } else {
                int i = 0;
                for (; i < args->count(); i++)
                {
                    kdesvn *widget = new kdesvn;
                    widget->show();
                    widget->load(args->url(i));
                }
            }
        }
        args->clear();
    }
    return app.exec();
}
