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
#ifndef HELPERSCACHEENTRY_H
#define HELPERSCACHEENTRY_H

#include "src/svnqt/client.hpp"
#include "src/svnqt/status.hpp"

// std::map 'cause QMap isn't usable
#include <map>
#include <algorithm>
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
    template<class T> void listsubs_if(QStringList&_what,T&oper)const;

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

template<class T> inline void cacheEntry::listsubs_if(QStringList&what,T&oper)const
{
    if (what.count()==0) {
        /* we are the one to get the list for*/
        oper = for_each(m_subMap.begin(),m_subMap.end(),oper);
        return;
    }
    /* otherwise find next */
    std::map<QString,cacheEntry>::const_iterator it=m_subMap.find(what[0]);
    if (it==m_subMap.end()) {
        /* not found */
        return;
    }
    what.erase(what.begin());
    it->second.listsubs_if(what,oper);
}

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

    template<class T>void listsubs_if(const QString&what,T&oper)const;

    void dump_tree();
};

template<class T> inline void itemCache::listsubs_if(const QString&_what,T&oper)const
{
    if (m_contentMap.size()==0) {
        return;
    }
    QStringList what = QStringList::split("/",_what);
    if (what.count()==0) {
        return;
    }
    std::map<QString,cacheEntry>::const_iterator it=m_contentMap.find(what[0]);
    if (it==m_contentMap.end()) {
        return;
    }
    if (what.count()==1) {
        oper = for_each(m_contentMap.begin(),m_contentMap.end(),oper);
        return;
    }
    what.erase(what.begin());
    it->second.listsubs_if(what,oper);
}

class ValidRemoteOnly
{
    svn::StatusEntries m_List;
public:
    ValidRemoteOnly():m_List(){}
    void operator()(const std::pair<const QString,helpers::cacheEntry>&_data) {
        if(_data.second.content().validReposStatus()&&!_data.second.content().validLocalStatus()) {
            m_List.push_back(_data.second.content());
        }
    }
    const svn::StatusEntries&liste()const{return m_List;}
};

}

#endif
