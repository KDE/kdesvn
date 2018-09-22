/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
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
#include "createrepodlg.h"
#include "ui_createrepodlg.h"

#include "svnqt/version_check.h"
#include "svnqt/repoparameter.h"

class RecurseCheck
{
    bool &value;
public:
    explicit RecurseCheck(bool &aValue)
      : value(aValue)
    {
        value = true;
    }
    ~RecurseCheck()
    {
        value = false;
    }
};


CreaterepoDlg::CreaterepoDlg(QWidget *parent)
    : KSvnDialog(QLatin1String("create_repo"), parent)
    , m_inChangeCompat(false)
    , m_ui(new Ui::CreateRepoDlg)
{
    m_ui->setupUi(this);
    setDefaultButton(m_ui->buttonBox->button(QDialogButtonBox::Ok));

    const bool bGE15 = (svn::Version::version_major() > 1 || svn::Version::version_minor() >= 5);
    m_ui->m_presvn15compat->setEnabled(bGE15);
    m_ui->m_presvn15compat->setVisible(bGE15);

    const bool bGE16 = (svn::Version::version_major() > 1 || svn::Version::version_minor() >= 6);
    m_ui->m_presvn16compat->setEnabled(bGE16);
    m_ui->m_presvn16compat->setVisible(bGE16);

    const bool bGE18 = (svn::Version::version_major() > 1 || svn::Version::version_minor() >= 8);
    m_ui->m_presvn18compat->setEnabled(bGE18);
    m_ui->m_presvn18compat->setVisible(bGE18);

    connect(m_ui->m_presvn15compat, &QAbstractButton::clicked, this, &CreaterepoDlg::compatChanged15);
    connect(m_ui->m_presvn16compat, &QAbstractButton::clicked, this, &CreaterepoDlg::compatChanged16);
    connect(m_ui->m_presvn18compat, &QAbstractButton::clicked, this, &CreaterepoDlg::compatChanged18);
    connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

CreaterepoDlg::~CreaterepoDlg()
{
    delete m_ui;
}

void CreaterepoDlg::fsTypeChanged(int which)
{
    m_ui->m_DisableFsync->setEnabled(which == 1);
    m_ui->m_LogKeep->setEnabled(which == 1);
}

QString CreaterepoDlg::targetDir()const
{
    // Local only
    return m_ui->m_ReposPathinput->url().toLocalFile();
}

bool CreaterepoDlg::createMain() const
{
    return m_ui->m_CreateMainDirs->isChecked();
}

void CreaterepoDlg::compatChanged15()
{
    if (m_inChangeCompat) {
        return;
    }
    RecurseCheck rc(m_inChangeCompat);
    if (m_ui->m_presvn15compat->isChecked()) {
        m_ui->m_presvn16compat->setChecked(false);
        m_ui->m_presvn18compat->setChecked(false);
    }
}

void CreaterepoDlg::compatChanged16()
{
    if (m_inChangeCompat) {
        return;
    }
    RecurseCheck rc(m_inChangeCompat);
    if (m_ui->m_presvn16compat->isChecked()) {
        m_ui->m_presvn15compat->setChecked(false);
        m_ui->m_presvn18compat->setChecked(false);
    }
}

void CreaterepoDlg::compatChanged18()
{
    if (m_inChangeCompat) {
        return;
    }
    RecurseCheck rc(m_inChangeCompat);
    if (m_ui->m_presvn18compat->isChecked()) {
        m_ui->m_presvn15compat->setChecked(false);
        m_ui->m_presvn16compat->setChecked(false);
    }
}

svn::repository::CreateRepoParameter CreaterepoDlg::parameter() const
{
    svn::repository::CreateRepoParameter params;
    params.path(targetDir());
    params.pre15_compat(m_ui->m_presvn15compat->isChecked());
    params.pre16_compat(m_ui->m_presvn16compat->isChecked());
    params.pre18_compat(m_ui->m_presvn18compat->isChecked());
    params.fstype(m_ui->m_FilesystemSelector->currentText());
    params.bdbnosync(m_ui->m_DisableFsync->isChecked());
    params.bdbautologremove(!m_ui->m_LogKeep->isChecked());
    return params;
}

