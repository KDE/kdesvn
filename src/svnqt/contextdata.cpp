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
#include <QCoreApplication>

namespace svn
{

static inline char *toAprCharPtr(const QString &str, apr_pool_t *pool)
{
  const QByteArray l = str.toUtf8();
  return apr_pstrndup(pool, l.data(), l.size());
}

ContextData::ContextData(const QString &configDir_)
    : listener(nullptr), logIsSet(false),
      m_promptCounter(0), m_ConfigDir(configDir_)
{
    const QByteArray cfgDirBa = m_ConfigDir.toUtf8();
    const char *c_configDir = cfgDirBa.isEmpty() ? nullptr : cfgDirBa.constData();

    // make sure the configuration directory exists
    svn_config_ensure(c_configDir, pool);

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
#if defined(WIN32)  //krazy:exclude=cpp
        apr_array_make(pool, 12, sizeof(svn_auth_provider_object_t *));
#else
        apr_array_make(pool, 11, sizeof(svn_auth_provider_object_t *));
#endif
    svn_auth_provider_object_t *provider;

#if defined(WIN32)  //krazy:exclude=cpp
    svn_auth_get_windows_simple_provider(&provider, pool);
    APR_ARRAY_PUSH(providers, svn_auth_provider_object_t *) = provider;
#endif

    svn_auth_get_simple_provider2
    (&provider, maySavePlaintext, this, pool);
    *(svn_auth_provider_object_t **)apr_array_push(providers) = provider;

    svn_auth_get_username_provider(&provider, pool);
    *(svn_auth_provider_object_t **)apr_array_push(providers) = provider;

    svn_auth_get_simple_prompt_provider(&provider, onCachedPrompt, this, 0, pool);
    *(svn_auth_provider_object_t **)apr_array_push(providers) = provider;

    svn_auth_get_simple_prompt_provider(&provider, onSavedPrompt, this, 0, pool);
    *(svn_auth_provider_object_t **)apr_array_push(providers) = provider;

    /* not very nice. should be infinite... */
    svn_auth_get_simple_prompt_provider(&provider, onSimplePrompt, this, 100000000, pool);
    *(svn_auth_provider_object_t **)apr_array_push(providers) = provider;

    // add ssl providers

    // file first then prompt providers
    svn_auth_get_ssl_server_trust_file_provider(&provider, pool);
    *(svn_auth_provider_object_t **)apr_array_push(providers) = provider;

    svn_auth_get_ssl_client_cert_file_provider(&provider, pool);
    *(svn_auth_provider_object_t **)apr_array_push(providers) = provider;

    svn_auth_get_ssl_client_cert_pw_file_provider2(&provider, maySavePlaintext, this, pool);
    *(svn_auth_provider_object_t **)apr_array_push(providers) = provider;

    svn_auth_get_ssl_server_trust_prompt_provider(&provider, onSslServerTrustPrompt, this, pool);
    *(svn_auth_provider_object_t **)apr_array_push(providers) = provider;

    // first try load from extra storage
    svn_auth_get_ssl_client_cert_pw_prompt_provider(&provider, onFirstSslClientCertPw, this, 0, pool);
    *(svn_auth_provider_object_t **)apr_array_push(providers) = provider;

    // plugged in 3 as the retry limit - what is a good limit?
    svn_auth_get_ssl_client_cert_pw_prompt_provider(&provider, onSslClientCertPwPrompt, this, 3, pool);
    *(svn_auth_provider_object_t **)apr_array_push(providers) = provider;

    svn_auth_baton_t *ab;
    svn_auth_open(&ab, providers, pool);

    // todo svn 1.8: svn_client_create_context2
    // initialize ctx structure
    svn_client_create_context(&m_ctx, pool);

    // get the config based on the configDir passed in
    svn_config_get_config(&(m_ctx->config), c_configDir, pool);

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

    m_ctx->log_msg_func3 = onLogMsg3;
    m_ctx->log_msg_baton3 = this;

    m_ctx->conflict_func = onWcConflictResolver;
    m_ctx->conflict_baton = this;

    m_ctx->conflict_func2 = onWcConflictResolver2;
    m_ctx->conflict_baton2 = this;

    m_ctx->client_name = "SvnQt wrapper client";
    initMimeTypes();
}


ContextData::~ContextData()
{
}


const QString &ContextData::getLogMessage() const
{
    return logMessage;
}

bool ContextData::retrieveLogMessage(QString &msg, const CommitItemList &_itemlist)
{
    bool ok = false;
    if (listener) {
        ok = listener->contextGetLogMessage(logMessage, _itemlist);
        if (ok) {
            msg = logMessage;
        } else {
            logIsSet = false;
        }
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
    if (listener != nullptr) {
        listener->contextNotify(path, action, kind, mime_type,
                                content_state, prop_state, revision);
    }
}

void ContextData::notify(const svn_wc_notify_t *action)
{
    if (listener != nullptr) {
        listener->contextNotify(action);
    }
}

bool ContextData::cancel()
{
    if (listener != nullptr) {
        return listener->contextCancel();
    } else {
        // don't cancel if no listener
        return false;
    }
}
const QString &ContextData::getUsername() const
{
    return username;
}

const QString &ContextData::getPassword() const
{
    return password;
}

bool ContextData::retrieveLogin(const char *username_,
                                const char *realm,
                                bool &may_save)
{
    bool ok;

    if (listener == nullptr) {
        return false;
    }

    username = QString::fromUtf8(username_);
    ok = listener->contextGetLogin(QString::fromUtf8(realm), username, password, may_save);

    return ok;
}

bool ContextData::retrieveSavedLogin(const char *username_,
                                     const char *realm,
                                     bool &may_save)
{
    bool ok;
    may_save = false;

    if (listener == nullptr) {
        return false;
    }

    username = QString::fromUtf8(username_);
    ok = listener->contextGetSavedLogin(QString::fromUtf8(realm), username, password);
    return ok;
}

bool ContextData::retrieveCachedLogin(const char *username_,
                                      const char *realm,
                                      bool &may_save)
{
    bool ok;
    may_save = false;

    if (listener == nullptr) {
        return false;
    }

    username = QString::fromUtf8(username_);
    ok = listener->contextGetCachedLogin(QString::fromUtf8(realm), username, password);
    return ok;
}

svn_client_ctx_t *ContextData::ctx()
{
    return m_ctx;
}

const QString &ContextData::configDir()const
{
    return m_ConfigDir;
}

svn_error_t *
ContextData::getContextData(void *baton, ContextData **data)
{
    if (baton == nullptr)
        return svn_error_create(SVN_ERR_CANCELLED, nullptr,
                                QCoreApplication::translate("svnqt", "invalid baton").toUtf8());

    ContextData *data_ = static_cast <ContextData *>(baton);

    if (data_->listener == nullptr)
        return svn_error_create(SVN_ERR_CANCELLED, nullptr,
                                QCoreApplication::translate("svnqt", "invalid listener").toUtf8());

    *data = data_;
    return SVN_NO_ERROR;
}

void ContextData::setAuthCache(bool value)
{
    void *param = nullptr;
    if (!value) {
        param = (void *)"1";
    }
    svn_auth_set_parameter(m_ctx->auth_baton,
                           SVN_AUTH_PARAM_NO_AUTH_CACHE, param);
}

void ContextData::setLogin(const QString &usr, const QString &pwd)
{
    username = usr;
    password = pwd;
    svn_auth_baton_t *ab = m_ctx->auth_baton;
    svn_auth_set_parameter(ab, SVN_AUTH_PARAM_DEFAULT_USERNAME, username.toUtf8());
    svn_auth_set_parameter(ab, SVN_AUTH_PARAM_DEFAULT_PASSWORD, password.toUtf8());
}

void ContextData::setLogMessage(const QString &msg)
{
    logMessage = msg;
    if (msg.isNull()) {
        logIsSet = false;
    } else {
        logIsSet = true;
    }
}

svn_error_t *ContextData::onLogMsg(const char **log_msg,
                                   const char **tmp_file,
                                   apr_array_header_t *commit_items,
                                   void *baton,
                                   apr_pool_t *pool)
{
    ContextData *data = nullptr;
    SVN_ERR(getContextData(baton, &data));

    QString msg;
    if (data->logIsSet) {
        msg = data->getLogMessage();
    } else {
        CommitItemList _items;
        _items.reserve(commit_items->nelts);
        for (int j = 0; j < commit_items->nelts; ++j) {
            svn_client_commit_item_t *item = ((svn_client_commit_item_t **)commit_items->elts)[j];
            _items.push_back(CommitItem(item));
        }
        if (!data->retrieveLogMessage(msg, _items)) {
            return data->generate_cancel_error();
        }
    }

    *log_msg = toAprCharPtr(msg, pool);
    *tmp_file = nullptr;
    return SVN_NO_ERROR;
}

svn_error_t *ContextData::onLogMsg2(const char **log_msg,
                                    const char **tmp_file,
                                    const apr_array_header_t *commit_items,
                                    void *baton,
                                    apr_pool_t *pool)
{
    ContextData *data = nullptr;
    SVN_ERR(getContextData(baton, &data));

    QString msg;
    if (data->logIsSet) {
        msg = data->getLogMessage();
    } else {
        CommitItemList _items;
        _items.reserve(commit_items->nelts);
        for (int j = 0; j < commit_items->nelts; ++j) {
            svn_client_commit_item2_t *item = ((svn_client_commit_item2_t **)commit_items->elts)[j];
            _items.push_back(CommitItem(item));
        }

        if (!data->retrieveLogMessage(msg, _items)) {
            return data->generate_cancel_error();
        }
    }

    *log_msg = toAprCharPtr(msg, pool);
    *tmp_file = nullptr;
    return SVN_NO_ERROR;
}

svn_error_t *ContextData::onLogMsg3(const char **log_msg,
                                    const char **tmp_file,
                                    const apr_array_header_t *commit_items,
                                    void *baton,
                                    apr_pool_t *pool)
{
    ContextData *data = nullptr;
    SVN_ERR(getContextData(baton, &data));

    QString msg;
    if (data->logIsSet) {
        msg = data->getLogMessage();
    } else {
        CommitItemList _items;
        _items.reserve(commit_items->nelts);
        for (int j = 0; j < commit_items->nelts; ++j) {
            svn_client_commit_item3_t *item = ((svn_client_commit_item3_t **)commit_items->elts)[j];
            _items.push_back(CommitItem(item));
        }

        if (!data->retrieveLogMessage(msg, _items)) {
            return data->generate_cancel_error();
        }
    }

    *log_msg = toAprCharPtr(msg, pool);
    *tmp_file = nullptr;
    return SVN_NO_ERROR;
}

void ContextData::onNotify(void *baton,
                           const char *path,
                           svn_wc_notify_action_t action,
                           svn_node_kind_t kind,
                           const char *mime_type,
                           svn_wc_notify_state_t content_state,
                           svn_wc_notify_state_t prop_state,
                           svn_revnum_t revision)
{
    if (baton == nullptr) {
        return;
    }
    ContextData *data = static_cast <ContextData *>(baton);
    data->notify(path, action, kind, mime_type, content_state,
                 prop_state, revision);
}

void ContextData::onNotify2(void *baton, const svn_wc_notify_t *action, apr_pool_t */*tpool*/)
{
    if (!baton) {
        return;
    }
    ContextData *data = static_cast <ContextData *>(baton);
    data->notify(action);
}

svn_error_t *ContextData::onCancel(void *baton)
{
    if (baton == nullptr) {
        return SVN_NO_ERROR;
    }
    ContextData *data = static_cast <ContextData *>(baton);
    if (data->cancel()) {
        return data->generate_cancel_error();
    } else {
        return SVN_NO_ERROR;
    }
}

svn_error_t *ContextData::onCachedPrompt(svn_auth_cred_simple_t **cred,
                                         void *baton,
                                         const char *realm,
                                         const char *username,
                                         svn_boolean_t _may_save,
                                         apr_pool_t *pool)
{
    ContextData *data = nullptr;
    SVN_ERR(getContextData(baton, &data));
    bool may_save = _may_save != 0;
    if (!data->retrieveCachedLogin(username, realm, may_save)) {
        return SVN_NO_ERROR;
    }
    svn_auth_cred_simple_t *lcred = (svn_auth_cred_simple_t *)
                                    apr_palloc(pool, sizeof(svn_auth_cred_simple_t));
    lcred->password = toAprCharPtr(data->getPassword(), pool);
    lcred->username = toAprCharPtr(data->getUsername(), pool);

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
    ContextData *data = nullptr;
    SVN_ERR(getContextData(baton, &data));
    bool may_save = _may_save != 0;
    if (!data->retrieveSavedLogin(username, realm, may_save)) {
        return SVN_NO_ERROR;
    }
    svn_auth_cred_simple_t *lcred = (svn_auth_cred_simple_t *)
                                    apr_palloc(pool, sizeof(svn_auth_cred_simple_t));
    lcred->password = toAprCharPtr(data->getPassword(), pool);
    lcred->username = toAprCharPtr(data->getUsername(), pool);

    // tell svn if the credentials need to be saved
    lcred->may_save = may_save;
    *cred = lcred;

    return SVN_NO_ERROR;
}

svn_error_t *ContextData::onSimplePrompt(svn_auth_cred_simple_t **cred,
                                         void *baton,
                                         const char *realm,
                                         const char *username,
                                         svn_boolean_t _may_save,
                                         apr_pool_t *pool)
{
    ContextData *data = nullptr;
    SVN_ERR(getContextData(baton, &data));
    bool may_save = _may_save != 0;
    if (!data->retrieveLogin(username, realm, may_save)) {
        return data->generate_cancel_error();
    }

    svn_auth_cred_simple_t *lcred = (svn_auth_cred_simple_t *)
                                    apr_palloc(pool, sizeof(svn_auth_cred_simple_t));
    lcred->password = toAprCharPtr(data->getPassword(), pool);
    lcred->username = toAprCharPtr(data->getUsername(), pool);

    // tell svn if the credentials need to be saved
    lcred->may_save = may_save;
    *cred = lcred;

    return SVN_NO_ERROR;
}

svn_error_t *ContextData::onSslServerTrustPrompt(svn_auth_cred_ssl_server_trust_t **cred,
                                                 void *baton,
                                                 const char *realm,
                                                 apr_uint32_t failures,
                                                 const svn_auth_ssl_server_cert_info_t *info,
                                                 svn_boolean_t may_save,
                                                 apr_pool_t *pool)
{
    ContextData *data = nullptr;
    SVN_ERR(getContextData(baton, &data));

    ContextListener::SslServerTrustData trustData(failures);
    if (realm != nullptr) {
        trustData.realm = QString::fromUtf8(realm);
    }
    trustData.hostname = QString::fromUtf8(info->hostname);
    trustData.fingerprint = QString::fromUtf8(info->fingerprint);
    trustData.validFrom = QString::fromUtf8(info->valid_from);
    trustData.validUntil = QString::fromUtf8(info->valid_until);
    trustData.issuerDName = QString::fromUtf8(info->issuer_dname);
    trustData.maySave = may_save != 0;

    apr_uint32_t acceptedFailures = failures;
    ContextListener::SslServerTrustAnswer answer =
        data->listener->contextSslServerTrustPrompt(
            trustData, acceptedFailures);

    if (answer == ContextListener::DONT_ACCEPT) {
        *cred = nullptr;
    } else {
        svn_auth_cred_ssl_server_trust_t *cred_ =
            (svn_auth_cred_ssl_server_trust_t *)
            apr_palloc(pool, sizeof(svn_auth_cred_ssl_server_trust_t));

        cred_->accepted_failures = failures;
        if (answer == ContextListener::ACCEPT_PERMANENTLY) {
            cred_->may_save = true;
        } else {
            cred_->may_save = false;
        }
        *cred = cred_;
    }

    return SVN_NO_ERROR;
}

svn_error_t *ContextData::onSslClientCertPrompt(svn_auth_cred_ssl_client_cert_t **cred,
                                                void *baton,
                                                apr_pool_t *pool)
{
    ContextData *data = nullptr;
    SVN_ERR(getContextData(baton, &data));

    QString certFile;
    if (!data->listener->contextSslClientCertPrompt(certFile)) {
        return data->generate_cancel_error();
    }

    svn_auth_cred_ssl_client_cert_t *cred_ =
        (svn_auth_cred_ssl_client_cert_t *)
        apr_palloc(pool, sizeof(svn_auth_cred_ssl_client_cert_t));

    cred_->cert_file = toAprCharPtr(certFile, pool);

    *cred = cred_;
    return SVN_NO_ERROR;
}

svn_error_t *ContextData::onFirstSslClientCertPw(
    svn_auth_cred_ssl_client_cert_pw_t **cred,
    void *baton,
    const char *realm,
    svn_boolean_t maySave,
    apr_pool_t *pool)
{
    ContextData *data = nullptr;
    SVN_ERR(getContextData(baton, &data));

    QString password;
    bool may_save = maySave != 0;
    if (!data->listener->contextLoadSslClientCertPw(password, QString::fromUtf8(realm))) {
        return SVN_NO_ERROR;
    }

    svn_auth_cred_ssl_client_cert_pw_t *cred_ =
        (svn_auth_cred_ssl_client_cert_pw_t *)
        apr_palloc(pool, sizeof(svn_auth_cred_ssl_client_cert_pw_t));

    cred_->password = toAprCharPtr(password, pool);
    cred_->may_save = may_save;
    *cred = cred_;

    return SVN_NO_ERROR;
}

svn_error_t *ContextData::onSslClientCertPwPrompt(
    svn_auth_cred_ssl_client_cert_pw_t **cred,
    void *baton,
    const char *realm,
    svn_boolean_t maySave,
    apr_pool_t *pool)
{
    ContextData *data = nullptr;
    SVN_ERR(getContextData(baton, &data));

    QString password;
    bool may_save = maySave != 0;
    if (!data->listener->contextSslClientCertPwPrompt(password, QString::fromUtf8(realm), may_save)) {
        return data->generate_cancel_error();
    }

    svn_auth_cred_ssl_client_cert_pw_t *cred_ =
        (svn_auth_cred_ssl_client_cert_pw_t *)
        apr_palloc(pool, sizeof(svn_auth_cred_ssl_client_cert_pw_t));

    cred_->password = toAprCharPtr(password, pool);
    cred_->may_save = may_save;
    *cred = cred_;

    return SVN_NO_ERROR;
}

void ContextData::setListener(ContextListener *_listener)
{
    listener = _listener;
}

ContextListener *ContextData::getListener() const
{
    return listener;
}

void ContextData::reset()
{
    m_promptCounter = 0;
    logIsSet = false;
}

svn_error_t *ContextData::generate_cancel_error()
{
    return svn_error_create(SVN_ERR_CANCELLED, nullptr, QCoreApplication::translate("svnqt", "Cancelled by user.").toUtf8());
}

void ContextData::onProgress(apr_off_t progress, apr_off_t total, void *baton, apr_pool_t *)
{
    ContextData *data = nullptr;
    if (getContextData(baton, &data) != SVN_NO_ERROR) {
        return;
    }
    data->getListener()->contextProgress(progress, total);
}

void ContextData::initMimeTypes()
{
    // code take from subversion 1.5 commandline client
    const char *mimetypes_file;
    svn_error_t *err = nullptr;
    svn_config_t *cfg = (svn_config_t *)apr_hash_get(m_ctx->config, SVN_CONFIG_CATEGORY_CONFIG,
                                                     APR_HASH_KEY_STRING);

    svn_config_get(cfg, &mimetypes_file,
                   SVN_CONFIG_SECTION_MISCELLANY,
                   SVN_CONFIG_OPTION_MIMETYPES_FILE, nullptr);
    if (mimetypes_file && *mimetypes_file) {
        if ((err = svn_io_parse_mimetypes_file(&(m_ctx->mimetypes_map),
                                               mimetypes_file, pool))) {
            svn_handle_error2(err, stderr, false, "svn: ");
        }
    }
}

svn_error_t *ContextData::onWcConflictResolver(svn_wc_conflict_result_t **result, const svn_wc_conflict_description_t *description, void *baton, apr_pool_t *pool)
{
    ContextData *data = nullptr;
    SVN_ERR(getContextData(baton, &data));
    ConflictResult cresult;
    if (!data->getListener()->contextConflictResolve(cresult, ConflictDescription(description))) {
        return data->generate_cancel_error();
    }
    cresult.assignResult(result, pool);
    return SVN_NO_ERROR;
}

svn_error_t *ContextData::onWcConflictResolver2(svn_wc_conflict_result_t **result,
                                                const svn_wc_conflict_description2_t *description,
                                                void *baton,
                                                apr_pool_t *result_pool,
                                                apr_pool_t *)
{
  ContextData *data = nullptr;
  SVN_ERR(getContextData(baton, &data));
  ConflictResult cresult;
  if (!data->getListener()->contextConflictResolve(cresult, ConflictDescription(description))) {
      return data->generate_cancel_error();
  }
  cresult.assignResult(result, result_pool);
  return SVN_NO_ERROR;
}

svn_error_t *ContextData::maySavePlaintext(svn_boolean_t *may_save_plaintext, const char *realmstring, void *baton, apr_pool_t *pool)
{
    Q_UNUSED(pool);
    ContextData *data = nullptr;
    SVN_ERR(getContextData(baton, &data));
    data->getListener()->maySavePlaintext(may_save_plaintext, QString::fromUtf8(realmstring));
    return SVN_NO_ERROR;
}

bool ContextData::contextAddListItem(DirEntries *entries, const svn_dirent_t *dirent, const svn_lock_t *lock, const QString &path)
{
    if (!getListener()) {
        if (!entries || !dirent) {
            return false;
        }
        entries->push_back(DirEntry(path, dirent, lock));
        return true;
    }
    return getListener()->contextAddListItem(entries, dirent, lock, path);
}

bool ContextListener::contextConflictResolve(ConflictResult &result, const ConflictDescription &description)
{
    Q_UNUSED(description);
    result.setChoice(ConflictResult::ChoosePostpone);
    return true;
}

bool ContextListener::contextAddListItem(DirEntries *entries, const svn_dirent_t *dirent, const svn_lock_t *lock, const QString &path)
{
    if (!entries || !dirent) {
        return false;
    }
    entries->push_back(DirEntry(path, dirent, lock));
    return true;
}

void ContextListener::maySavePlaintext(svn_boolean_t *may_save_plaintext, const QString &realmstring)
{
    Q_UNUSED(realmstring);
    if (may_save_plaintext) {
        *may_save_plaintext = true;
    }
}

}
