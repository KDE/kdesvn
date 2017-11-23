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


#include "copymoveview_impl.h"
#include "ksvnwidgets/ksvndialog.h"

#include <KLocalizedString>

CopyMoveView_impl::CopyMoveView_impl(const QString &baseName, const QString &sourceName, bool move, QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    m_BaseName = baseName;
    if (!m_BaseName.isEmpty() && !m_BaseName.endsWith(QLatin1Char('/'))) {
        m_BaseName += QLatin1Char('/');
    }
    m_PrefixLabel->setText(m_BaseName);
    m_OldNameLabel->setText("<b>" + sourceName + "</b>");
    m_OldName = sourceName;
    if (m_BaseName.length() > 0) {
        QString t = m_OldName.right(m_OldName.length() - m_BaseName.length());
        m_NewNameInput->setText(t);
    } else {
        m_PrefixLabel->hide();
        m_NewNameInput->setText(sourceName);
    }
    if (move) {
        m_HeadOneLabel->setText(i18n("Rename/move"));
    } else {
        m_HeadOneLabel->setText(i18n("Copy"));
    }
}

CopyMoveView_impl::~CopyMoveView_impl()
{
}


/*!
    \fn CopyMoveView_impl::newName()
 */
QString CopyMoveView_impl::newName() const
{
    return m_BaseName + m_NewNameInput->text();
}


/*!
    \fn CopyMoveView_impl::getMoveCopyTo(bool*ok,const QString&old,const QString&base,QWidget*)
 */
QString CopyMoveView_impl::getMoveCopyTo(bool *ok, bool move,
                                         const QString &old, const QString &base, QWidget *parent)
{
    QPointer<KSvnSimpleOkDialog> dlg(new KSvnSimpleOkDialog(QStringLiteral("copy_move_dlg"), parent));
    dlg->setWindowTitle(move ? i18nc("@title:window", "Move/Rename File/Directory") : i18nc("@title:window", "Copy File/Directory"));
    dlg->setWithCancelButton();

    CopyMoveView_impl *ptr = new CopyMoveView_impl(base, old, (move), dlg);
    dlg->addWidget(ptr);
    QString nName;
    if (dlg->exec() != QDialog::Accepted) {
        if (ok) {
            *ok = false;
        }
    } else {
        nName = ptr->newName();
        if (ok) {
            *ok = true;
        }
    }
    delete dlg;
    return nName;
}
