/***************************************************************************
 *   Copyright (C) 2005-2007 by Rajko Albrecht                             *
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
#include "kiolistener.h"
#include "kiosvn.h"

#include <kdebug.h>
#include <klocale.h>
#include <dcopclient.h>

KioListener::KioListener(kio_svnProtocol*_par)
 : svn::ContextListener(),m_notifyCounter(0),m_External(false),m_HasChanges(false),m_FirstTxDelta(false)
{
    par = _par;
}


KioListener::~KioListener()
{
}




/*!
    \fn KioListener::contextCancel()
 */
bool KioListener::contextCancel()
{
    return par->wasKilled();
}


/*!
    \fn KioListener::contextGetLogMessage (QString & msg)
 */
bool KioListener::contextGetLogMessage (QString & msg,const svn::CommitItemList&_items)
{
#if 1
    QByteArray reply;
    QByteArray params;
    QCString replyType;
    QDataStream stream(params,IO_WriteOnly);

    if (_items.count()>0) {
        QMap<QString,QString> list;
        for (unsigned i = 0;i<_items.count();++i) {
            if (_items[i].path().isEmpty()) {
                list[_items[i].url()]=QChar(_items[i].actionType());
            } else {
                list[_items[i].path()]=QChar(_items[i].actionType());
            }
        }
        stream << list;
        if (!par->dcopClient()->call("kded","kdesvnd","get_logmsg(QMap<QString,QString>)",params,replyType,reply)) {
            msg = "Communication with dcop failed";
            kdWarning()<<msg<<endl;
            return false;
        }
    } else {
        if (!par->dcopClient()->call("kded","kdesvnd","get_logmsg()",params,replyType,reply)) {
            msg = "Communication with dcop failed";
            kdWarning()<<msg<<endl;
            return false;
        }
    }

    if (replyType!="QStringList") {
        msg = "Wrong reply type";
        kdWarning()<<msg<<endl;
        return false;
    }
    QDataStream stream2(reply,IO_ReadOnly);
    QStringList lt;
    stream2>>lt;
    if (lt.count()!=1) {
        msg = "Wrong or missing log (may cancel pressed).";
        kdDebug()<< msg << endl;
        return false;
    }
    msg = lt[0];
#else
    msg = "Made with a kio::svn client";
#endif
    return true;
}

/*! the content of that method is taken from the notify in kio::svn in KDE SDK */
/* this moment we don't use it full 'cause not all is made via KIO */
void KioListener::contextNotify (const char * path,
                    svn_wc_notify_action_t action,
                    svn_node_kind_t kind ,
                    const char * mime_type ,
                    svn_wc_notify_state_t content_state,
                    svn_wc_notify_state_t prop_state,
                    svn_revnum_t revision)
{
    if (par->wasKilled()) {
        return;
    }
    QString userstring;

    switch(action) {
        case svn_wc_notify_add:
        {
            if (mime_type && (svn_mime_type_is_binary (mime_type)))
                userstring = i18n( "A (bin) %1" ).arg( path );
            else
                userstring = i18n( "A %1" ).arg( path );
            break;
        }
        break;
        case svn_wc_notify_copy: //copy
            break;
        case svn_wc_notify_delete: //delete
            m_HasChanges = TRUE;
            userstring = i18n( "D %1" ).arg( path );
            break;
        case svn_wc_notify_restore : //restore
            userstring=i18n( "Restored %1." ).arg( path );
            break;
        case svn_wc_notify_revert : //revert
            userstring=i18n( "Reverted %1." ).arg( path );
            break;
        case svn_wc_notify_failed_revert: //failed revert
            userstring=i18n( "Failed to revert %1.\nTry updating instead." ).arg( path );
            break;
        case svn_wc_notify_resolved: //resolved
            userstring=i18n( "Resolved conflicted state of %1." ).arg( path );
            break;
        case svn_wc_notify_skip: //skip
            if ( content_state == svn_wc_notify_state_missing )
                userstring=i18n("Skipped missing target %1.").arg( path );
            else
                userstring=i18n("Skipped %1.").arg( path );
            break;
        case svn_wc_notify_update_delete: //update_delete
            m_HasChanges = TRUE;
            userstring=i18n( "D %1" ).arg( path );
            break;
        case svn_wc_notify_update_add: //update_add
            m_HasChanges = TRUE;
            userstring=i18n( "A %1" ).arg( path );
            break;
        case svn_wc_notify_update_update: //update_update
            {
                /* If this is an inoperative dir change, do no notification.
                   An inoperative dir change is when a directory gets closed
                   without any props having been changed. */
                if (! ((kind == svn_node_dir)
                            && ((prop_state == svn_wc_notify_state_inapplicable)
                                || (prop_state == svn_wc_notify_state_unknown)
                                || (prop_state == svn_wc_notify_state_unchanged)))) {
                    m_HasChanges = TRUE;

                    if (kind == svn_node_file) {
                        if (content_state == svn_wc_notify_state_conflicted)
                            userstring = "C";
                        else if (content_state == svn_wc_notify_state_merged)
                            userstring = "G";
                        else if (content_state == svn_wc_notify_state_changed)
                            userstring = "U";
                    }

                    if (prop_state == svn_wc_notify_state_conflicted)
                        userstring += "C";
                    else if (prop_state == svn_wc_notify_state_merged)
                        userstring += "G";
                    else if (prop_state == svn_wc_notify_state_changed)
                        userstring += "U";
                    else
                        userstring += " ";

                    if (! ((content_state == svn_wc_notify_state_unchanged
                                    || content_state == svn_wc_notify_state_unknown)
                                && (prop_state == svn_wc_notify_state_unchanged
                                    || prop_state == svn_wc_notify_state_unknown)))
                        userstring += QString( " " ) + path;
                }
                break;
            }
        case svn_wc_notify_update_completed: //update_completed
            {
                if (!m_External) {
                    if (SVN_IS_VALID_REVNUM(revision)) {
                        userstring = i18n("Finished at revision %1.").arg(revision);
                    } else {
                        userstring = i18n("Finished.");
                    }
                } else {
                    if (SVN_IS_VALID_REVNUM(revision)) {
                        userstring = i18n("Finished external at revision %1.").arg(revision);
                    } else {
                        userstring = i18n("Finished external.");
                    }
                }

#if 0
                if (! nb->suppress_final_line) {
                    if (SVN_IS_VALID_REVNUM (revision)) {
                        if (nb->is_export) {
                            if ( m_External )
                                userstring = i18n("Exported external at revision %1.").arg( revision );
                            else
                                userstring = i18n("Exported revision %1.").arg( revision );
                        } else if (nb->is_checkout) {
                            if ( m_External )
                                userstring = i18n("Checked out external at revision %1.").arg( revision );
                            else
                                userstring = i18n("Checked out revision %1.").arg( revision);
                        } else {
                            if (m_HasChanges) {
                                if ( m_External )
                                    userstring=i18n("Updated external to revision %1.").arg( revision );
                                else
                                    userstring = i18n("Updated to revision %1.").arg( revision);
                            } else {
                                if ( m_External )
                                    userstring = i18n("External at revision %1.").arg( revision );
                                else
                                    userstring = i18n("At revision %1.").arg( revision);
                            }
                        }
                    } else  /* no revision */ {
                        if (nb->is_export) {
                            if ( m_External )
                                userstring = i18n("External export complete.");
                            else
                                userstring = i18n("Export complete.");
                        } else if (nb->is_checkout) {
                            if ( m_External )
                                userstring = i18n("External checkout complete.");
                            else
                                userstring = i18n("Checkout complete.");
                        } else {
                            if ( m_External )
                                userstring = i18n("External update complete.");
                            else
                                userstring = i18n("Update complete.");
                        }
                    }
                }
#endif
            }
            if (m_External)
                m_External = FALSE;
            break;
        case svn_wc_notify_update_external: //update_external
            m_External = TRUE;
            userstring = i18n("Fetching external item into %1." ).arg( path );
            break;
        case svn_wc_notify_status_completed: //status_completed
            if (SVN_IS_VALID_REVNUM (revision))
                userstring = i18n( "Status against revision: %1.").arg( revision );
            break;
        case svn_wc_notify_status_external: //status_external
             userstring = i18n("Performing status on external item at %1.").arg( path );
            break;
        case svn_wc_notify_commit_modified: //commit_modified
            userstring = i18n( "Sending %1.").arg( path );
            break;
        case svn_wc_notify_commit_added: //commit_added
            if (mime_type && svn_mime_type_is_binary (mime_type)) {
                userstring = i18n( "Adding (bin) %1.").arg( path );
            } else {
                userstring = i18n( "Adding %1.").arg( path );
            }
            break;
        case svn_wc_notify_commit_deleted: //commit_deleted
            userstring = i18n( "Deleting %1.").arg( path );
            break;
        case svn_wc_notify_commit_replaced: //commit_replaced
            userstring = i18n( "Replacing %1.").arg( path );
            break;
        case svn_wc_notify_commit_postfix_txdelta: //commit_postfix_txdelta
            if (!m_FirstTxDelta) {
                m_FirstTxDelta = TRUE;
		// check fullstops!
                userstring=i18n("Transmitting file data ");
            } else {
                userstring=".";
            }
            break;

            break;
        case svn_wc_notify_blame_revision: //blame_revision
            break;
        default:
            break;
    }
    par->setMetaData(QString::number(counter()).rightJustify( 10,'0' )+ "path" , QString::FROMUTF8( path ));
    par->setMetaData(QString::number( counter() ).rightJustify( 10,'0' )+ "action", QString::number( action ));
    par->setMetaData(QString::number( counter() ).rightJustify( 10,'0' )+ "kind", QString::number( kind ));
    par->setMetaData(QString::number( counter() ).rightJustify( 10,'0' )+ "mime_t", QString::FROMUTF8( mime_type ));
    par->setMetaData(QString::number( counter() ).rightJustify( 10,'0' )+ "content", QString::number( content_state ));
    par->setMetaData(QString::number( counter() ).rightJustify( 10,'0' )+ "prop", QString::number( prop_state ));
    par->setMetaData(QString::number( counter() ).rightJustify( 10,'0' )+ "rev", QString::number( revision ));
    par->setMetaData(QString::number( counter() ).rightJustify( 10,'0' )+ "string", userstring );
    incCounter();
}

void KioListener::contextNotify (const svn_wc_notify_t *action)
{
    if (!action) return;
//    if (action->action<svn_wc_notify_locked) {
        contextNotify(action->path,action->action,action->kind,action->mime_type,
            action->content_state,action->prop_state,action->revision);
//        return;
//    }
//    QString aString = NotifyAction(action->action);
}

svn::ContextListener::SslServerTrustAnswer
KioListener::contextSslServerTrustPrompt (const SslServerTrustData & data,
                                 apr_uint32_t & acceptedFailures)
{
    QByteArray reply;
    QByteArray params;
    QCString replyType;
    QDataStream stream(params,IO_WriteOnly);
    stream << data.hostname
        << data.fingerprint
        << data.validFrom
        << data.validUntil
        << data.issuerDName
        << data.realm;

    if (!par->dcopClient()->call("kded","kdesvnd",
        "get_sslaccept(QString,QString,QString,QString,QString,QString)",
        params,replyType,reply)) {
        kdWarning()<<"Communication with dcop failed"<<endl;
        return DONT_ACCEPT;
    }
    if (replyType!="int") {
        kdWarning()<<"Wrong reply type"<<endl;
        return DONT_ACCEPT;
    }
    QDataStream stream2(reply,IO_ReadOnly);
    int res;
    stream2>>res;
    switch (res) {
        case -1:
            return DONT_ACCEPT;
            break;
        case 1:
            return ACCEPT_PERMANENTLY;
            break;
        default:
        case 0:
            return ACCEPT_TEMPORARILY;
            break;
    }
    /* avoid compiler warnings */
    return ACCEPT_TEMPORARILY;
}

bool KioListener::contextLoadSslClientCertPw(QString&password,const QString&realm)
{
    return pws.getCertPw(realm,password);
}

bool KioListener::contextSslClientCertPrompt (QString & certFile)
{
    QByteArray reply;
    QByteArray params;
    QCString replyType;
    if (!par->dcopClient()->call("kded","kdesvnd",
        "get_sslclientcertfile()",
        params,replyType,reply)) {
        kdWarning()<<"Communication with dcop failed"<<endl;
        return false;
    }
    if (replyType!="QString") {
        kdWarning()<<"Wrong reply type"<<endl;
        return false;
    }
    QDataStream stream2(reply,IO_ReadOnly);
    stream2>>certFile;
    if (certFile.isEmpty()) {
        return false;
    }
    return true;
}

bool KioListener::contextSslClientCertPwPrompt (QString & password,
                                   const QString & realm, bool & maySave)
{
    return false;
}

bool KioListener::contextGetSavedLogin (const QString & realm,QString & username,QString & password)
{
    pws.getLogin(realm,username,password);
    return true;
}


bool KioListener::contextGetLogin (const QString & realm,
                     QString & username,
                     QString & password,
                     bool & maySave)
{
    QByteArray reply;
    QByteArray params;
    QCString replyType;

    QDataStream stream(params,IO_WriteOnly);
    stream << realm;
    stream << username;

    if (!par->dcopClient()->call("kded","kdesvnd","get_login(QString,QString)",params,replyType,reply)) {
        kdWarning()<<"Communication with dcop failed"<<endl;
        return false;
    }
    if (replyType!="QStringList") {
        kdWarning()<<"Wrong reply type"<<endl;
        return false;
    }
    QDataStream stream2(reply,IO_ReadOnly);
    QStringList lt;
    stream2>>lt;
    if (lt.count()!=3) {
        kdDebug()<<"Wrong or missing auth list (may cancel pressed)." << endl;
        return false;
    }
    username = lt[0];
    password = lt[1];
    maySave = lt[2]=="true";
    return true;
}


/*!
    \fn KioListener::contextProgress(long long int current, long long int max)
 */
void KioListener::contextProgress(long long int cur, long long int max)
{
    if (par) {
        par->contextProgress(cur,max);
    }
}
