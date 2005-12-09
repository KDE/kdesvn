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
