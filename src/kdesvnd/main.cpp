/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht                                  *
 *   rajko.albrecht@tecways.com                                            *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <dcopclient.h>
#include "kdesvnd_dcop.h"

static const char description[] =
    I18N_NOOP("Kdesvn DCOP service");

static const char version[] = "0.1";

static KCmdLineOptions options[] =
{
    KCmdLineLastOption
};

int main (int argc, char *argv[])
{
    KLocale::setMainCatalogue("kdesvn");
    KAboutData aboutdata("kdesvnd", I18N_NOOP("KDE"),
                version, description,
                KAboutData::License_GPL, "(C) %{YEAR}, Rajko Albrecht");
    aboutdata.addAuthor("Rajko Albrecht",I18N_NOOP("Developer"),"rajko.albrecht@tecways.com");

    KCmdLineArgs::init( argc, argv, &aboutdata );
    KCmdLineArgs::addCmdLineOptions( options );
    KUniqueApplication::addCmdLineOptions();

    if (!KUniqueApplication::start())
    {
        kdDebug() << "kdesvnd is already running!" << endl;
        return (0);
    }

    KUniqueApplication app;
    kdDebug() << "starting kdesvnd_dcop " << endl;
    // This app is started automatically, no need for session management
    app.disableSessionManagement();
    kdesvnd_dcop *service = new kdesvnd_dcop;
    kdDebug() << "starting kdesvnd_dcop " << endl;
    return app.exec();

}
