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


#include "contextdata.h"
#include "context_listener.h"
#include "conflictresult.h"
#include "conflictdescription.h"

#include <svn_config.h>
#include <svn_wc.h>

namespace svn {

ContextData::ContextData(const QString & configDir_)
    : listener (0), logIsSet (false),
        m_promptCounter (0), m_ConfigDir(configDir_)
{
    const char * c_configDir = 0;
    if( m_ConfigDir.length () > 0 )
    {
        c_configDir = m_ConfigDir.TOUTF8();
    }

    // make sure the configuration directory exists
    svn_config_ensure (c_configDir, pool);

      // initialize authentication providers
      // * simple
      // * username
      // * simple pw cache of frontend app
      // * simple pw storage
      // * simple prompt
      // * ssl server trust file
      // * ssl server trust prompt
      // * ssl client cert pw file
      // * ssl client cert pw load
      // * ssl client cert pw prompt
      // * ssl client cert file
      // ===================
      // 11 providers (+1 for windowsvariant)

    apr_array_header_t *providers =
#if defined(WIN32) && (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 4)
        apr_array_make (pool, 12, sizeof (svn_auth_provider_object_t *));
#else
        apr_array_make (pool, 11, sizeof (svn_auth_provider_object_t *));
#endif
    svn_auth_provider_object_t *provider;

#if defined(WIN32) && (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 4)
    svn_auth_get_windows_simple_provider (&provider, pool);
	APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
#endif

#if  ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 6) || (SVN_VER_MAJOR > 2))
    svn_auth_get_simple_provider2
    (&provider, maySavePlaintext,this, pool);
#else
#if (SVN_VER_MINOR >= 4)
    svn_auth_get_simple_provider
#else
    svn_client_get_simple_provider
#endif
    (&provider, pool);
#endif
    *(svn_auth_provider_object_t **)apr_array_push (providers) = provider;

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 4)
    svn_auth_get_username_provider
#else
    svn_client_get_username_provider
#endif
    (&provider,pool);
    *(svn_auth_provider_object_t **)apr_array_push (providers) = provider;

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 4)
    svn_auth_get_simple_prompt_provider
#else
    svn_client_get_simple_prompt_provider
#endif
    (&provider,onCachedPrompt,this,0,pool);
    *(svn_auth_provider_object_t **)apr_array_push (providers) = provider;

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 4)
    svn_auth_get_simple_prompt_provider
#else
    svn_client_get_simple_prompt_provider
#endif
    (&provider,onSavedPrompt,this,0,pool);
    *(svn_auth_provider_object_t **)apr_array_push (providers) = provider;

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 4)
      svn_auth_get_simple_prompt_provider
#else
      svn_client_get_simple_prompt_provider
#endif
    /* not very nice. should be infinite... */
    (&provider,onSimplePrompt,this,100000000,pool);
    *(svn_auth_provider_object_t **)apr_array_push (providers) = provider;

    // add ssl providers

    // file first then prompt providers
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 4)
    svn_auth_get_ssl_server_trust_file_provider
#else
    svn_client_get_ssl_server_trust_file_provider
#endif
    (&provider, pool);
    *(svn_auth_provider_object_t **)apr_array_push (providers) = provider;

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 4)
    svn_auth_get_ssl_client_cert_file_provider
#else
    svn_client_get_ssl_client_cert_file_provider
#endif
    (&provider, pool);
    *(svn_auth_provider_object_t **)apr_array_push (providers) = provider;

#if  ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 6) || (SVN_VER_MAJOR > 2))
    svn_auth_get_ssl_client_cert_pw_file_provider2
    (&provider, maySavePlaintext,this, pool);
#else
#if (SVN_VER_MINOR >= 4)
    svn_auth_get_ssl_client_cert_pw_file_provider
#else
    svn_client_get_ssl_client_cert_pw_file_provider
#endif
    (&provider, pool);
#endif
    *(svn_auth_provider_object_t **)apr_array_push (providers) = provider;

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 4)
    svn_auth_get_ssl_server_trust_prompt_provider
#else
    svn_client_get_ssl_server_trust_prompt_provider
#endif
    (&provider, onSslServerTrustPrompt, this, pool);
    *(svn_auth_provider_object_t **)apr_array_push (providers) = provider;

    // first try load from extra storage
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 4)
    svn_auth_get_ssl_client_cert_pw_prompt_provider
#else
    svn_client_get_ssl_client_cert_pw_prompt_provider
#endif
    (&provider, onFirstSslClientCertPw, this, 0, pool);
    *(svn_auth_provider_object_t **)apr_array_push (providers) = provider;

    // plugged in 3 as the retry limit - what is a good limit?
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 4)
    svn_auth_get_ssl_client_cert_pw_prompt_provider
#else
    svn_client_get_ssl_client_cert_pw_prompt_provider
#endif
    (&provider, onSslClientCertPwPrompt, this, 3, pool);
    *(svn_auth_provider_object_t **)apr_array_push (providers) = provider;

      svn_auth_baton_t *ab;
      svn_auth_open (&ab, providers, pool);

      // initialize ctx structure
      svn_client_create_context(&m_ctx,pool);

      // get the config based on the configDir passed in
      svn_config_get_config ( &(m_ctx->config), c_configDir, pool);

      // tell the auth functions where the config is
      if (c_configDir) {
        svn_auth_set_parameter(ab, SVN_AUTH_PARAM_CONFIG_DIR,
            c_configDir);
      }

      m_ctx->auth_baton = ab;
      m_ctx->notify_func = onNotify;
      m_ctx->notify_baton = this;
      m_ctx->cancel_func = onCancel;
      m_ctx->cancel_baton = this;
      m_ctx->notify_func2 = onNotify2;
      m_ctx->notify_baton2 = this;

      m_ctx->log_msg_func = onLogMsg;
      m_ctx->log_msg_baton = this;
      // subversion 1.3 functions
      m_ctx->log_msg_func2 = onLogMsg2;
      m_ctx->log_msg_baton2 = this;

      m_ctx->progress_func = onProgress;
      m_ctx->progress_baton = this;

#if  ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5) || (SVN_VER_MAJOR > 2))
      m_ctx->log_msg_func3 = onLogMsg3;
      m_ctx->log_msg_baton3 = this;

      m_ctx->conflict_func = onWcConflictResolver;
      m_ctx->conflict_baton = this;

      m_ctx->client_name = "SvnQt wrapper client";
      initMimeTypes();
#endif
}


ContextData::~ContextData()
{
}


const QString&ContextData::getLogMessage () const
{
    return logMessage;
}

bool ContextData::retrieveLogMessage (QString & msg,const CommitItemList&_itemlist)
{
    bool ok = false;
    if (listener) {
        ok = listener->contextGetLogMessage (logMessage,_itemlist);
        if (ok)
            msg = logMessage;
        else
            logIsSet = false;
    }
    return ok;
}

void ContextData::notify(const char *path,
            svn_wc_notify_action_t action,
            svn_node_kind_t kind,
            const char *mime_type,
            svn_wc_notify_state_t content_state,
            svn_wc_notify_state_t prop_state,
            svn_revnum_t revision)
{
    if (listener != 0) {
        listener->contextNotify (path, action, kind, mime_type,
                                 content_state, prop_state, revision);
    }
}

void ContextData::notify (const svn_wc_notify_t *action)
{
    if (listener != 0) {
        listener->contextNotify(action);
    }
}

bool ContextData::cancel()
{
    if (listener != 0) {
        return listener->contextCancel ();
    } else {
        // don't cancel if no listener
        return false;
    }
}
const QString& ContextData::getUsername () const
{
    return username;
}

const QString& ContextData::getPassword() const
{
    return password;
}

bool ContextData::retrieveLogin (const char * username_,
                   const char * realm,
                   bool &may_save)
{
    bool ok;

    if (listener == 0)
        return false;

    username = QString::FROMUTF8(username_);
    ok = listener->contextGetLogin(QString::FROMUTF8(realm),username, password, may_save);

    return ok;
}

bool ContextData::retrieveSavedLogin (const char * username_,
                                 const char * realm,
                                 bool & may_save)
{
    bool ok;
    may_save = false;

    if (listener == 0)
        return false;

    username = QString::FROMUTF8(username_);
    ok = listener->contextGetSavedLogin(QString::FROMUTF8(realm),username, password);
    return ok;
}

bool ContextData::retrieveCachedLogin (const char * username_,
                                 const char * realm,
                                 bool & may_save)
{
    bool ok;
    may_save = false;

    if (listener == 0)
        return false;

    username = QString::FROMUTF8(username_);
    ok = listener->contextGetCachedLogin(QString::FROMUTF8(realm),username, password);
    return ok;
}

svn_client_ctx_t *ContextData::ctx()
{
    return m_ctx;
}

const QString&ContextData::configDir()const
{
    return m_ConfigDir;
}

svn_error_t *
ContextData::getContextData (void * baton, ContextData ** data)
{
    if (baton == NULL)
        return svn_error_create (SVN_ERR_CANCELLED, NULL,
                                 QObject::tr("invalid baton").TOUTF8());

    ContextData * data_ = static_cast <ContextData*>(baton);

    if (data_->listener == 0)
        return svn_error_create (SVN_ERR_CANCELLED, NULL,
                                 QObject::tr("invalid listener").TOUTF8());

    *data = data_;
    return SVN_NO_ERROR;
}

void ContextData::setAuthCache(bool value)
{
    void *param = 0;
    if (!value) {
        param = (void *)"1";
    }
    svn_auth_set_parameter (m_ctx->auth_baton,
        SVN_AUTH_PARAM_NO_AUTH_CACHE,param);
}

void ContextData::setLogin(const QString& usr, const QString& pwd)
{
    username = usr;
    password = pwd;
    svn_auth_baton_t * ab = m_ctx->auth_baton;
    svn_auth_set_parameter (ab, SVN_AUTH_PARAM_DEFAULT_USERNAME, username.TOUTF8());
    svn_auth_set_parameter (ab, SVN_AUTH_PARAM_DEFAULT_PASSWORD, password.TOUTF8());
}

void ContextData::setLogMessage (const QString& msg)
{
    logMessage = msg;
    if (msg.isNull()) {
        logIsSet = false;
    } else {
        logIsSet = true;
    }
}

svn_error_t *ContextData::onLogMsg (const char **log_msg,
              const char **tmp_file,
              apr_array_header_t * commit_items,
              void *baton,
              apr_pool_t * pool)
{
    ContextData * data = 0;
    SVN_ERR (getContextData (baton, &data));

    QString msg;
    if (data->logIsSet) {
        msg = data->getLogMessage ();
    } else {
        CommitItemList _items;
        for (int j = 0; j < commit_items->nelts; ++j) {
            svn_client_commit_item_t*item = ((svn_client_commit_item_t **)commit_items->elts)[j];
            _items.push_back(CommitItem(item));
        }
        if (!data->retrieveLogMessage (msg,_items)) {
            return data->generate_cancel_error();
        }
    }

    *log_msg = apr_pstrdup (pool,msg.TOUTF8());
    *tmp_file = NULL;
    return SVN_NO_ERROR;
}

svn_error_t *ContextData::onLogMsg2 (const char **log_msg,
              const char **tmp_file,
              const apr_array_header_t * commit_items,
              void *baton,
              apr_pool_t * pool)
{
    ContextData * data = 0;
    SVN_ERR (getContextData (baton, &data));

    QString msg;
    if (data->logIsSet) {
        msg = data->getLogMessage ();
    } else {
        CommitItemList _items;
        for (int j = 0; j < commit_items->nelts; ++j) {
            svn_client_commit_item2_t*item = ((svn_client_commit_item2_t **)commit_items->elts)[j];
            _items.push_back(CommitItem(item));
        }

        if (!data->retrieveLogMessage (msg,_items)) {
            return data->generate_cancel_error();
        }
    }

    *log_msg = apr_pstrdup (pool,msg.TOUTF8());
    *tmp_file = NULL;
    return SVN_NO_ERROR;
}

#if  ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5) || (SVN_VER_MAJOR > 2))
svn_error_t *ContextData::onLogMsg3 (const char **log_msg,
              const char **tmp_file,
              const apr_array_header_t * commit_items,
              void *baton,
              apr_pool_t * pool)
{
    ContextData * data = 0;
    SVN_ERR (getContextData (baton, &data));

    QString msg;
    if (data->logIsSet) {
        msg = data->getLogMessage ();
    } else {
        CommitItemList _items;
        for (int j = 0; j < commit_items->nelts; ++j) {
            svn_client_commit_item3_t*item = ((svn_client_commit_item3_t **)commit_items->elts)[j];
            _items.push_back(CommitItem(item));
        }

        if (!data->retrieveLogMessage (msg,_items)) {
            return data->generate_cancel_error();
        }
    }

    *log_msg = apr_pstrdup (pool,msg.TOUTF8());
    *tmp_file = NULL;
    return SVN_NO_ERROR;
}
#endif

void ContextData::onNotify (void * baton,
              const char *path,
              svn_wc_notify_action_t action,
              svn_node_kind_t kind,
              const char *mime_type,
              svn_wc_notify_state_t content_state,
              svn_wc_notify_state_t prop_state,
              svn_revnum_t revision)
{
    if (baton == 0)
        return;
    ContextData * data = static_cast <ContextData *> (baton);
    data->notify (path, action, kind, mime_type, content_state,
                    prop_state, revision);
}

void ContextData::onNotify2(void*baton,const svn_wc_notify_t *action,apr_pool_t */*tpool*/)
{
    if (!baton) return;
    ContextData * data = static_cast <ContextData *> (baton);
    data->notify (action);
}

svn_error_t * ContextData::onCancel (void * baton)
{
    if (baton == 0) return SVN_NO_ERROR;
    ContextData * data = static_cast <ContextData *> (baton);
    if( data->cancel () )
        return data->generate_cancel_error();
    else
        return SVN_NO_ERROR;
}

svn_error_t *ContextData::onCachedPrompt(svn_auth_cred_simple_t **cred,
                                            void *baton,
                                            const char *realm,
                                            const char *username,
                                            svn_boolean_t _may_save,
                                            apr_pool_t *pool)
{
    ContextData * data = 0;
    SVN_ERR (getContextData (baton, &data));
    bool may_save = _may_save != 0;
    if (!data->retrieveCachedLogin (username, realm, may_save ))
        return SVN_NO_ERROR;
    svn_auth_cred_simple_t* lcred = (svn_auth_cred_simple_t*)
            apr_palloc (pool, sizeof (svn_auth_cred_simple_t));
    QByteArray l;
    l = data->getPassword().TOUTF8();
    lcred->password = apr_pstrndup (pool,l,l.size());
    l = data->getUsername().TOUTF8();
    lcred->username = apr_pstrndup (pool,l,l.size());

      // tell svn if the credentials need to be saved
    lcred->may_save = may_save;
    *cred = lcred;

    return SVN_NO_ERROR;
}

svn_error_t *ContextData::onSavedPrompt(svn_auth_cred_simple_t **cred,
                                            void *baton,
                                            const char *realm,
                                            const char *username,
                                            svn_boolean_t _may_save,
                                            apr_pool_t *pool)
{
    ContextData * data = 0;
    SVN_ERR (getContextData (baton, &data));
    bool may_save = _may_save != 0;
    if (!data->retrieveSavedLogin (username, realm, may_save ))
        return SVN_NO_ERROR;
    svn_auth_cred_simple_t* lcred = (svn_auth_cred_simple_t*)
            apr_palloc (pool, sizeof (svn_auth_cred_simple_t));
    QByteArray l;
    l = data->getPassword().TOUTF8();
    lcred->password = apr_pstrndup (pool,l,l.size());
    l = data->getUsername().TOUTF8();
    lcred->username = apr_pstrndup (pool,l,l.size());

      // tell svn if the credentials need to be saved
    lcred->may_save = may_save;
    *cred = lcred;

    return SVN_NO_ERROR;
}

svn_error_t *ContextData::onSimplePrompt (svn_auth_cred_simple_t **cred,
                    void *baton,
                    const char *realm,
                    const char *username,
                    svn_boolean_t _may_save,
                    apr_pool_t *pool)
{
      ContextData * data = 0;
      SVN_ERR (getContextData (baton, &data));
      bool may_save = _may_save != 0;
      if (!data->retrieveLogin (username, realm, may_save ))
        return data->generate_cancel_error();

      svn_auth_cred_simple_t* lcred = (svn_auth_cred_simple_t*)
        apr_palloc (pool, sizeof (svn_auth_cred_simple_t));
      QByteArray l;
      l = data->getPassword().TOUTF8();
      lcred->password = apr_pstrndup (pool,l,l.size());
      l = data->getUsername().TOUTF8();
      lcred->username = apr_pstrndup (pool,l,l.size());

      // tell svn if the credentials need to be saved
      lcred->may_save = may_save;
      *cred = lcred;

      return SVN_NO_ERROR;
}

svn_error_t * ContextData::onSslServerTrustPrompt (svn_auth_cred_ssl_server_trust_t **cred,
                            void *baton,
                            const char *realm,
                            apr_uint32_t failures,
                            const svn_auth_ssl_server_cert_info_t *info,
                            svn_boolean_t may_save,
                            apr_pool_t *pool)
{
      ContextData * data = 0;
      SVN_ERR (getContextData (baton, &data));

      ContextListener::SslServerTrustData trustData (failures);
      if (realm != NULL)
        trustData.realm = realm;
      trustData.hostname = info->hostname;
      trustData.fingerprint = info->fingerprint;
      trustData.validFrom = info->valid_from;
      trustData.validUntil = info->valid_until;
      trustData.issuerDName = info->issuer_dname;
      trustData.maySave = may_save != 0;

      apr_uint32_t acceptedFailures = failures;
      ContextListener::SslServerTrustAnswer answer =
        data->listener->contextSslServerTrustPrompt (
          trustData, acceptedFailures );

      if(answer == ContextListener::DONT_ACCEPT) {
        *cred = 0L;
      } else
      {
        svn_auth_cred_ssl_server_trust_t *cred_ =
          (svn_auth_cred_ssl_server_trust_t*)
          apr_palloc (pool, sizeof (svn_auth_cred_ssl_server_trust_t));

        cred_->accepted_failures = failures;
        if (answer == ContextListener::ACCEPT_PERMANENTLY)
        {
          cred_->may_save = true;
        } else {
            cred_->may_save = false;
        }
        *cred = cred_;
      }

      return SVN_NO_ERROR;
}

svn_error_t * ContextData::onSslClientCertPrompt (svn_auth_cred_ssl_client_cert_t **cred,
                           void *baton,
                           apr_pool_t *pool)
{
    ContextData * data = 0;
    SVN_ERR (getContextData (baton, &data));

    QString certFile;
    if (!data->listener->contextSslClientCertPrompt (certFile))
        return data->generate_cancel_error();

    svn_auth_cred_ssl_client_cert_t *cred_ =
        (svn_auth_cred_ssl_client_cert_t*)
        apr_palloc (pool, sizeof (svn_auth_cred_ssl_client_cert_t));

    cred_->cert_file = certFile.TOUTF8();

    *cred = cred_;
    return SVN_NO_ERROR;
}

svn_error_t *ContextData::onFirstSslClientCertPw (
        svn_auth_cred_ssl_client_cert_pw_t **cred,
        void *baton,
        const char *realm,
        svn_boolean_t maySave,
        apr_pool_t *pool)
{
    ContextData * data = 0;
    SVN_ERR (getContextData (baton, &data));

    QString password;
    bool may_save = maySave != 0;
    if (!data->listener->contextLoadSslClientCertPw(password, QString::FROMUTF8(realm)))
        return SVN_NO_ERROR;

    svn_auth_cred_ssl_client_cert_pw_t *cred_ =
            (svn_auth_cred_ssl_client_cert_pw_t *)
            apr_palloc (pool, sizeof (svn_auth_cred_ssl_client_cert_pw_t));

    cred_->password = password.TOUTF8();
    cred_->may_save = may_save;
    *cred = cred_;

    return SVN_NO_ERROR;
}

svn_error_t * ContextData::onSslClientCertPwPrompt(
        svn_auth_cred_ssl_client_cert_pw_t **cred,
        void *baton,
        const char *realm,
        svn_boolean_t maySave,
        apr_pool_t *pool)
{
    ContextData * data = 0;
    SVN_ERR (getContextData (baton, &data));

    QString password;
    bool may_save = maySave != 0;
    if (!data->listener->contextSslClientCertPwPrompt (password, QString::FROMUTF8(realm), may_save))
        return data->generate_cancel_error();

    svn_auth_cred_ssl_client_cert_pw_t *cred_ =
        (svn_auth_cred_ssl_client_cert_pw_t *)
        apr_palloc (pool, sizeof (svn_auth_cred_ssl_client_cert_pw_t));

    cred_->password = password.TOUTF8();
    cred_->may_save = may_save;
    *cred = cred_;

    return SVN_NO_ERROR;
}

void ContextData::setListener(ContextListener * _listener)
{
    listener = _listener;
}

ContextListener * ContextData::getListener() const
{
    return listener;
}

void ContextData::reset()
{
    m_promptCounter = 0;
    logIsSet = false;
}

svn_error_t * ContextData::generate_cancel_error()
{
    return svn_error_create (SVN_ERR_CANCELLED, 0, QObject::tr("Cancelled by user.").TOUTF8());
}

void ContextData::onProgress(apr_off_t progress, apr_off_t total, void*baton, apr_pool_t*)
{
    ContextData * data = 0;
    if (getContextData (baton, &data)!=SVN_NO_ERROR)
    {
        return;
    }
    data->getListener()->contextProgress(progress,total);
}

#if  ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5) || (SVN_VER_MAJOR > 2))
void ContextData::initMimeTypes()
{
    // code take from subversion 1.5 commandline client
    const char *mimetypes_file;
    svn_error_t * err = 0L;
    svn_config_t * cfg = (svn_config_t *)apr_hash_get(m_ctx->config, SVN_CONFIG_CATEGORY_CONFIG,
                       APR_HASH_KEY_STRING);

    svn_config_get(cfg, &mimetypes_file,
                   SVN_CONFIG_SECTION_MISCELLANY,
                   SVN_CONFIG_OPTION_MIMETYPES_FILE, 0);
    if (mimetypes_file && *mimetypes_file) {
        if ((err = svn_io_parse_mimetypes_file(&(m_ctx->mimetypes_map),
             mimetypes_file, pool))) {
                 svn_handle_error2(err, stderr, false, "svn: ");
        }
    }
}
#endif

svn_error_t* ContextData::onWcConflictResolver(svn_wc_conflict_result_t**result,const svn_wc_conflict_description_t *description, void *baton, apr_pool_t *pool)
{
#if  ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5) || (SVN_VER_MAJOR > 2))
    ContextData * data = 0;
    SVN_ERR (getContextData (baton, &data));
    ConflictResult cresult;
    if (!data->getListener()->contextConflictResolve(cresult,description)) {
        return data->generate_cancel_error();
    }
    cresult.assignResult(result,pool);
    return SVN_NO_ERROR;
#else
    Q_UNUSED(result);
    Q_UNUSED(description);
    Q_UNUSED(baton);
    Q_UNUSED(pool);
    return svn_error_create (SVN_ERR_CANCELLED, NULL,tr("invalid subversion version."));
#endif
}

svn_error_t* ContextData::maySavePlaintext(svn_boolean_t *may_save_plaintext, const char *realmstring, void *baton, apr_pool_t *pool)
{
    Q_UNUSED(pool);
#if  ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 6) || (SVN_VER_MAJOR > 2))
    ContextData * data = 0;
    SVN_ERR(getContextData (baton, &data));
    data->getListener()->maySavePlaintext(may_save_plaintext,QString::FROMUTF8(realmstring));
    return SVN_NO_ERROR;
#else
    Q_UNUSED(may_save_plaintext);
    Q_UNUSED(realmstring);
    Q_UNUSED(baton);
    return svn_error_create (SVN_ERR_CANCELLED, NULL,QObject::tr("invalid subversion version.").TOUTF8());
#endif
}

bool ContextData::contextAddListItem(DirEntries*entries, const svn_dirent_t*dirent,const svn_lock_t*lock,const QString&path)
{
    if (!getListener()) {
        if (!entries || !dirent) {
            return false;
        }
        entries->push_back(new DirEntry(/*QString::FROMUTF8(path)*/path, dirent, lock));
        return true;
    }
    return getListener()->contextAddListItem(entries,dirent,lock,path);
}

bool ContextListener::contextConflictResolve(ConflictResult&result,const ConflictDescription&description)
{
    Q_UNUSED(description);
#if  ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5) || (SVN_VER_MAJOR > 2))
    result.setChoice(ConflictResult::ChoosePostpone);
#else
    Q_UNUSED(result);
#endif
    return true;
}

bool ContextListener::contextAddListItem(DirEntries*entries, const svn_dirent_t*dirent,const svn_lock_t*lock,const QString&path)
{
    if (!entries || !dirent) {
        return false;
    }
    entries->push_back(new DirEntry(/*QString::FROMUTF8(path)*/path,dirent,lock));
    return true;
}

void ContextListener::maySavePlaintext(svn_boolean_t *may_save_plaintext, const QString&realmstring)
{
    Q_UNUSED(realmstring);
    if (may_save_plaintext) {
        *may_save_plaintext = true;
    }
}

}
