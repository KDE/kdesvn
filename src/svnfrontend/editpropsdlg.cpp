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
#include "editpropsdlg.h"
#include "ui_editpropsdlg.h"

#include <QWhatsThis>

EditPropsDlg::EditPropsDlg(bool bAddMode, QWidget *parent)
    : KSvnDialog(QLatin1String("modify_properties"), parent)
    , m_isDir(false)
    , m_ui(new Ui::EditPropsDlg)
{
    m_ui->setupUi(this);
    if (bAddMode) {
        setWindowTitle(i18nc("@title:window", "Add Property"));
    }
    connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    /// @TODO Read these values from a text or config file
    fileProperties += QStringLiteral("svn:eol-style");
    fileProperties += QStringLiteral("svn:executable");
    fileProperties += QStringLiteral("svn:keywords");
    fileProperties += QStringLiteral("svn:needs-lock");
    fileProperties += QStringLiteral("svn:mime-type");

    fileComments += i18n("One of <b>'native'</b>, <b>'LF'</b>, <b>'CR'</b>, <b>'CRLF'</b>.");
    fileComments += i18n("If present, make the file executable.<br/>"
                         "This property can not be set on a directory. "
                         "A non-recursive attempt will fail, and a recursive attempt "
                         "will set the property only on the file children of the folder.");
    fileComments += i18n("Keywords to be expanded into the contents of a file.<br/>"
                         "They can be inserted into documents by placing a keyword anchor "
                         "which is formatted as $KeywordName$.<br/>"
                         "Valid keywords are:<br/>"
                         "<b>URL/HeadURL</b> The URL for the head revision of the project.<br/>"
                         "<b>Author/LastChangedBy</b> The last person to change the file.<br/>"
                         "<b>Date/LastChangedDate</b> The date/time the object was last modified.<br/>"
                         "<b>Revision/Rev/LastChangedRevision</b> The last revision the object changed.<br/>"
                         "<b>Id</b> A compressed summary of the previous 4 keywords.");
    fileComments += i18n("Set this to any value (e.g. <b>'*'</b>) to enforce locking for this file.<br/>"
                         "The file will be set read-only when checked out or updated, "
                         "indicating that a user must acquire a lock on the file before "
                         "they can edit and commit changes.");
    fileComments += i18n("The mimetype of the file. Used to determine "
                         "whether to merge the file and how to serve it from "
                         "Apache. A mimetype beginning with <b>'text/'</b> (or an absent "
                         "mimetype) is treated as text. Anything else is treated as binary.");

    dirProperties += QStringLiteral("svn:eol-style");
    dirProperties += QStringLiteral("svn:executable");
    dirProperties += QStringLiteral("svn:externals");
    dirProperties += QStringLiteral("svn:ignore");
    dirProperties += QStringLiteral("svn:mime-type");
    dirProperties += QStringLiteral("bugtraq:label");
    dirProperties += QStringLiteral("bugtraq:url");
    dirProperties += QStringLiteral("bugtraq:message");
    dirProperties += QStringLiteral("bugtraq:warnifnoissue");
    dirProperties += QStringLiteral("bugtraq:number");
    dirProperties += QStringLiteral("bugtraq:append");
    dirProperties += QStringLiteral("bugtraq:logregex");

    dirComments += i18n("One of <b>'native'</b>, <b>'LF'</b>, <b>'CR'</b>, <b>'CRLF'</b>.");
    dirComments += i18n("If present, make the file executable.<br/>"
                        "This property can not be set on a directory. "
                        "A non-recursive attempt will fail, and a recursive attempt "
                        "will set the property only on the file children of the folder.");
    /* TRANSLATORS: Do not translate "example" in the URL because this is according
       TRANSLATORS: to https://www.rfc-editor.org/rfc/rfc2606.txt a reserved URL.*/
    dirComments += i18n("A newline separated list of module specifiers, each "
                        "consisting of a relative directory path, optional revision "
                        "flags, and a URL. For example:<br/>"
                        "<nobr><b>foo http://example.com/repos/projectA</b></nobr><br/>"
                        "<nobr><b>foo/bar -r 1234 http://example.com/repos/projectB</b></nobr>");
    dirComments += i18n("A newline separated list of file patterns to ignore.");
    dirComments += i18n("The mimetype of the file. Used to determine "
                        "whether to merge the file and how to serve it from "
                        "Apache. A mimetype beginning with <b>'text/'</b> (or an absent "
                        "mimetype) is treated as text. Anything else is treated as binary.");
    dirComments += i18n("Label text to show for the edit box where the user enters the issue number.");
    /* TRANSLATORS: Do not translate "example" in the URL because this is according
       TRANSLATORS: to https://www.rfc-editor.org/rfc/rfc2606.txt a reserved URL.*/
    dirComments += i18n("URL pointing to the issue tracker. It must contain "
                        "<b>%BUGID%</b> which gets replaced with the bug issue number. Example:<br/>"
                        "<nobr><b>http://example.com/mantis/view.php?id=%BUGID%</b></nobr>");
    dirComments += i18n("String which is appended to a log message when an issue "
                        "number is entered. The string must contain <b>%BUGID%</b> "
                        "which gets replaced with the bug issue number.");
    dirComments += i18n("Set to <b>'yes'</b> if a warning shall be shown when "
                        "no issue is entered in the commit dialog. Possible values:<br/>"
                        "<b>'true'</b>/<b>'yes'</b> or <b>'false'</b>/<b>'no'</b>.");
    dirComments += i18n("Set to <b>'false'</b> if your bugtracking system has "
                        "issues which are referenced not by numbers.<br/>"
                        "Possible values: <b>'true'</b> or <b>'false'</b>.");
    dirComments += i18n("Set to <b>'false'</b> if you want the bugtracking ID "
                        "to be inserted at the top of the log message. The "
                        "default is <b>'true'</b> which means the bugtracking "
                        "ID is appended to the log message.");
    dirComments += i18n("Two regular expressions separated by a newline.<br/>"
                        "The first expression is used to find a string referring to an issue, the "
                        "second expression is used to extract the bare bug ID from that string.");

    m_ui->m_NameEdit->setCompletionMode(KCompletion::CompletionPopupAuto);
    m_ui->m_NameEdit->setHistoryItems(fileProperties, true);

    m_ui->m_NameEdit->setToolTip(i18n("Select or enter new property"));
    connect(m_ui->m_NameEdit, QOverload<const QString &>::of(&KHistoryComboBox::activated),
            this, &EditPropsDlg::updateToolTip);
}

EditPropsDlg::~EditPropsDlg()
{
    delete m_ui;
}

void EditPropsDlg::updateToolTip(const QString &selection)
{
    QString comment;
    if (m_isDir) {
        int i = dirProperties.indexOf(selection);
        if (i >= 0) {
            comment = dirComments.at(i);
        }
    } else {
        int i = fileProperties.indexOf(selection);
        if (i >= 0) {
            comment = fileComments.at(i);
        }
    }
    if (comment.isEmpty()) {
        comment = i18n("No help for this property available");
    }
    m_ui->m_NameEdit->setToolTip(comment);
}

void EditPropsDlg::setDir(bool dir)
{
    if (dir == m_isDir) {
        // Change not necessary
        return;
    }
    m_ui->m_NameEdit->setHistoryItems(dir ? dirProperties : fileProperties, true);

    m_isDir = dir;
}

QString EditPropsDlg::propName()const
{
    return m_ui->m_NameEdit->currentText();
}

QString EditPropsDlg::propValue()const
{
    return m_ui->m_ValueEdit->toPlainText();
}

void EditPropsDlg::setPropName(const QString &n)
{
    m_ui->m_NameEdit->addToHistory(n);
    m_ui->m_NameEdit->setCurrentItem(n);
    updateToolTip(n);
}

void EditPropsDlg::setPropValue(const QString &v)
{
    m_ui->m_ValueEdit->setText(v);
}
