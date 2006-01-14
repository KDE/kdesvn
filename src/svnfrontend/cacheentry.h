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
#ifndef HELPERSCACHEENTRY_H
#define HELPERSCACHEENTRY_H

#include "svncpp/client.hpp"
#include "svncpp/status.hpp"

// std::map 'cause QMap isn't usable
#include <map>
#include <qstring.h>
#include <qstringlist.h>

namespace helpers {

/**
Class for fast search of modified or updated items.

	@author Rajko Albrecht <ral@alwins-world.de>
*/
class cacheEntry{

protected:
    QString m_key;
    bool m_isValid;
    svn::Status m_content;
    std::map<QString,cacheEntry> m_subMap;

public:
    cacheEntry();
    cacheEntry(const QString&key);
    cacheEntry(const cacheEntry&other);

    virtual ~cacheEntry();

    virtual bool find(QStringList&,svn::StatusEntries&)const;
    //! Checks if cache contains a specific item
    /*!
     * the keylist will manipulated - so copy-operations aren't needed.
     * \param what Stringlist containing the components to search for
     * \return true if found (may or may not valid!) otherwise false
     */
    virtual bool find(QStringList&what)const;
    //! Checks if cache contains a specific valid item
    /*!
     * if yes, the content will stored in st
     * \param what the keylist to search for
     * \param st target status to store content if found
     * \return true if found
     */
    virtual bool findSingleValid(QStringList&what,svn::Status&st)const;
    //! Checks if cache contains a specific valid item
    /*!
     * in difference to virtual bool find(QStringList&,svn::StatusEntries&)const no copy operations
     * are made inside so it works much faster for simple find.
     * \param what the keylist to search for
     * \param check_valid_subs if true, return true if a subitem is valid even the item isn't valid
     * \return true if found
     */
    virtual bool findSingleValid(QStringList&what,bool check_valid_subs)const;

    virtual void appendValidSub(svn::StatusEntries&)const;
    virtual bool isValid()const;
    virtual const svn::Status&content()const;

    virtual bool deleteKey(QStringList&,bool exact);
    virtual void insertKey(QStringList&,const svn::Status&);
    virtual void setValidStatus(const QString&key,const svn::Status&);
    virtual bool hasValidSubs()const;
    virtual void markInvalid();
    const QString&key()const;

    cacheEntry& operator=(const cacheEntry&other);
    void dump_tree(int level=0)const;
};

class itemCache
{
protected:
    std::map<QString,cacheEntry> m_contentMap;

public:
    itemCache();
    virtual ~itemCache();

    void setContent(const svn::StatusEntries&dlist);
    void clear();
    //! Checks if cache contains a specific item
    /*!
     * the keylist will manipulated - so copy-operations aren't needed.
     * \param what Stringlist containing the components to search for
     * \return true if found (may or may not valid!) otherwise false
     */
    virtual bool find(const QString&what)const;
    virtual bool find(const QString&,svn::StatusEntries&)const;

    virtual void deleteKey(const QString&what,bool exact);
    virtual void insertKey(const svn::Status&);
    virtual bool findSingleValid(const QString&what,svn::Status&)const;
    virtual bool findSingleValid(const QString&what,bool check_valid_subs)const;

    void dump_tree();
};

}

#endif
