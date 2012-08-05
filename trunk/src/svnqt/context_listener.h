/*
 * Port for usage with qt-framework and development for kdesvn
 * Copyright (C) 2005-2009 by Rajko Albrecht (ral@alwins-world.de)
 * http://kdesvn.alwins-world.de
 */
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

#ifndef SVNQT_CONTEXT_LISTENER_H
#define SVNQT_CONTEXT_LISTENER_H

// svncpp
#include "svnqt/pool.h"
#include "svnqt/commititem.h"
#include "svnqt/svnqt_defines.h"
// qt
#include <qstring.h>
// Subversion api
#include <svn_client.h>


namespace svn
{
    class ConflictResult;
    class ConflictDescription;
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
     * authentication information. This will called until valid information were
     * inserted or it returns false.
     *
     * @param username username set as default by subversion
     * @param realm in which username/password will be used
     * @param password target storage for password
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
     * @param username username set as default by subversion
     * @param realm in which username/password will be used
     * @param password  target storage for password
     * @return continue action? should only in case of emergency return false.
     * @retval true continue
     */
    virtual bool
    contextGetSavedLogin(const QString & realm,
                         QString & username,
                         QString & password) = 0;
    /**
     * this method will be called to retrieve
     * authentication information stored not persistent. This
     * will only called once!
     *
     * @param username username set as default by subversion
     * @param realm in which username/password will be used
     * @param password  target storage for password
     * @return continue action? should only in case of emergency return false.
     * @retval true continue
     */
    virtual bool
    contextGetCachedLogin(const QString & realm,
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
    contextNotify(const char *path,
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
    virtual void contextNotify (const svn_wc_notify_t *action)=0;

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

    /** Callback for svn_wc_conflict_resolver_func_t in subversion 1.5
     * This method is only useful when build with subverion 1.5 or above. The default implementation sets
     * result to ConflictResult::ChoosePostpone. Then conflicts while merge, update and switch results in an
     * item with "conflict" status set.
     *
     * @param result The result where to store
     * @param description description of conflict.
     * @return true if result may used and operaion should continue.
     * @sa svn_wc_conflict_description_t, svn_wc_conflict_result_t
     * @since subversion 1.5
     */
    virtual bool contextConflictResolve(ConflictResult&result,const ConflictDescription&description);
    /** Callback for svn_auth_plaintext_prompt_func_t in subversion 1.6
     * @param may_save call back memory where to store true (yes, save plaintext) or false (no save of plaintext passwords)
     * The default implementation set it to "true"
     */
    virtual void maySavePlaintext(svn_boolean_t *may_save_plaintext, const QString&realmstring);
    /** Callback for generating list entries
     * This base implementation just adds items to @a entries. This may used for special listener like the one from KIO
     * where items may displayed direkt on call and not stored into @a entries.
     * @param entries default target list
     * @param dirent entry to add (send by subversion)
     * @param lock accociated lock (may be null!)
     * @param path the path of the item
     * @return true if inserted/displayd, false if dirent or entries aren't valid.
     */
    virtual bool contextAddListItem(DirEntries*entries, const svn_dirent_t*dirent,const svn_lock_t*lock,const QString&path);
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
