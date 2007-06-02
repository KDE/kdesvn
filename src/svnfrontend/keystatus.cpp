/***************************************************************************
 *   Copyright (C) 2006-2007 by Rajko Albrecht                             *
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
#include "keystatus.h"

#include <qwidget.h>
#include <X11/Xlib.h>

void KeyState::keystate(int*root_x,int*root_y,int*win_x,int*win_y,unsigned int*keybstate)
{
        Window root;
        Window child;
        unsigned int kstate;
        XQueryPointer( qt_xdisplay(), qt_xrootwin(), &root, &child,
                       root_x, root_y, win_x, win_y, &kstate);
        *keybstate=0;
        if (kstate&ControlMask) {
            *keybstate|=Qt::ControlButton;
        }
        if (kstate&ShiftMask) {
            *keybstate|=Qt::ShiftButton;
        }
}
