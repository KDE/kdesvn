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
#include "kmultilinedelegate.h"

#include <ktextedit.h>

KMultilineDelegate::KMultilineDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

QWidget *KMultilineDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &/* option */,
        const QModelIndex &/* index */) const
{
    KTextEdit *editor = new KTextEdit(parent);
    //editor->setMinimumHeight(35);
    return editor;
}

void KMultilineDelegate::setEditorData(QWidget *editor,
                                       const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();

    KTextEdit *editBox = static_cast<KTextEdit *>(editor);
    editBox->setText(value);
}

void KMultilineDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                      const QModelIndex &index) const
{
    KTextEdit *editBox = static_cast<KTextEdit *>(editor);
    model->setData(index, editBox->toPlainText(), Qt::EditRole);
}

void KMultilineDelegate::updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

QSize KMultilineDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index)const
{
    QSize s = QItemDelegate::sizeHint(option, index);
    if (s.height() < 35) {
        s.setHeight(35);
    }
    return s;
}
