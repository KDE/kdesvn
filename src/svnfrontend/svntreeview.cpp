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
#include <QDropEvent>
#include <QApplication>
#include <QAction>
#include <QMenu>

#include <KIcon>
#include <KIconLoader>
#include <KDebug>
#include <KUrl>
#include <KLocale>

SvnTreeView::SvnTreeView(QWidget *parent)
    : QTreeView(parent)
{

}

SvnTreeView::~SvnTreeView()
{
}

void SvnTreeView::startDrag(Qt::DropActions supportedActions)
{
    // only one dragging at time
    static bool isDrag = false;
    if (isDrag) {
        return;
    }
    isDrag = true;
    QModelIndexList indexes = selectionModel()->selectedRows();
    if (indexes.count() > 0) {
        QMimeData *data = model()->mimeData(indexes);
        if (data == 0) {
            isDrag = false;
            return;
        }
        QDrag *drag = new QDrag(this);
        QPixmap pixmap;
        if (indexes.count() == 1) {
            QAbstractProxyModel *proxyModel = static_cast<QAbstractProxyModel *>(model());
            SvnItemModel *itemModel = static_cast<SvnItemModel *>(proxyModel->sourceModel());
            const QModelIndex index = proxyModel->mapToSource(indexes.first());

            SvnItemModelNode *item = itemModel->nodeForIndex(index);
            pixmap = item->getPixmap(KIconLoader::SizeMedium, false);
        } else {
            pixmap = KIcon("document-multiple").pixmap(KIconLoader::SizeMedium, KIconLoader::SizeMedium);
        }
        drag->setPixmap(pixmap);
        drag->setMimeData(data);
        drag->exec(supportedActions, Qt::IgnoreAction);
    }
    isDrag = false;
}

void SvnTreeView::dropEvent(QDropEvent *event)
{
    if (!KUrl::List::canDecode(event->mimeData())) {
        return;
    }

    QAbstractProxyModel *proxyModel = static_cast<QAbstractProxyModel *>(model());

    const QModelIndex index = indexAt(event->pos());
    QModelIndex index2;
    QMap<QString, QString> metaMap;
    if (index.isValid()) {
        index2 = proxyModel->mapToSource(index);
    }

    Qt::DropAction action = event->dropAction();
    KUrl::List list = KUrl::List::fromMimeData(event->mimeData(), &metaMap);
    bool intern = false;
    if (metaMap.find("kdesvn-source") != metaMap.end()) {
        SvnItemModel *itemModel = static_cast<SvnItemModel *>(proxyModel->sourceModel());
        QMap<QString, QString>::const_iterator it = metaMap.constFind("kdesvn-id");
        if (it != metaMap.constEnd() && it.value() == itemModel->uniqueIdentifier()) {
            intern = true;
        }
    }
    Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();

    QMetaObject::invokeMethod(this, "doDrop",
                              Q_ARG(KUrl::List, list),
                              Q_ARG(QModelIndex, index2),
                              Q_ARG(bool, intern),
                              Q_ARG(Qt::DropAction, action),
                              Q_ARG(Qt::KeyboardModifiers, modifiers)
                             );
    event->acceptProposedAction();
}

void SvnTreeView::doDrop(const KUrl::List &list, const QModelIndex &parent, bool intern, Qt::DropAction action, Qt::KeyboardModifiers modifiers)
{
    if (intern && ((modifiers & Qt::ControlModifier) == 0) &&
            ((modifiers & Qt::ShiftModifier) == 0)) {

        QMenu popup;
        QString seq = QKeySequence(Qt::ShiftModifier).toString();
        seq.chop(1); // chop superfluous '+'
        QAction *popupMoveAction = new QAction(i18n("&Move Here") + '\t' + seq, this);
        popupMoveAction->setIcon(KIcon("go-jump"));
        seq = QKeySequence(Qt::ControlModifier).toString();
        seq.chop(1);
        QAction *popupCopyAction = new QAction(i18n("&Copy Here") + '\t' + seq, this);
        popupCopyAction->setIcon(KIcon("edit-copy"));
        QAction *popupCancelAction = new QAction(i18n("C&ancel") + '\t' + QKeySequence(Qt::Key_Escape).toString(), this);
        popupCancelAction->setIcon(KIcon("process-stop"));

        popup.addAction(popupMoveAction);
        popup.addAction(popupCopyAction);
        popup.addSeparator();
        popup.addAction(popupCancelAction);
        QAction *result = popup.exec(QCursor::pos());

        if (result == popupCopyAction) {
            action = Qt::CopyAction;
        } else if (result == popupMoveAction) {
            action = Qt::MoveAction;
        } else if (result == popupCancelAction || !result) {
            return;
        }
    }

    QAbstractProxyModel *proxyModel = static_cast<QAbstractProxyModel *>(model());
    SvnItemModel *itemModel = static_cast<SvnItemModel *>(proxyModel->sourceModel());
    QModelIndex _p;
    if (!parent.isValid() && (_p = rootIndex()).isValid()) {
        QAbstractProxyModel *proxyModel = static_cast<QAbstractProxyModel *>(model());
        _p = proxyModel->mapToSource(_p);
    } else {
        _p = parent;
    }
    itemModel->dropUrls(list, action, parent.row(), parent.column(), _p, intern);
}
