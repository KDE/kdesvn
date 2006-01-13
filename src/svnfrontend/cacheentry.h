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

#include <qmap.h>
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
    virtual bool findSingleValid(QStringList&what,svn::Status&)const;
    virtual void appendValidSub(svn::StatusEntries&)const; 
    virtual bool isValid()const;
    virtual const svn::Status&content()const;

    virtual void deleteKey(QStringList&,bool exact);
    virtual void insertKey(QStringList&,const svn::Status&);
    virtual void setValidStatus(const QString&key,const svn::Status&);
    virtual bool hasValidSubs();
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

    void dump_tree();
};

}

#endif
