/*
 * ====================================================================
 * Copyright (c) 2002-2005 The RapidSvn Group.  All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library (in the file LGPL.txt); if not,
 * write to the Free Software Foundation, Inc., 51 Franklin St,
 * Fifth Floor, Boston, MA  02110-1301  USA
 *
 * This software consists of voluntary contributions made by many
 * individuals.  For exact contribution history, see the revision
 * history and logs, available at http://rapidsvn.tigris.org/.
 * ====================================================================
 */

#ifndef _SVNCPP_CONTEXT_LISTENER_HPP_
#define _SVNCPP_CONTEXT_LISTENER_HPP_

// svncpp
#include "svnqt/pool.hpp"
#include "svnqt/commititem.hpp"
#include "svnqt/svnqt_defines.hpp"
// qt
#include <qstring.h>
// Subversion api
#include <svn_client.h>


namespace svn
{
  /**
   * This is the interface that is used by @a Context
   * for callbacks.
   * To use this you will have to inherit from this
   * interface and overwrite the virtual methods.
   */
  class SVNQT_EXPORT ContextListener
  {
  public:
    /**
     * empty destructor avoids a lot of compiler warnings
     */
    virtual ~ContextListener(){}
    /**
     * this method will be called to retrieve
     * authentication information
     *
     * WORKAROUND FOR apr_xlate PROBLEM:
     * STRINGS ALREADY HAVE TO BE UTF8!!!
     *
     * @param username
     * @param realm in which username/password will be used
     * @param password
     * @param maySave in/out set false to not save
     * @return continue action?
     * @retval true continue
     */
    virtual bool
    contextGetLogin (const QString & realm,
                     QString & username,
                     QString & password,
                     bool & maySave) = 0;
    /**
     * this method will be called to retrieve
     * authentication information stored not by subversion. This
     * will only called once!
     *
     * @param username
     * @param realm in which username/password will be used
     * @param password
     * @return continue action?
     * @retval true continue
     */
    virtual bool
    contextGetSavedLogin(const QString & realm,
                         QString & username,
                         QString & password) = 0;
    /**
     * this method will be called to notify about
     * the progress of an ongoing action
     *
     * @param path
     * @param action
     * @param kind
     * @param mime_type
     * @param content_state
     * @param prop_state
     * @param revision
     */
    virtual void
    contextNotify (const char *path,
                   svn_wc_notify_action_t action,
                   svn_node_kind_t kind,
                   const char *mime_type,
                   svn_wc_notify_state_t content_state,
                   svn_wc_notify_state_t prop_state,
                   svn_revnum_t revision) = 0;
    /**
     * this method will be called to notify about
     * the progress of an ongoing action
     *
     * @param action the action got notified about
     * @since subversion 1.2
     */
    virtual void
    contextNotify (const svn_wc_notify_t *action) = 0;
    /**
     * this method will be called periodically to allow
     * the app to cancel long running operations
     *
     * @return cancel action?
     * @retval true cancel
     */
    virtual bool
    contextCancel() = 0;

    /**
     * this method will be called to retrieve
     * a log message
     *
     * WORKAROUND FOR apr_xlate PROBLEM:
     * STRINGS ALREADY HAVE TO BE UTF8!!!
     *
     * @param msg log message
     * @return continue action?
     * @retval true continue
     */
    virtual bool
    contextGetLogMessage (QString & msg,const CommitItemList&) = 0;

    typedef enum
    {
      DONT_ACCEPT = 0,
      ACCEPT_TEMPORARILY,
      ACCEPT_PERMANENTLY
    } SslServerTrustAnswer;


    /**
     * @see contextSslServerTrust
     * @see svn_auth_cred_ssl_server_trust_t
     */
    struct SslServerTrustData
    {
    public:
      /** bit coded failures */
      const apr_uint32_t failures;

      /** certificate information */
      QString hostname;
      QString fingerprint;
      QString validFrom;
      QString validUntil;
      QString issuerDName;
      QString realm;
      bool maySave;

      SslServerTrustData (const apr_uint32_t failures_)
        : failures (failures_), hostname (""), fingerprint (""),
          validFrom (""), validUntil (""), issuerDName (""),
          realm (""), maySave (true)
      {
      }
    };


    /**
     * this method is called if there is ssl server
     * information, that has to be confirmed by the user
     *
     * @param data
     * @param acceptedFailures
     * @return @a SslServerTrustAnswer
     */
    virtual SslServerTrustAnswer
    contextSslServerTrustPrompt (const SslServerTrustData & data,
                                 apr_uint32_t & acceptedFailures) = 0;

    /**
     * this method is called to retrieve client side
     * information
     */
    virtual bool
    contextSslClientCertPrompt (QString & certFile) = 0;

    /**
     * this method is called to retrieve the password
     * for the client certificate
     *
     * @param password
     * @param realm
     * @param maySave
     */
    virtual bool
    contextSslClientCertPwPrompt (QString & password,
                                  const QString & realm,
                                  bool & maySave) = 0;
    /**
     * this method is called to retrieve the password
     * for the client certificate from a local storage or such. it will called only once.
     *
     * @param password
     * @param realm
     */
    virtual bool
    contextLoadSslClientCertPw(QString&password,const QString&realm)=0;

    virtual void
    contextProgress(long long int current, long long int max) = 0;

    /**
     * try to translate a text. In current implementation does
     * nothing than returning the origin but may used to get an
     * application specific translation.
     * @param what text to translate
     * @return translated text or origin.
     */
    virtual QString translate(const QString&what){return what;}
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
