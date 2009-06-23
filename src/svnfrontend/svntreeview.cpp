/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
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

#include "svntreeview.h"
#include "models/svnitemmodel.h"
#include "models/svnitemnode.h"

#include <QDrag>
#include <QAbstractProxyModel>

#include <KIcon>
#include <KIconLoader>
#include <KDebug>

SvnTreeView::SvnTreeView(QWidget * parent)
    :QTreeView(parent)
{

}

SvnTreeView::~SvnTreeView()
{
}

void SvnTreeView::startDrag(Qt::DropActions supportedActions)
{
    // only one dragging at time
    static bool isDrag = false;
    if (isDrag) return;
    isDrag = true;
    QModelIndexList indexes = selectionModel()->selectedRows();
    if (indexes.count()>0) {
        QMimeData * data = model()->mimeData(indexes);
        if (data == 0) {
            isDrag = false;
            return;
        }
        QDrag* drag = new QDrag(this);
        QPixmap pixmap;
        if (indexes.count() == 1) {
            QAbstractProxyModel* proxyModel = static_cast<QAbstractProxyModel*>(model());
            SvnItemModel* itemModel = static_cast<SvnItemModel*>(proxyModel->sourceModel());
            const QModelIndex index = proxyModel->mapToSource(indexes.first());

            SvnItemModelNode*item = itemModel->nodeForIndex(index);
            pixmap = item->getPixmap(KIconLoader::SizeMedium,false);
        } else {
            kDebug()<<"Multi pix"<<endl;
            pixmap = KIcon("document-multiple").pixmap(KIconLoader::SizeMedium,KIconLoader::SizeMedium);
        }
        drag->setPixmap(pixmap);
        drag->setMimeData(data);
        drag->exec(supportedActions, Qt::IgnoreAction);
    }
    isDrag = false;
}

#include "svntreeview.moc"
