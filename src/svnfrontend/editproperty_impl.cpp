/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht                                  *
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
#include <qstringlist.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#include <ktextedit.h>
#include <kcombobox.h>
#include <kglobalsettings.h>
#include <kdebug.h>

#include "editproperty_impl.h"

KHistoryCombo *m_NameEdit;
KTextEdit *m_TextEdit;

EditProperty_impl::EditProperty_impl(QWidget *parent, const char *name)
    :EditPropsDlgData(parent, name)
{
    // TODO Read these values from a text or config file
    fileProperties += "svn:eol-style";
    fileProperties += "svn:executable";
    fileProperties += "svn:keywords";
    fileProperties += "svn:needs-lock";
    fileProperties += "svn:mime-type";

    fileComments += "One of <b>'native'</b>, <b>'LF'</b>, <b>'CR'</b>, <b>'CRLF'</b></b>.";
    fileComments += "If present, make the file executable.<br>\
        This property can not be set on a directory.<br>\
        A non-recursive attempt will fail, and a recursive attempt<br>\
        will set the property only on the file children of the folder.";
    fileComments += "Keywords to be expanded into the contents of a file.<br>\
        They can be inserted into documents by placing a keyword anchor<br>\
        which is formatted as $KeywordName$.<br>\
        Valid keywords are:<br>\
        <b>URL, HeadURL</b> The URL for the head revision of the project.<br>\
        <b>Author</b> LastChangedBy The last person to change the file.<br>\
        <b>Date, LastChangedDate</b> The date/time the object was last modified.<br>\
        <b>Rev, LastChangedRevision</b> The last revision the object changed.<br>\
        <b>Id</b> A compressed summary of the previous 4 keywords.";
    fileComments += "Set this to any value (e.g. <b>'*'</b>) to enforce locking for this file.<br>\
        The file will be set read-only when checked out or updated,<br>\
        indicating that a user must acquire a lock on the file before<br>\
        they can edit and commit changes.";
    fileComments += "The mimetype of the file. Used to determine<br>\
        whether to merge the file and how to serve it from<br>\
        Apache. A mimetype beginning with <b>'text/'</b> (or an absent<br>\
        mimetype) is treated as text. Anything else is treated as binary.";

    dirProperties += "svn:eol-style";
    dirProperties += "svn:executable";
    dirProperties += "svn:external";
    dirProperties += "svn:ignore";
    dirProperties += "svn:needs-lock";
    dirProperties += "svn:mime-type";
    dirProperties += "bugtraq:label";
    dirProperties += "bugtraq:url";
    dirProperties += "bugtraq:message";
    dirProperties += "bugtraq:warnifnoissue";
    dirProperties += "bugtraq:number";
    dirProperties += "bugtraq:append";
    dirProperties += "bugtraq:logregex";

    dirComments += "One of <b>'native'</b>, <b>'LF'</b>, <b>'CR'</b>, <b>'CRLF'</b></b>.";
    dirComments += "If present, make the file executable.<br>\
        This property can not be set on a directory.<br>\
        A non-recursive attempt will fail, and a recursive attempt<br>\
        will set the property only on the file children of the folder.";
    dirComments += "A newline separated list of module specifiers, each of<br>\
        which consists of a relative directory path, optional revision<br>\
        flags, and an URL. For example:<br>\
        foo	http://example.com/repos/projectA<br>\
        foo/bar -r 1234 http://example.com/repos/projectB<br>";
    dirComments += "A newline separated list of file patterns to ignore.";
    dirComments += "Set this to any value (e.g. <b>'*'</b>) to enforce locking for this file.<br>\
        The file will be set read-only when checked out or updated,<br>\
        indicating that a user must acquire a lock on the file before<br>\
        they can edit and commit changes.";
    dirComments += "The mimetype of the file. Used to determine<br>\
        whether to merge the file and how to serve it from<br>\
        Apache. A mimetype beginning with <b>'text/'</b> (or an absent<br>\
        mimetype) is treated as text. Anything else is treated as binary.";
    dirComments += "Label text to show for the edit box where the user enters the issue number.";
    dirComments += "URL pointing to the issue tracker. It must contain<br>\
        <b>%BUGID%</b> which gets replaced with the bug issue number.<br>\
        Example: <b>http://<mantisserver>/mantis/view.php?id=%BUGID%</b>";
    dirComments += "String which is appended to a log message when an issue<br>\
        number is entered. The string must contain <b>%BUGID%</b> <br>\
        which gets replaced with the bug issue number.";
    dirComments += "Set to <b>'yes'</b> if a warning shall be shown when<br>\
        no issue is entered in the commit dialog. Possible values:<br>\
        <b>'true'</b>/<b>'yes'</b> or <b>'false'</b>/<b>'no'</b>";
    dirComments += "Set to <b>'false'</b> if your bugtracking system has<br>\
        issues which are referenced not by numbers.<br>\
        Possible values: <b>'true'</b> or <b>'false'</b>";
    dirComments += "Set to <b>'false'</b> if you want the bugtracking ID<br>\
        to be inserted at the top of the log message. The<br>\
        default is <b>'true'</b> which means the bugtracking<br>\
        ID is appended to the log message.";
    dirComments += "Two regular expressions separated by a newline.<br>\
        The first expression is used to find a string referring to an issue, the<br>\
        second expression is used to extract the bare bug ID from that string.";

    m_NameEdit->setAutoCompletion(true);
    m_NameEdit->setCompletionMode(KGlobalSettings::CompletionPopupAuto);
    m_NameEdit->setDuplicatesEnabled(false);
    m_NameEdit->setHistoryItems(fileProperties, true);
    isDir = false;

    QToolTip::add(m_NameEdit, "Select or enter new property");
    connect(m_NameEdit, SIGNAL(activated(const QString &)), this, SLOT(updateToolTip(const QString &)));
}


EditProperty_impl::~EditProperty_impl() {
    kdDebug() << "EditProperty_impl got destroyed" << endl;
}


void EditProperty_impl::slotHelp()
{
    qWarning( "PropertiesDlg::slotHelp(): Not implemented yet" );
}


void EditProperty_impl::updateToolTip(const QString & selection)
{
    int i;
    QString comment;

    if (isDir) {
        i = dirProperties.findIndex(selection);
        if (i >= 0) {
            comment = dirComments[i];
        } else {
            comment = "No help for this property available";
        }
    } else {
        i = fileProperties.findIndex(selection);
        if (i >= 0) {
            comment = fileComments[i];
        } else {
            comment = "No help for this property available";
        }
    }

    QToolTip::add(m_NameEdit, comment);

    // TODO Display a WhatsThis help
    /*
    QPoint pos = m_TextEdit->pos();
    pos.setX(pos.x() + m_TextEdit->width()/2);
    pos.setY(pos.y() + m_TextEdit->height()/2);
    QWhatsThis::display(comment, mapToGlobal(pos))
    */
}

void EditProperty_impl::setDir(bool dir)
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


QString EditProperty_impl::PropName()const
{
    return m_NameEdit->currentText();
}


QString EditProperty_impl::PropValue()const
{
    return m_ValueEdit->text();
}

void EditProperty_impl::setPropName(const QString&n)
{
    m_NameEdit->addToHistory(n);
    m_NameEdit->setCurrentItem(n);
}

void EditProperty_impl::setPropValue(const QString&v)
{
    m_ValueEdit->setText(v);
}

#include "editproperty_impl.moc"
