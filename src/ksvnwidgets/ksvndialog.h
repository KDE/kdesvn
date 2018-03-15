/***************************************************************************
 *   Copyright (C) 2016 Christian Ehrlicher <ch.ehrlicher@gmx.de>          *
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
#pragma once

#include <QDialog>

class QDialogButtonBox;
class QVBoxLayout;

class KSvnDialog : public QDialog
{
    Q_OBJECT
public:
    explicit KSvnDialog(const QString &configGroupName, QWidget *parent = nullptr);
    ~KSvnDialog();

protected:
    void setDefaultButton(QPushButton *defaultButton);
    void showEvent(QShowEvent *e) override;
private:
    QString m_configGroupName;
};

class KSvnSimpleOkDialog : public KSvnDialog
{
    Q_OBJECT
public:
    explicit KSvnSimpleOkDialog(const QString &configGroupName, QWidget *parent = nullptr);

    /**
     * @brief Add a cancel button to the button box
     */
    void setWithCancelButton();
    /**
     * @brief Add a new widget to the VBoxLayout
     */
    void addWidget(QWidget *widget);
    /**
     * @brief Add the button box to the vbox layout.
     * only needed if exec() is not called
     * --> if it is not treated as modal dialog
     */
    void addButtonBox();
    /**
     * @brief Set the appropriate help context
     */
    void setHelp(const QString &context);
    int exec() override;

    QDialogButtonBox *buttonBox() { return m_bBox; }

private Q_SLOTS:
    void onHelpRequested();
private:
    QVBoxLayout *m_layout;
    QDialogButtonBox *m_bBox;
    bool m_bBoxAdded;
    QString m_helpContext;
};
