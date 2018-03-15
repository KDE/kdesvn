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
#include "commandexec.h"
#include "settings/kdesvnsettings.h"
#include "svnfrontend/svnactions.h"
#include "svnfrontend/dummydisplay.h"
#include "svnqt/targets.h"
#include "svnqt/url.h"
#include "svnqt/dirent.h"
#include "helpers/ktranslateurl.h"
#include "helpers/sshagent.h"
#include "helpers/windowgeometryhelper.h"
#include "svnfrontend/fronthelpers/rangeinput_impl.h"
#include "svnfrontend/copymoveview_impl.h"
#include "ksvnwidgets/ksvndialog.h"

#include <KMessageBox>

#include <QCommandLineParser>
#include <QDir>
#include <QTextBrowser>
#include <QTextStream>
#include <QUrlQuery>

class pCPart
{
public:
    pCPart();
    ~pCPart();

    QString cmd;
    QStringList urls;
    bool ask_revision;
    bool rev_set;
    bool outfile_set;
    bool single_revision;
    bool force;
    int log_limit;
    SvnActions *m_SvnWrapper;
    QCommandLineParser *parser;
    QStringList args;
    svn::Revision start, end;

    // for output
    QString outfile;
    QTextStream Stdout, Stderr;
    DummyDisplay *disp;
    QMap<int, svn::Revision> extraRevisions;
    QMap<int, QUrl> repoUrls;
};

pCPart::pCPart()
    : cmd()
    , urls()
    , ask_revision(false)
    , rev_set(false)
    , outfile_set(false)
    , single_revision(false)
    , force(false)
    , log_limit(0)
    , m_SvnWrapper(nullptr)
    , parser(nullptr)
    , start(svn::Revision::UNDEFINED)
    , end(svn::Revision::UNDEFINED)
    , Stdout(stdout)
    , Stderr(stderr)
    , disp(new DummyDisplay())
{
    m_SvnWrapper = new SvnActions(disp, true);
}

pCPart::~pCPart()
{
    delete m_SvnWrapper;
    delete disp;
}

CommandExec::CommandExec(QObject *parent)
    : QObject(parent)
    , m_lastMessagesLines(0)
{
    m_pCPart = new pCPart;
    m_pCPart->parser = nullptr;
    SshAgent ag;
    ag.querySshAgent();

    connect(m_pCPart->m_SvnWrapper, SIGNAL(clientException(QString)), this, SLOT(clientException(QString)));
    connect(m_pCPart->m_SvnWrapper, SIGNAL(sendNotify(QString)), this, SLOT(slotNotifyMessage(QString)));
    m_pCPart->m_SvnWrapper->reInitClient();
}


CommandExec::~CommandExec()
{
    delete m_pCPart;
}

int CommandExec::exec(const QCommandLineParser *parser)
{
    m_pCPart->parser = const_cast<QCommandLineParser*>(parser);
    m_pCPart->args = parser->positionalArguments();
    if (m_pCPart->args.isEmpty()) {
        return -1;
    }
    m_lastMessages.clear();
    m_lastMessagesLines = 0;
    m_pCPart->m_SvnWrapper->reInitClient();
    bool dont_check_second = false;
    bool dont_check_all = false;
    bool path_only = false;
    bool no_revision = false;
    bool check_force = false;

    if (m_pCPart->args.count() >= 2) {
        m_pCPart->cmd = m_pCPart->args.at(1);
        m_pCPart->cmd = m_pCPart->cmd.toLower();
    }
    QByteArray slotCmd;
    if (!QString::compare(m_pCPart->cmd, QLatin1String("log"))) {
        slotCmd = SLOT(slotCmd_log());
    } else if (!QString::compare(m_pCPart->cmd, QLatin1String("cat"))) {
        slotCmd = SLOT(slotCmd_cat());
        m_pCPart->single_revision = true;
    } else if (!QString::compare(m_pCPart->cmd, QLatin1String("get"))) {
        slotCmd = SLOT(slotCmd_get());
        m_pCPart->single_revision = true;
    } else if (!QString::compare(m_pCPart->cmd, QLatin1String("help"))) {
        slotCmd = SLOT(slotCmd_help());
    } else if (!QString::compare(m_pCPart->cmd, QLatin1String("blame")) ||
               !QString::compare(m_pCPart->cmd, QLatin1String("annotate"))) {
        slotCmd = SLOT(slotCmd_blame());
    } else if (!QString::compare(m_pCPart->cmd, QLatin1String("update"))) {
        slotCmd = SLOT(slotCmd_update());
        m_pCPart->single_revision = true;
    } else if (!QString::compare(m_pCPart->cmd, QLatin1String("diff"))) {
        m_pCPart->start = svn::Revision::WORKING;
        slotCmd = SLOT(slotCmd_diff());
    } else if (!QString::compare(m_pCPart->cmd, QLatin1String("info"))) {
        slotCmd = SLOT(slotCmd_info());
        m_pCPart->single_revision = true;
    } else if (!QString::compare(m_pCPart->cmd, QLatin1String("commit")) ||
               !QString::compare(m_pCPart->cmd, QLatin1String("ci"))) {
        slotCmd = SLOT(slotCmd_commit());
    } else if (!QString::compare(m_pCPart->cmd, QLatin1String("list")) ||
               !QString::compare(m_pCPart->cmd, QLatin1String("ls"))) {
        slotCmd = SLOT(slotCmd_list());
    } else if (!QString::compare(m_pCPart->cmd, QLatin1String("copy")) ||
               !QString::compare(m_pCPart->cmd, QLatin1String("cp"))) {
        slotCmd = SLOT(slotCmd_copy());
        dont_check_second = true;
    } else if (!QString::compare(m_pCPart->cmd, QLatin1String("move")) ||
               !QString::compare(m_pCPart->cmd, QLatin1String("rename")) ||
               !QString::compare(m_pCPart->cmd, QLatin1String("mv"))) {
        slotCmd = SLOT(slotCmd_move());
        dont_check_second = true;
    } else if (!QString::compare(m_pCPart->cmd, QLatin1String("checkout")) ||
               !QString::compare(m_pCPart->cmd, QLatin1String("co"))) {
        slotCmd = SLOT(slotCmd_checkout());
        dont_check_second = true;
    } else if (!QString::compare(m_pCPart->cmd, QLatin1String("checkoutto")) ||
               !QString::compare(m_pCPart->cmd, QLatin1String("coto"))) {
        slotCmd = SLOT(slotCmd_checkoutto());
        dont_check_second = true;
    } else if (!QString::compare(m_pCPart->cmd, QLatin1String("export"))) {
        slotCmd = SLOT(slotCmd_export());
        dont_check_second = true;
    } else if (!QString::compare(m_pCPart->cmd, QLatin1String("exportto"))) {
        slotCmd = SLOT(slotCmd_exportto());
        dont_check_second = true;
    } else if (!QString::compare(m_pCPart->cmd, QLatin1String("delete")) ||
               !QString::compare(m_pCPart->cmd, QLatin1String("del")) ||
               !QString::compare(m_pCPart->cmd, QLatin1String("rm")) ||
               !QString::compare(m_pCPart->cmd, QLatin1String("remove"))) {
        slotCmd = SLOT(slotCmd_delete());
    } else if (!QString::compare(m_pCPart->cmd, QLatin1String("add"))) {
        slotCmd = SLOT(slotCmd_add());
        dont_check_all = true;
        path_only = true;
    } else if (!QString::compare(m_pCPart->cmd, QLatin1String("undo")) ||
               !QString::compare(m_pCPart->cmd, QLatin1String("revert"))) {
        slotCmd = SLOT(slotCmd_revert());
    } else if (!QString::compare(m_pCPart->cmd, QLatin1String("checknew")) ||
               !QString::compare(m_pCPart->cmd, QLatin1String("addnew"))) {
        slotCmd = SLOT(slotCmd_addnew());
    } else if (!QString::compare(m_pCPart->cmd, QLatin1String("switch"))) {
        slotCmd = SLOT(slotCmd_switch());
    } else if (!QString::compare(m_pCPart->cmd, QLatin1String("tree"))) {
        slotCmd = SLOT(slotCmd_tree());
    } else if (!QString::compare(m_pCPart->cmd, QLatin1String("lock"))) {
        slotCmd = SLOT(slotCmd_lock());
        no_revision = true;
        check_force = true;
    } else if (!QString::compare(m_pCPart->cmd, QLatin1String("unlock"))) {
        slotCmd = SLOT(slotCmd_unlock());
        no_revision = true;
        check_force = true;
    }

    bool found = connect(this, SIGNAL(executeMe()), this, slotCmd.constData());
    if (!found) {
        KMessageBox::sorry(nullptr,
                           i18n("Command \"%1\" not implemented or known", m_pCPart->cmd),
                           i18n("SVN Error"));
        return -1;
    }

    QString tmp;
    QString mainProto;
    for (int j = 2; j < m_pCPart->args.count(); ++j) {
        QUrl tmpurl = QUrl::fromUserInput(m_pCPart->args.at(j),
                                          QDir::currentPath());
        tmpurl.setScheme(svn::Url::transformProtokoll(tmpurl.scheme()));
        if (tmpurl.scheme().contains(QLatin1String("ssh"))) {
            SshAgent ag;
            // this class itself checks if done before
            ag.addSshIdentities();
        }
        m_pCPart->extraRevisions[j - 2] = svn::Revision::HEAD;

        if (tmpurl.isLocalFile() && (j == 2 || !dont_check_second) && !dont_check_all) {
            QUrl repoUrl;
            if (m_pCPart->m_SvnWrapper->isLocalWorkingCopy(tmpurl.path(), repoUrl)) {
                tmp = tmpurl.path();
                m_pCPart->repoUrls[j - 2] = repoUrl;
                m_pCPart->extraRevisions[j - 2] = svn::Revision::WORKING;
                if (j == 2) {
                    mainProto.clear();
                }
            } else {
                tmp = tmpurl.url();
                if (j == 2) {
                    mainProto = QLatin1String("file://");
                }
            }
        } else if (path_only) {
            tmp = tmpurl.path();
        } else {
            tmp = tmpurl.url();
            if (j == 2) {
                mainProto = tmpurl.scheme();
            }
        }
        if ((j > 2 && dont_check_second) || dont_check_all) {
            if (mainProto.isEmpty()) {
                tmp = tmpurl.path();
            }
        }
        const QVector<QStringRef> l = tmp.splitRef(QLatin1Char('?'), QString::SkipEmptyParts);
        if (!l.isEmpty()) {
            tmp = l.first().toString();
        }
        while (tmp.endsWith(QLatin1Char('/'))) {
            tmp.chop(1);
        }
        m_pCPart->urls.append(tmp);
        if ((j > 2 && dont_check_second) || dont_check_all) {
            continue;
        }
        const QList<QPair<QString, QString> > q = QUrlQuery(tmpurl).queryItems();
        for(int i = 0; i < q.size(); ++i) {
            if (q.at(i).first == QLatin1String("rev")) {
                svn::Revision re = q.at(i).second;
                if (re) {
                    m_pCPart->extraRevisions[j - 2] = re;
                }
            }
        }
    }
    if (m_pCPart->urls.isEmpty()) {
        m_pCPart->urls.append(QLatin1String("."));
    }

    if (!no_revision) {
        if (m_pCPart->parser->isSet("R")) {
            m_pCPart->ask_revision = true;
            if (!askRevision()) {
                return 0;
            }
        } else if (m_pCPart->parser->isSet("r")) {
            scanRevision();
        }
    }

    m_pCPart->force = check_force && m_pCPart->parser->isSet("f");

    if (m_pCPart->parser->isSet("o")) {
        m_pCPart->outfile_set = true;
        m_pCPart->outfile = m_pCPart->parser->value("o");
    }
    if (m_pCPart->parser->isSet("l")) {
        QString s = m_pCPart->parser->value("l");
        m_pCPart->log_limit = s.toInt();
        if (m_pCPart->log_limit < 0) {
            m_pCPart->log_limit = 0;
        }
    }

    emit executeMe();
    if (Kdesvnsettings::self()->cmdline_show_logwindow() &&
            m_lastMessagesLines >= Kdesvnsettings::self()->cmdline_log_minline()) {
        QPointer<KSvnSimpleOkDialog> dlg(new KSvnSimpleOkDialog(QStringLiteral("kdesvn_cmd_log"), QApplication::activeModalWidget()));
        QTextBrowser *ptr = new QTextBrowser(dlg);
        ptr->setText(m_lastMessages);
        ptr->setReadOnly(true);
        dlg->addWidget(ptr);
        QString cmd = qApp->arguments().join(QLatin1Char(' '));
        dlg->setWindowTitle(cmd);
        dlg->exec();
        delete dlg;
    }
    return 0;
}



/*!
    \fn CommandExec::clientException(const QString&)
 */
void CommandExec::clientException(const QString &what)
{
    m_pCPart->Stderr << what << endl;
    KMessageBox::sorry(nullptr, what, i18n("SVN Error"));
}


void CommandExec::slotCmd_log()
{
    int limit = m_pCPart->log_limit;
    if (m_pCPart->end == svn::Revision::UNDEFINED) {
        m_pCPart->end = svn::Revision::HEAD;
    }
    if (m_pCPart->start == svn::Revision::UNDEFINED) {
        m_pCPart->start = 1;
    }
    bool list = Kdesvnsettings::self()->log_always_list_changed_files();
    if (m_pCPart->extraRevisions[0] == svn::Revision::WORKING) {
        m_pCPart->extraRevisions[0] = svn::Revision::UNDEFINED;
    }
    m_pCPart->m_SvnWrapper->makeLog(m_pCPart->start, m_pCPart->end,
                                    m_pCPart->extraRevisions.value(0),
                                    m_pCPart->urls.at(0),
                                    Kdesvnsettings::log_follows_nodes(),
                                    list,
                                    limit);
}

void CommandExec::slotCmd_tree()
{
    if (m_pCPart->end == svn::Revision::UNDEFINED) {
        m_pCPart->end = svn::Revision::HEAD;
    }
    if (m_pCPart->start == svn::Revision::UNDEFINED) {
        m_pCPart->start = 1;
    }
    m_pCPart->m_SvnWrapper->makeTree(m_pCPart->urls.at(0), m_pCPart->extraRevisions.value(0),
                                     m_pCPart->start, m_pCPart->end);
}

void CommandExec::slotCmd_checkout()
{
    m_pCPart->m_SvnWrapper->CheckoutExport(QUrl::fromUserInput(m_pCPart->urls.at(0),
                                                               QDir::currentPath()), false);
}

void CommandExec::slotCmd_checkoutto()
{
    m_pCPart->m_SvnWrapper->CheckoutExport(QUrl::fromUserInput(m_pCPart->urls.at(0),
                                                               QDir::currentPath()), false, true);
}

void CommandExec::slotCmd_export()
{
    m_pCPart->m_SvnWrapper->CheckoutExport(QUrl::fromUserInput(m_pCPart->urls.at(0),
                                                               QDir::currentPath()), true);
}

void CommandExec::slotCmd_exportto()
{
    m_pCPart->m_SvnWrapper->CheckoutExport(QUrl::fromUserInput(m_pCPart->urls.at(0),
                                                               QDir::currentPath()), true, true);
}

void CommandExec::slotCmd_blame()
{
    if (!m_pCPart->end) {
        m_pCPart->end = svn::Revision::HEAD;
    }
    if (!m_pCPart->start) {
        m_pCPart->start = 1;
    }
    m_pCPart->m_SvnWrapper->makeBlame(m_pCPart->start, m_pCPart->end, m_pCPart->urls.at(0));
}

void CommandExec::slotCmd_cat()
{
    QMap<int, svn::Revision>::const_iterator cIt = m_pCPart->extraRevisions.constFind(0);
    if (cIt != m_pCPart->extraRevisions.constEnd()) {
        m_pCPart->rev_set = true;
        m_pCPart->start = cIt.value();
    } else {
        m_pCPart->end = svn::Revision::HEAD;
    }
    m_pCPart->m_SvnWrapper->slotMakeCat(
        (m_pCPart->rev_set ? m_pCPart->start : m_pCPart->end), m_pCPart->urls.at(0), m_pCPart->urls.at(0)
        , (m_pCPart->rev_set ? m_pCPart->start : m_pCPart->end), nullptr);
}

void CommandExec::slotCmd_get()
{
    if (m_pCPart->extraRevisions.find(0) != m_pCPart->extraRevisions.end()) {
        m_pCPart->rev_set = true;
        m_pCPart->start = m_pCPart->extraRevisions[0];
    } else {
        m_pCPart->end = svn::Revision::HEAD;
    }
    if (!m_pCPart->outfile_set || m_pCPart->outfile.isEmpty()) {
        clientException(i18n("\"GET\" requires output file"));
        return;
    }
    m_pCPart->m_SvnWrapper->makeGet((m_pCPart->rev_set ? m_pCPart->start : m_pCPart->end), m_pCPart->urls.at(0), m_pCPart->outfile,
                                    (m_pCPart->rev_set ? m_pCPart->start : m_pCPart->end));
}

void CommandExec::slotCmd_update()
{
    const svn::Targets targets = svn::Targets::fromStringList(m_pCPart->urls);
    m_pCPart->m_SvnWrapper->makeUpdate(targets,
                                       (m_pCPart->rev_set ? m_pCPart->start : svn::Revision::HEAD), svn::DepthUnknown);
}

void CommandExec::slotCmd_diff()
{
    if (m_pCPart->urls.count() == 1) {
        if (!m_pCPart->rev_set && !svn::Url::isValid(m_pCPart->urls.at(0))) {
            m_pCPart->start = svn::Revision::BASE;
            m_pCPart->end = svn::Revision::WORKING;
        }
        m_pCPart->m_SvnWrapper->makeDiff(m_pCPart->urls.at(0), m_pCPart->start, m_pCPart->urls.at(0), m_pCPart->end);
    } else {
        svn::Revision r1 = svn::Revision::HEAD;
        svn::Revision r2 = svn::Revision::HEAD;
        QMap<int, svn::Revision>::const_iterator cIt = m_pCPart->extraRevisions.constFind(0);
        if (cIt != m_pCPart->extraRevisions.constEnd()) {
            r1 = cIt.value();
        } else if (!svn::Url::isValid(m_pCPart->urls.at(0))) {
            r1 = svn::Revision::WORKING;
        }
        if (m_pCPart->extraRevisions.find(1) != m_pCPart->extraRevisions.end()) {
            r2 = m_pCPart->extraRevisions[1];
        } else if (!svn::Url::isValid(m_pCPart->urls.at(1))) {
            r2 = svn::Revision::WORKING;
        }
        m_pCPart->m_SvnWrapper->makeDiff(m_pCPart->urls.at(0), r1, m_pCPart->urls.at(1), r2);
    }
}

void CommandExec::slotCmd_info()
{
    QMap<int, svn::Revision>::const_iterator cIt = m_pCPart->extraRevisions.constFind(0);
    if (cIt != m_pCPart->extraRevisions.constEnd()) {
        m_pCPart->rev_set = true;
        m_pCPart->start = cIt.value();
    }
    m_pCPart->m_SvnWrapper->makeInfo(m_pCPart->urls, (m_pCPart->rev_set ? m_pCPart->start : m_pCPart->end),
                                     svn::Revision::UNDEFINED, false);
}

void CommandExec::slotCmd_commit()
{
    const svn::Targets targets(svn::Targets::fromStringList(m_pCPart->urls));
    m_pCPart->m_SvnWrapper->makeCommit(targets);
}

void CommandExec::slotCmd_list()
{
    svn::DirEntries res;
    svn::Revision rev = m_pCPart->end;
    if (m_pCPart->rev_set) {
        rev = m_pCPart->start;
    } else if (m_pCPart->extraRevisions[0]) {
        rev = m_pCPart->extraRevisions[0];
    }
    if (!m_pCPart->m_SvnWrapper->makeList(m_pCPart->urls.at(0), res, rev, svn::DepthInfinity)) {
        return;
    }
    Q_FOREACH(const svn::DirEntry &entry, res) {
        QString d = entry.time().toString(QStringLiteral("yyyy-MM-dd hh:mm::ss"));
        m_pCPart->Stdout
                << (entry.kind() == svn_node_dir ? "D" : "F") << " "
                << d << " "
                << entry.name() << endl;
    }
}

void CommandExec::slotCmd_copy()
{
    QString target;
    if (m_pCPart->urls.count() < 2) {
        bool ok;
        target = CopyMoveView_impl::getMoveCopyTo(&ok, false,
                                                  m_pCPart->urls.at(0), QString(), nullptr);
        if (!ok) {
            return;
        }
    } else {
        target = m_pCPart->urls.at(1);
    }
    QMap<int, svn::Revision>::const_iterator cIt = m_pCPart->extraRevisions.constFind(0);
    if (cIt != m_pCPart->extraRevisions.constEnd()) {
        m_pCPart->rev_set = true;
        m_pCPart->start = cIt.value();
    } else {
        m_pCPart->end = svn::Revision::HEAD;
    }
    m_pCPart->m_SvnWrapper->makeCopy(m_pCPart->urls.at(0), target, (m_pCPart->rev_set ? m_pCPart->start : m_pCPart->end));
}

void CommandExec::slotCmd_move()
{
    bool ok;
    QString target;
    if (m_pCPart->urls.count() < 2) {
        target = CopyMoveView_impl::getMoveCopyTo(&ok, true,
                                                  m_pCPart->urls.at(0), QString(), nullptr);
        if (!ok) {
            return;
        }
    } else {
        target = m_pCPart->urls.at(1);
    }
    m_pCPart->m_SvnWrapper->makeMove(m_pCPart->urls.at(0), target);
}

void CommandExec::slotCmd_delete()
{
    m_pCPart->m_SvnWrapper->makeDelete(m_pCPart->urls);
}

void CommandExec::slotCmd_add()
{
    m_pCPart->m_SvnWrapper->addItems(svn::Targets::fromStringList(m_pCPart->urls), svn::DepthInfinity);
}

void CommandExec::slotCmd_revert()
{
    m_pCPart->m_SvnWrapper->slotRevertItems(m_pCPart->urls);
}

void CommandExec::slotCmd_addnew()
{
    m_pCPart->m_SvnWrapper->checkAddItems(m_pCPart->urls.at(0));
}

/*!
    \fn CommandExec::scanRevision()
 */
bool CommandExec::scanRevision()
{
    const QString revstring = m_pCPart->parser->value(QStringLiteral("r"));
    const QVector<QStringRef> revl = revstring.splitRef(QLatin1Char(':'), QString::SkipEmptyParts);
    if (revl.isEmpty()) {
        return false;
    }
    m_pCPart->start = revl[0].toString();
    if (revl.count() > 1) {
        m_pCPart->end = revl[1].toString();
    }
    m_pCPart->rev_set = true;
    return true;
}

void CommandExec::slotNotifyMessage(const QString &msg)
{
    m_pCPart->m_SvnWrapper->slotExtraLogMsg(msg);
    if (Kdesvnsettings::self()->cmdline_show_logwindow()) {
        ++m_lastMessagesLines;
        if (!m_lastMessages.isEmpty()) {
            m_lastMessages.append("\n");
        }
        m_lastMessages.append(msg);
    }
}

bool CommandExec::askRevision()
{
    bool ret = false;
    Rangeinput_impl::revision_range range;
    if (Rangeinput_impl::getRevisionRange(range, true, m_pCPart->single_revision)) {
        m_pCPart->start = range.first;
        m_pCPart->end = range.second;
        m_pCPart->rev_set = true;
        ret = true;
    }
    return ret;
}


/*!
    \fn CommandExec::slotCmd_switch()
 */
void CommandExec::slotCmd_switch()
{
    if (m_pCPart->urls.count() > 1) {
        clientException(i18n("May only switch one URL at time"));
        return;
    }
    if (m_pCPart->repoUrls.find(0) == m_pCPart->repoUrls.end()) {
        clientException(i18n("Switch only on working copies"));
        return;
    }
    m_pCPart->m_SvnWrapper->makeSwitch(m_pCPart->urls.at(0),
                                       m_pCPart->repoUrls.value(0));
}

void CommandExec::slotCmd_lock()
{
//     m_pCPart->m_SvnWrapper->makeLock(m_pCPart->urls.at(0),"",m_pCPart->force);
    m_pCPart->m_SvnWrapper->makeLock(m_pCPart->urls, QString(), m_pCPart->force);
}

void CommandExec::slotCmd_unlock()
{
//     m_pCPart->m_SvnWrapper->makeUnlock(m_pCPart->urls.at(0),m_pCPart->force);
    m_pCPart->m_SvnWrapper->makeUnlock(m_pCPart->urls, m_pCPart->force);
}
