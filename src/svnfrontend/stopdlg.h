/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht   *
 *   ral@alwins-world.de   *
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
#ifndef STOPDLG_H
#define STOPDLG_H

#include <kdialogbase.h>

class QTimer;

class CContextListener;
class QLabel;
/**
@author Rajko Albrecht
*/
class StopDlg : public KDialogBase
{
Q_OBJECT
public:
    StopDlg(CContextListener*,QWidget *parent = 0, const char *name = 0,const QString&caption=QString::null,const QString&text=QString::null);
    ~StopDlg();

    bool cancelld();

protected:
    CContextListener*m_Context;
    int m_MinDuration;
    bool mCancelled;
    QTimer * mShowTimer;
    QString mCancelText;
    bool mShown;
    QLabel*mLabel;

protected slots:
    virtual void slotAutoShow();
    virtual void slotCancel();
};

#endif
