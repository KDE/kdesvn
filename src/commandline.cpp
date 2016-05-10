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
#include "commandline.h"
#include "kdesvn_part.h"
#include "commandline_part.h"
#include <KPluginFactory>
#include <KPluginLoader>
#include <KHelpClient>
#include <QCommandLineParser>

CommandLine::CommandLine(const QCommandLineParser *parser)
    : m_parser(parser)
{}

CommandLine::~CommandLine()
{}

int CommandLine::exec()
{
    if (m_parser->positionalArguments().isEmpty()) {
        return -1;
    }
    if (m_parser->positionalArguments().count() < 2) {
        cmd = QLatin1String("help");
    } else {
        cmd = m_parser->positionalArguments().at(1);
    }
    if (cmd == QLatin1String("help")) {
        displayHelp();
        return 0;
    }
#ifdef EXTRA_KDE_LIBPATH
    QCoreApplication::addLibraryPath(QString::fromLocal8Bit(EXTRA_KDE_LIBPATH));
#endif
    KPluginLoader loader("kdesvnpart");
    KPluginFactory *factory = loader.factory();
    if (factory) {
        QObject *_p = (factory->create<QObject>("commandline_part", nullptr));
        if (!_p || QString::fromLatin1(_p->metaObject()->className()) != QLatin1String("commandline_part")) {
            return 0;
        }
        commandline_part *cpart = static_cast<commandline_part *>(_p);
        int res = cpart->exec(m_parser);
        return res;
    }
    return 0;
}

void CommandLine::displayHelp()
{
    KHelpClient::invokeHelp(QLatin1String("kdesvn-commandline"), QLatin1String("kdesvn"));
}
