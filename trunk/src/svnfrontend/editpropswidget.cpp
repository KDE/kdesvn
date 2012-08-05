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
#include "editpropswidget.h"

#include <QStringList>
#include <QToolTip>
#include <QWhatsThis>

#include <ktextedit.h>
#include <kcombobox.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <klocale.h>
#include <kicon.h>

EditPropsWidget::EditPropsWidget(QWidget *parent, const char *name)
    : QWidget(parent)
{
    setupUi(this);
    setObjectName(name);

    helpButton->setIcon(KIcon("help-hint"));

    /// @TODO Read these values from a text or config file
    fileProperties += ("svn:eol-style");
    fileProperties += ("svn:executable");
    fileProperties += ("svn:keywords");
    fileProperties += ("svn:needs-lock");
    fileProperties += ("svn:mime-type");

    fileComments += i18n("One of <b>'native'</b>, <b>'LF'</b>, <b>'CR'</b>, <b>'CRLF'</b>.");
    fileComments += i18n("If present, make the file executable.<br>"
        "This property can not be set on a directory. "
        "A non-recursive attempt will fail, and a recursive attempt "
        "will set the property only on the file children of the folder.");
    fileComments += i18n("Keywords to be expanded into the contents of a file.<br>"
        "They can be inserted into documents by placing a keyword anchor "
        "which is formatted as $KeywordName$.<br>"
        "Valid keywords are:<br>"
        "<b>URL/HeadURL</b> The URL for the head revision of the project.<br>"
        "<b>Author/LastChangedBy</b> The last person to change the file.<br>"
        "<b>Date/LastChangedDate</b> The date/time the object was last modified.<br>"
        "<b>Revision/Rev/LastChangedRevision</b> The last revision the object changed.<br>"
        "<b>Id</b> A compressed summary of the previous 4 keywords.");
    fileComments += i18n("Set this to any value (e.g. <b>'*'</b>) to enforce locking for this file.<br>"
        "The file will be set read-only when checked out or updated, "
        "indicating that a user must acquire a lock on the file before "
        "they can edit and commit changes.");
    fileComments += i18n("The mimetype of the file. Used to determine "
        "whether to merge the file and how to serve it from "
        "Apache. A mimetype beginning with <b>'text/'</b> (or an absent "
        "mimetype) is treated as text. Anything else is treated as binary.");

    dirProperties += ("svn:eol-style");
    dirProperties += ("svn:executable");
    dirProperties += ("svn:externals");
    dirProperties += ("svn:ignore");
    dirProperties += ("svn:mime-type");
    dirProperties += ("bugtraq:label");
    dirProperties += ("bugtraq:url");
    dirProperties += ("bugtraq:message");
    dirProperties += ("bugtraq:warnifnoissue");
    dirProperties += ("bugtraq:number");
    dirProperties += ("bugtraq:append");
    dirProperties += ("bugtraq:logregex");

    dirComments += i18n("One of <b>'native'</b>, <b>'LF'</b>, <b>'CR'</b>, <b>'CRLF'</b>.");
    dirComments += i18n("If present, make the file executable.<br>"
        "This property can not be set on a directory. "
        "A non-recursive attempt will fail, and a recursive attempt "
        "will set the property only on the file children of the folder.");
    /* TRANSLATORS: Do not translate "example" in the URL because this is according
       TRANSLATORS: to http://www.rfc-editor.org/rfc/rfc2606.txt a reserved URL.*/
    dirComments += i18n("A newline separated list of module specifiers, each "
        "consisting of a relative directory path, optional revision "
        "flags, and a URL. For example:<br>"
        "<nobr><b>foo http://example.com/repos/projectA</b></nobr><br>"
        "<nobr><b>foo/bar -r 1234 http://example.com/repos/projectB</b></nobr>");
    dirComments += i18n("A newline separated list of file patterns to ignore.");
    dirComments += i18n("The mimetype of the file. Used to determine "
        "whether to merge the file and how to serve it from "
        "Apache. A mimetype beginning with <b>'text/'</b> (or an absent "
        "mimetype) is treated as text. Anything else is treated as binary.");
    dirComments += i18n("Label text to show for the edit box where the user enters the issue number.");
    /* TRANSLATORS: Do not translate "example" in the URL because this is according
       TRANSLATORS: to http://www.rfc-editor.org/rfc/rfc2606.txt a reserved URL.*/
    dirComments += i18n("URL pointing to the issue tracker. It must contain "
        "<b>%BUGID%</b> which gets replaced with the bug issue number. Example:<br>"
        "<nobr><b>http://example.com/mantis/view.php?id=%BUGID%</b></nobr>");
    dirComments += i18n("String which is appended to a log message when an issue "
        "number is entered. The string must contain <b>%BUGID%</b> "
        "which gets replaced with the bug issue number.");
    dirComments += i18n("Set to <b>'yes'</b> if a warning shall be shown when "
        "no issue is entered in the commit dialog. Possible values:<br>"
        "<b>'true'</b>/<b>'yes'</b> or <b>'false'</b>/<b>'no'</b>.");
    dirComments += i18n("Set to <b>'false'</b> if your bugtracking system has "
        "issues which are referenced not by numbers.<br>"
        "Possible values: <b>'true'</b> or <b>'false'</b>.");
    dirComments += i18n("Set to <b>'false'</b> if you want the bugtracking ID "
        "to be inserted at the top of the log message. The "
        "default is <b>'true'</b> which means the bugtracking "
        "ID is appended to the log message.");
    dirComments += i18n("Two regular expressions separated by a newline.<br>"
        "The first expression is used to find a string referring to an issue, the "
        "second expression is used to extract the bare bug ID from that string.");

    m_NameEdit->setAutoCompletion(true);
    m_NameEdit->setCompletionMode(KGlobalSettings::CompletionPopupAuto);
    m_NameEdit->setDuplicatesEnabled(false);
    m_NameEdit->setHistoryItems(fileProperties, true);
    isDir = false;

    m_NameEdit->setToolTip("Select or enter new property");
    connect(m_NameEdit, SIGNAL(activated(const QString &)), this, SLOT(updateToolTip(const QString &)));
}


EditPropsWidget::~EditPropsWidget()
{
}


void EditPropsWidget::updateToolTip(const QString & selection)
{
    int i;

    if (isDir) {
        i = dirProperties.indexOf(selection);
        if (i >= 0) {
            comment = dirComments[i];
        } else {
            comment = "No help for this property available";
        }
    } else {
        i = fileProperties.indexOf(selection);
        if (i >= 0) {
            comment = fileComments[i];
        } else {
            comment = "No help for this property available";
        }
    }
    m_NameEdit->setToolTip(comment);
}

void EditPropsWidget::setDir(bool dir)
{
    if (dir == isDir) {
        // Change not necessary
        return;
    }
    if (dir) {
        m_NameEdit->clearHistory();
        m_NameEdit->setHistoryItems(dirProperties, true);
    } else {
        m_NameEdit->clearHistory();
        m_NameEdit->setHistoryItems(fileProperties, true);
    }

    isDir = dir;
}


QString EditPropsWidget::propName()const
{
    return m_NameEdit->currentText();
}


QString EditPropsWidget::propValue()const
{
    return m_ValueEdit->toPlainText();
}

void EditPropsWidget::setPropName(const QString&n)
{
    m_NameEdit->addToHistory(n);
    m_NameEdit->setCurrentItem(n);
    updateToolTip(n);
}


void EditPropsWidget::setPropValue(const QString&v)
{
    m_ValueEdit->setText(v);
}

void EditPropsWidget::showHelp()
{
    QPoint pos = m_ValueEdit->pos();
    pos.setX(pos.x() + m_ValueEdit->width()/2);
    pos.setY(pos.y() + m_ValueEdit->height()/4);
    QWhatsThis::showText(mapToGlobal(pos),comment);
}

#include "editpropswidget.moc"
