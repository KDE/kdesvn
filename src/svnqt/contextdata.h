/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this program (in the file LGPL.txt); if not,         *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at http://kdesvn.alwins-world.de.           *
 ***************************************************************************/
#ifndef SVNCONTEXTDATA_HPP
#define SVNCONTEXTDATA_HPP

#include "svnqt/pool.h"
#include "svnqt/apr.h"
#include "svnqt/commititem.h"
#include "svnqt/svnqt_defines.h"

#include <svn_client.h>
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 4)
#include <svn_auth.h>
#endif
#include <qstring.h>

struct svn_wc_conflict_result_t;
struct svn_wc_conflict_description_t;

namespace svn {

    class ContextListener;
/**
	@author Rajko Albrecht <ral@alwins-world.de>
*/

class SVNQT_NOEXPORT ContextData{
public:
    ContextData(const QString & configDir_);
    ~ContextData();

    // data methods
    svn_client_ctx_t*ctx();
    const QString&configDir()const;
    void setListener (ContextListener * listener);
    ContextListener * getListener () const;
    void reset();

    // svn methods
    void setAuthCache(bool value);
    /** @see Context::setLogin */
    void setLogin (const QString& usr, const QString& pwd);
    /** @see Context::setLogMessage */
    void setLogMessage (const QString& msg);
    const QString&getLogMessage ()const;
    /**
     * if the @a listener is set, use it to retrieve the log
     * message using ContextListener::contextGetLogMessage.
     * This return values is given back, then.
     *
     * if the @a listener is not set the its checked whether
     * the log message has been set using @a setLogMessage
     * yet. If not, return false otherwise true
     *
     * @param msg log message
     * @retval false cancel
     */
    bool retrieveLogMessage (QString & msg,const CommitItemList&);

    bool contextAddListItem(DirEntries*entries, const svn_dirent_t*dirent,const svn_lock_t*lock,const QString&path);

    /**
     * if the @a listener is set call the method
     * @a contextNotify
     */
    void notify (const char *path,
            svn_wc_notify_action_t action,
            svn_node_kind_t kind,
            const char *mime_type,
            svn_wc_notify_state_t content_state,
            svn_wc_notify_state_t prop_state,
            svn_revnum_t revision);
    void notify (const svn_wc_notify_t *action);
    /**
     * if the @a listener is set call the method
     * @a contextCancel
     */
    bool cancel();
    const QString& getUsername () const;
    const QString& getPassword () const;

    /**
     * if the @a listener is set and no password has been
     * set yet, use it to retrieve login and password using
     * ContextListener::contextGetLogin.
     *
     * if the @a listener is not set, check if setLogin
     * has been called yet.
     *
     * @return continue?
     * @retval false cancel
     */
    bool
    retrieveLogin (const char * username_,
                   const char * realm,
                   bool &may_save);
    /**
     * if the @a listener is set and no password has been
     * set yet, use it to retrieve login and password using
     * ContextListener::contextGetSavedLogin.
     *
     * if the @a listener is not set, check if setLogin
     * has been called yet.
     *
     * @return continue?
     * @retval false cancel
     */
    bool
    retrieveSavedLogin(const char * username_,
                       const char * realm,
                       bool &may_save);
    /**
     * if the @a listener is set and no password has been
     * set yet, use it to retrieve login and password using
     * ContextListener::contextGetCachedLogin.
     *
     * if the @a listener is not set, check if setLogin
     * has been called yet.
     *
     * @return continue?
     * @retval false cancel
     */
    bool
    retrieveCachedLogin(const char * username_,
                       const char * realm,
                       bool &may_save);

protected:
    // static methods
    /**
     * the @a baton is interpreted as ContextData *
     * Several checks are performed on the baton:
     * - baton == 0?
     * - baton->Data
     * - listener set?
     *
     * @param baton
     * @param data returned data if everything is OK
     * @retval SVN_NO_ERROR if everything is fine
     * @retval SVN_ERR_CANCELLED on invalid values
     */
    static svn_error_t *
    getContextData (void * baton, ContextData ** data);

    /**
     * this function gets called by the subversion api function
     * when a log message is needed. This is the case on a commit
     * for example
     */
    static svn_error_t *
    onLogMsg (const char **log_msg,
              const char **tmp_file,
              apr_array_header_t * commit_items,
              void *baton,
              apr_pool_t * pool);

    /**
     * this function gets called by the subversion api function
     * when a log message is needed. This is the case on a commit
     * for example
     */
    static svn_error_t *
    onLogMsg2 (const char **log_msg,
              const char **tmp_file,
              const apr_array_header_t * commit_items,
              void *baton,
              apr_pool_t * pool);

#if  ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5) || (SVN_VER_MAJOR > 2))
    /**
     * this function gets called by the subversion api function
     * when a log message is needed. This is the case on a commit
     * for example
     */
    static svn_error_t *
    onLogMsg3 (const char **log_msg,
              const char **tmp_file,
              const apr_array_header_t * commit_items,
              void *baton,
              apr_pool_t * pool);
#endif

    /**
     * this is the callback function for the subversion
     * api functions to signal the progress of an action
     */
    static void
    onNotify (void * baton,
              const char *path,
              svn_wc_notify_action_t action,
              svn_node_kind_t kind,
              const char *mime_type,
              svn_wc_notify_state_t content_state,
              svn_wc_notify_state_t prop_state,
              svn_revnum_t revision);
    /**
     * this is the callback function for the subversion 1.2
     * api functions to signal the progress of an action
     *
     * @todo right now we forward only to @a onNotify,
     *       but maybe we should a notify2 to the listener
     * @since subversion 1.2
     */
    static void
    onNotify2(void*baton,const svn_wc_notify_t *action,apr_pool_t */*tpool*/);
    /**
     * this is the callback function for the subversion
     * api functions to signal the progress of an action
     * @param baton pointer to a ContextData instance
     */
    static svn_error_t * onCancel (void * baton);

    /**
     * @see svn_auth_simple_prompt_func_t
     * this method is an alternate and called ONEs before onSimplePrompt.
     * So we can try load password from other source.
     */
    static svn_error_t *
    onCachedPrompt(svn_auth_cred_simple_t **cred,
                           void *baton,
                           const char *realm,
                           const char *username,
                           svn_boolean_t _may_save,
                           apr_pool_t *pool);

    /**
     * @see svn_auth_simple_prompt_func_t
     * this method is an alternate and called ONEs before onSimplePrompt.
     * So we can try load password from other source.
     */
    static svn_error_t *
    onSavedPrompt(svn_auth_cred_simple_t **cred,
                           void *baton,
                           const char *realm,
                           const char *username,
                           svn_boolean_t _may_save,
                           apr_pool_t *pool);
    /**
     * @see svn_auth_simple_prompt_func_t
     */
    static svn_error_t *
    onSimplePrompt (svn_auth_cred_simple_t **cred,
                    void *baton,
                    const char *realm,
                    const char *username,
                    svn_boolean_t _may_save,
                    apr_pool_t *pool);
    /**
     * @see svn_auth_ssl_server_trust_prompt_func_t
     */
    static svn_error_t *
    onSslServerTrustPrompt (svn_auth_cred_ssl_server_trust_t **cred,
                            void *baton,
                            const char *realm,
                            apr_uint32_t failures,
                            const svn_auth_ssl_server_cert_info_t *info,
                            svn_boolean_t may_save,
                            apr_pool_t *pool);
    static svn_error_t *
    onSslClientCertPrompt (svn_auth_cred_ssl_client_cert_t **cred,
                           void *baton,
                           apr_pool_t *pool);
    /**
     * @see svn_auth_ssl_client_cert_pw_prompt_func_t
     */
    static svn_error_t *
    onFirstSslClientCertPw (
            svn_auth_cred_ssl_client_cert_pw_t **cred,
            void *baton,
            const char *realm,
            svn_boolean_t maySave,
            apr_pool_t *pool);

    /**
     * @see svn_auth_ssl_client_cert_pw_prompt_func_t
     */
    static svn_error_t *
    onSslClientCertPwPrompt (
            svn_auth_cred_ssl_client_cert_pw_t **cred,
            void *baton,
            const char *realm,
            svn_boolean_t maySave,
            apr_pool_t *pool);

    /**
     * @see svn_client_ctx_t::progress_func
     */
    static void onProgress(apr_off_t progress, apr_off_t total, void *baton, apr_pool_t *pool);

    /**
     * @see svn_wc_conflict_resolver_func_t
     * @since subversion 1.5
     */
    static svn_error_t* onWcConflictResolver(svn_wc_conflict_result_t**result,const svn_wc_conflict_description_t *description, void *baton, apr_pool_t *pool);

    /**
     * @see svn_auth_plaintext_prompt_func_t
     * @since subversion 1.6
     */
    static svn_error_t* maySavePlaintext(svn_boolean_t *may_save_plaintext, const char *realmstring, void *baton, apr_pool_t *pool);

    // extra methods
    svn_error_t *
        generate_cancel_error();

#if  ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5) || (SVN_VER_MAJOR > 1))
    /** read in mimetypes map
     * @since subversion 1.5
     */
    void initMimeTypes();
#endif
protected:
    Apr apr;

    ContextListener * listener;
    bool logIsSet;
    int m_promptCounter;
    Pool pool;
    svn_client_ctx_t*m_ctx;
    QString username;
    QString password;
    QString logMessage;
    QString m_ConfigDir;

};

}

#endif
