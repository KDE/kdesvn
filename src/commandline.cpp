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
#include <KHelpClient>
#include <QCommandLineParser>

#include "commandexec.h"

CommandLine::CommandLine(const QCommandLineParser *parser)
    : m_parser(parser)
{
}

CommandLine::~CommandLine()
{
}

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

    CommandExec exec(nullptr);

    return exec.exec(m_parser);
}

void CommandLine::displayHelp()
{
    KHelpClient::invokeHelp(QStringLiteral("kdesvn-commandline"), QStringLiteral("kdesvn"));
}
