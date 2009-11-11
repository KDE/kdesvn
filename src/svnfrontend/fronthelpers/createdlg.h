/***************************************************************************
 *   Copyright (C) 2008 by Rajko Albrecht  ral@alwins-world.de             *
 *   http://kdesvn.alwins-world.de/                                        *
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
#ifndef CREATEDLG_H
#define CREATEDLG_H

#include "src/settings/kdesvnsettings.h"
#include <kdialog.h>
#include <kconfig.h>
#include <kvbox.h>
#include <kapplication.h>

#include <QString>

template<class T> inline KDialog* createDialog(T**ptr,const QString&_head,const QFlags<KDialog::ButtonCode>&_buttons,const char*name="standard_dialog",bool showHelp=false,bool modal=true,const KGuiItem&u1=KGuiItem())
{
    QFlags<KDialog::ButtonCode> buttons = _buttons;
    if (showHelp) {
        buttons = buttons|KDialog::Help;
    }
    if (!u1.text().isEmpty()) {
        buttons=buttons|KDialog::User1;
    }
    KDialog * dlg = new KDialog(modal?KApplication::activeModalWidget():0);
    if (!dlg) return dlg;
    dlg->setCaption(_head);
    dlg->setModal(modal);
    dlg->setButtons(buttons);
    if (!u1.text().isEmpty()) {
        dlg->setButtonGuiItem(KDialog::User1,u1);
    }
    if (name) {
        dlg->setObjectName(name);
    }

    QWidget* Dialog1Layout = new KVBox(dlg);
    dlg->setMainWidget(Dialog1Layout);
    *ptr = new T(Dialog1Layout);
    KConfigGroup _k(Kdesvnsettings::self()->config(),name?name:"standard_size");
    dlg->restoreDialogSize(_k);
    return dlg;
}

template<class T> inline KDialog*createYesDialog(T**ptr,const QString&_head,bool YesNo=false,const char*name="standard_dialog",bool showHelp=false,bool modal=true,const KGuiItem&u1=KGuiItem())
{
    QFlags<KDialog::ButtonCode> buttons = KDialog::Yes;
    if (YesNo) {
        buttons = buttons|KDialog::No;
    }
    return createDialog(ptr,_head,buttons,name,showHelp,modal,u1);
}

template<class T> inline KDialog*createOkDialog(T**ptr,const QString&_head,bool OkCancel=false,const char*name="standard_dialog",bool showHelp=false,bool modal=true,const KGuiItem&u1=KGuiItem())
{
    QFlags<KDialog::ButtonCode> buttons = KDialog::Ok;
    if (OkCancel) {
        buttons = buttons|KDialog::Cancel;
    }
    return createDialog(ptr,_head,buttons,name,showHelp,modal,u1);
}

#endif
