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
#ifndef SVNCONTEXTDATA_H
#define SVNCONTEXTDATA_H

#include "pool.hpp"
#include "apr.hpp"
#include "commititem.hpp"
#include "svnqt_defines.hpp"

#include <svn_client.h>
#include <qstring.h>

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
              apr_array_header_t *, //UNUSED commit_items
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
              const apr_array_header_t *, //UNUSED commit_items
              void *baton,
              apr_pool_t * pool);

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
    onSslClientCertPwPrompt (
            svn_auth_cred_ssl_client_cert_pw_t **cred,
            void *baton,
            const char *realm,
            svn_boolean_t maySave,
            apr_pool_t *pool);

    svn_error_t *
        generate_cancel_error();
protected:
    Apr apr;

    ContextListener * listener;
    bool logIsSet;
    int m_promptCounter;
    Pool pool;
    svn_client_ctx_t m_ctx;
    QString username;
    QString password;
    QString logMessage;
    QString m_ConfigDir;

};

}

#endif
