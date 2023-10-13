// SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <KAbstractFileItemActionPlugin>
#include <KFileItemListProperties>
#include <KIO/CommandLauncherJob>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QAction>
#include <QDBusInterface>
#include <QDBusReply>
#include <QMenu>

class KdeSvnActionsPlugin : public KAbstractFileItemActionPlugin
{
    Q_OBJECT
public:
    KdeSvnActionsPlugin(QObject *parent, const QVariantList &)
        : KAbstractFileItemActionPlugin(parent)
    {
    }

    QList<QAction *> fetchActions(const KFileItemListProperties &fileItemInfos, const QString &function, QWidget *parentWidget)
    {
        const QString url = fileItemInfos.urlList().constFirst().toString();
        QList<QAction *> actions;
        const auto createAction = [parentWidget, &actions, this](const QString &icon, const QString &text) {
            QAction *action = new QAction(QIcon::fromTheme(icon), text, parentWidget);
            connect(action, &QAction::triggered, this, [action]() {
                const QString exec = action->data().toString();
                auto job = new KIO::CommandLauncherJob(exec);
                job->setDesktopName(QStringLiteral("org.kde.kdesvn"));
                job->start();
            });
            actions << action;
            return action;
        };
        const auto createActionWithExec = [url, &createAction](const QString &icon, const QString &text, QLatin1String command) {
            auto action = createAction(icon, text);
            const QString fullCommand = QLatin1String("kdesvn exec ") + command + QLatin1Char(' ') + url;
            action->setData(fullCommand);
        };

        const QString app = QStringLiteral("org.kde.kded5");
        const QString object = QStringLiteral("/modules/kdesvnd");
        const QString interface = QStringLiteral("org.kde.kdesvnd");
        QDBusInterface remote(app, object, interface);
        if (QDBusReply<QStringList> reply = remote.call(function, QUrl::toStringList(fileItemInfos.urlList())); reply.isValid()) {
            const QStringList dbusActions = reply.value();
            if (dbusActions.contains(QLatin1String("Checkout"))) {
                createActionWithExec(QStringLiteral("kdesvncheckout"), i18n("Checkout From Repository..."), QLatin1String("checkout"));
            }
            if (dbusActions.contains(QLatin1String("Export"))) {
                createActionWithExec(QStringLiteral("kdesvnexport"), i18n("Export..."), QLatin1String("export"));
            }
            if (dbusActions.contains(QLatin1String("Update"))) {
                createActionWithExec(QStringLiteral("kdesvnupdate"), i18n("Update (Kdesvn)"), QLatin1String("update"));
            }
            if (dbusActions.contains(QLatin1String("Commit"))) {
                createActionWithExec(QStringLiteral("kdesvncommit"), i18n("Commit (Kdesvn)"), QLatin1String("commit"));
            }
            if (dbusActions.contains(QLatin1String("Log"))) {
                createActionWithExec(QStringLiteral("kdesvnlog"), i18n("kdesvn log (last 100)"), QLatin1String(" -r HEAD:1 -l 100 log"));
            }
            if (dbusActions.contains(QLatin1String("Exportto"))) {
                createActionWithExec(QStringLiteral("kdesvnexport"), i18n("Export from a Subversion repository..."), QLatin1String("exportto"));
            }
            if (dbusActions.contains(QLatin1String("Checkoutto"))) {
                createActionWithExec(QStringLiteral("kdesvncheckout"), i18n("Checkout from a repository..."), QLatin1String("checkoutto"));
            }

            // Normal menu
            if (dbusActions.contains(QLatin1String("Info"))) {
                createActionWithExec(QStringLiteral("kdesvninfo"), i18n("Detailed Subversion info"), QLatin1String("info"));
            }
            if (dbusActions.contains(QLatin1String("Add"))) {
                createActionWithExec(QStringLiteral("kdesvnadd"), i18n("Add to Repository"), QLatin1String("add"));
            }
            if (dbusActions.contains(QLatin1String("Addnew"))) {
                createActionWithExec(QStringLiteral("kdesvnaddrecursive"), i18n("Check for unversioned items"), QLatin1String("addnew"));
            }
            if (dbusActions.contains(QLatin1String("Delete"))) {
                createActionWithExec(QStringLiteral("kdesvndelete"), i18n("Delete From Repository"), QLatin1String("rm"));
            }
            if (dbusActions.contains(QLatin1String("Revert"))) {
                createActionWithExec(QStringLiteral("kdesvnreverse"), i18n("Revert Local Changes"), QLatin1String("revert"));
            }
            if (dbusActions.contains(QLatin1String("Rename"))) {
                createActionWithExec(QStringLiteral("kdesvnmove"), i18n("Rename..."), QLatin1String("mv"));
            }
            if (dbusActions.contains(QLatin1String("Import"))) {
                auto action = createAction(QStringLiteral("svn_import"), i18n("Import Repository"));
                const QString exec = QLatin1String("kio_svn_helper -i ") + url;
                action->setData(exec);
            }
            if (dbusActions.contains(QLatin1String("Switch"))) {
                createActionWithExec(QStringLiteral("kdesvnswitch"), i18n("Switch..."), QLatin1String("switch"));
            }
            if (dbusActions.contains(QLatin1String("Merge"))) {
                auto action = createAction(QStringLiteral("kdesvnmerge"), i18n("Merge..."));
                const QString exec = QLatin1String("kio_svn_helper -m ") + url;
                action->setData(exec);
            }
            if (dbusActions.contains(QLatin1String("Blame"))) {
                createActionWithExec(QStringLiteral("kdesvnblame"), i18n("Blame..."), QLatin1String("blame"));
            }
            if (dbusActions.contains(QLatin1String("CreatePatch"))) {
                auto action = createAction(QStringLiteral("kdesvn"), i18n("Create Patch..."));
                const QString exec = QLatin1String("kio_svn_helper -p ") + url;
                action->setData(exec);
            }
            if (dbusActions.contains(QLatin1String("Diff"))) {
                createActionWithExec(QStringLiteral("kdesvndiff"), i18n("Diff (local)"), QLatin1String("diff"));
            }
            if (dbusActions.contains(QLatin1String("Tree"))) {
                createActionWithExec(QStringLiteral("kdesvnlog"), i18n("Display revision tree"), QLatin1String("tree"));
            }
        } else {
            qWarning() << "Error when requesting kdesvn menu:" << reply.error();
        }
        return actions;
    }

    QList<QAction *> actions(const KFileItemListProperties &fileItemInfos, QWidget *parentWidget) override
    {
        QList<QAction *> actions = fetchActions(fileItemInfos, QStringLiteral("getTopLevelActionMenu"), parentWidget);
        auto menu = new QMenu(i18n("Subversion (kdesvn)"), parentWidget);
        menu->setIcon(QIcon::fromTheme(QStringLiteral("kdesvn")));
        auto submenuActions = fetchActions(fileItemInfos, QStringLiteral("getActionMenu"), menu);
        if (!submenuActions.isEmpty()) {
            menu->addActions(submenuActions);
            actions << menu->menuAction();
        }
        return actions;
    }
};
K_PLUGIN_CLASS_WITH_JSON(KdeSvnActionsPlugin, "kdesvnactionsplugin.json")

#include "kdesvnactionsplugin.moc"
