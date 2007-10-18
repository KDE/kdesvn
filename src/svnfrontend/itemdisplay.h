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

#ifndef __ITEMDISPLAY_H
#define __ITEMDISPLAY_H

#include "src/svnqt/svnqttypes.hpp"
#include <q3ptrlist.h>
#include <qstring.h>
#include <kurl.h>

class QWidget;

class SvnItem;

namespace svn
{
    class Status;
}

class ItemDisplay
{
public:
    ItemDisplay();
    virtual ~ItemDisplay(){}
    virtual bool isWorkingCopy()const;
    virtual QWidget*realWidget() = 0;
    virtual SvnItem*Selected()=0;
    virtual void SelectionList(Q3PtrList<SvnItem>*)=0;
    virtual const QString&baseUri()const;
    virtual bool openURL( const KUrl &url,bool noReinit=false )=0;
    virtual SvnItem*SelectedOrMain()=0;
    virtual bool isNetworked()const;
    virtual const QString&lastError()const;
    virtual bool filterOut(const SvnItem*);
    virtual bool filterOut(const svn::StatusPtr&);
    QString relativePath(const SvnItem*item);

protected:
    void setWorkingCopy(bool);
    void setNetworked(bool);
    void setBaseUri(const QString&);
    QString m_LastException;

private:
    bool m_isWorkingCopy;
    bool m_isNetworked;
    QString m_baseUri;
};

#endif
