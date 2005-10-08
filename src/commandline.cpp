/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht                                  *
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
#include <kcmdlineargs.h>
#include <kdialogbase.h>
#include <ktextbrowser.h>
#include <klocale.h>
#include <qstring.h>
#include <qlayout.h>
#include <qvbox.h>

class CommandLineData
{
public:
    CommandLineData():cmd(""){};
    virtual ~CommandLineData(){};

    void displayHelp();

    static QString genericText();
    QString cmd;

};

CommandLine::CommandLine(KCmdLineArgs*_args)
{
    m_args = _args;
    m_data = new CommandLineData;
}

CommandLine::~CommandLine()
{
}

int CommandLine::exec()
{
    if (!m_args||m_args->count()<1) {
        return -1;
    }
    if (m_args->count()<2) {
        m_data->cmd = "help";
    } else {
        m_data->cmd = m_args->arg(1);
    }
    if (m_data->cmd=="help") {
        m_data->displayHelp();
        return 0;
    }
    KLibFactory *factory = KLibLoader::self()->factory("libkdesvnpart");
    if (factory) {
        if (QCString(factory->className())!="cFactory") {
            kdDebug()<<"wrong factory"<<endl;
            return -1;
        }
        cFactory*cfa = static_cast<cFactory*>(factory);
        QStringList s;
        for (int i = 1; i < m_args->count(); i++) {
            s.append(m_args->arg(i));
        }
        commandline_part * cpart = cfa->createCommandIf((QObject*)0,(const char*)0,s);
        int res = cpart->exec();
        return res;
    }
    return 0;
}

void CommandLineData::displayHelp()
{
    KDialogBase * dlg = new KDialogBase(
        0,
        "help_dlg",
        true,
        "Commandline help",
        KDialogBase::Ok/*,
        (OkCancel?KDialogBase::Cancel:KDialogBase::Close),
        KDialogBase::Cancel,
        true*//*,(OkCancel?KStdGuiItem::ok():KStdGuiItem::close())*/);

    if (!dlg) return;
    QWidget* Dialog1Layout = dlg->makeVBoxMainWidget();
    KTextBrowser*ptr = new KTextBrowser(Dialog1Layout);
    QString text;
    text = "<html><head></head><body>";
    text+=I18N_NOOP("<code>kdesvn exec &lt;command&gt;&nbsp;&lt;param&gt;&nbsp;path</code><br>");
    text+=genericText();
    text+="</body></html>";
    ptr->setText(text);
    dlg->resize(600,400);
    dlg->exec();
    delete dlg;
//    dlg->resize(dlg->configDialogSize(*(Settings::self()->config()),"help_window"));
}

QString CommandLineData::genericText()
{
    QString result = "";
    static QString br = "<br>";
    static QString lb = "<tr><td>";
    static QString ts = "</td><td>";
    static QString le = "</td></tr>\n";


    result+=br;
    result+="<table><tr><th colspan=\"2\">";
    result+=I18N_NOOP("Commands known");
    result+="</th></tr>\n";
    result+=lb;
    result+=QString("cat")+ts+I18N_NOOP("Get content and print them out to stdout or file")+le;
    result+=lb+QString("log")+ts+I18N_NOOP("Display log of item")+le;
    result+="</table>";
    return result;
}
