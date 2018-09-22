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
#ifndef REVISIONTREE_H
#define REVISIONTREE_H

#include "svnqt/log_entry.h"
#include "svnqt/revision.h"
#include "svnqt/client.h"

#include <qstring.h>

class CContextListener;
class RtreeData;
class RevTreeWidget;

namespace svn
{
class Client;
}

/**
    @author Rajko Albrecht <ral@alwins-world.de>
*/
class RevisionTree final
{
public:
    Q_DISABLE_COPY(RevisionTree)
    RevisionTree(const svn::ClientP &,
                 CContextListener *aListener,
                 const QString &reposRoot,
                 const svn::Revision &startr, const svn::Revision &endr,
                 const QString &, const svn::Revision &baserevision,
                 QWidget *parent = nullptr);
    ~RevisionTree();

    bool isValid()const;
    RevTreeWidget *getView();

protected:
    long m_Baserevision;
    long m_InitialRevsion;
    QString m_Path;
    bool m_Valid;

    RtreeData *m_Data;

    bool topDownScan();
    bool bottomUpScan(long startrev, unsigned recurse, const QString &path, long sRev = -1);
    bool isDeleted(long revision, const QString &);

    static bool isParent(const QString &_par, const QString &tar);

    void fillItem(long revIndex, int pathIndex, const QString &nodeName, const QString &path);
};

#endif
