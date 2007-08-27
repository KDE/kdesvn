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

#include "src/svnqt/svnqttypes.hpp"
#include "src/svnqt/status.hpp"

// std::map 'cause QMap isn't usable
#include <map>
#include <algorithm>
#include <qstring.h>
#include <qstringlist.h>

namespace helpers {

/**
    Class for fast search of items.

    @author Rajko Albrecht <ral@alwins-world.de>
*/
template<class C> class cacheEntry {
public:
    typedef std::map<QString,cacheEntry<C> > cache_map_type;

protected:
    QString m_key;
    bool m_isValid;
    C m_content;
    cache_map_type m_subMap;
    typedef typename cache_map_type::const_iterator citer;
    typedef typename cache_map_type::iterator iter;

public:
    cacheEntry();
    cacheEntry(const QString&key);
    cacheEntry(const cacheEntry<C>&other);

    virtual ~cacheEntry(){};

    virtual bool find(QStringList&,QLIST<C>&)const;
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
    virtual bool findSingleValid(QStringList&what,C&st)const;
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

    virtual void appendValidSub(QLIST<C>&)const;
    virtual bool isValid()const
    {
        return m_isValid;
    }
    virtual const C&content()const
    {
        return m_content;
    }
    virtual bool deleteKey(QStringList&,bool exact);
    virtual void insertKey(QStringList&,const C&);
    virtual void setValidContent(const QString&key,const C&st)
    {
        m_key = key;
        m_isValid=true;
        m_content=st;
    }
    virtual bool hasValidSubs()const;
    virtual void markInvalid() {
        m_content=C();
        m_isValid=false;
    }
    const QString&key()const {
        return m_key;
    }

    cacheEntry<C>& operator=(const cacheEntry<C>&other);
#if 0
    void dump_tree(int level=0)const
    {
        QString pre;
        pre.fill('-',level);
        std::map<QString,cacheEntry>::const_iterator it;
        for (it=m_subMap.begin();it!=m_subMap.end();++it) {
            std::cout<<pre.latin1()<<it->first.latin1() << " (" << it->second.m_key.latin1() << ")"<<std::endl;
            it->second.dump_tree(level+1);
        }
    }
#endif
};

template<class C> inline cacheEntry<C>::cacheEntry()
    : m_key(""),m_isValid(false)
{
}

template<class C> inline cacheEntry<C>::cacheEntry(const QString&key)
    : m_key(key),m_isValid(false)
{
}

template<class C> inline cacheEntry<C>::cacheEntry(const cacheEntry<C>&other)
    : m_key(other.m_key),m_isValid(other.m_isValid),
        m_content(other.m_content),m_subMap(other.m_subMap)
{
}

template<class C> inline cacheEntry<C>& cacheEntry<C>::operator=(const cacheEntry<C>&other)
{
    m_key=other.m_key;
    m_isValid = other.m_isValid;
    m_content = other.m_content;
    m_subMap = other.m_subMap;
    return *this;
}

template<class C> inline  bool cacheEntry<C>::find(QStringList&what,QLIST<C>&t)const
{
    if (what.count()==0) {
        return false;
    }
    citer it;
    it = m_subMap.find(what[0]);
    if (it==m_subMap.end()) {
       // kdDebug()<<what[0]<< " not found in tree"<<endl;
        return false;
    }
   // kdDebug()<<what[0]<< " found in tree"<<endl;
    if (what.count()==1) {
       // kdDebug()<<"Seems last item in stage "<< m_key << " - " << what[0] << endl;
//         if (it->second.m_key == what[0]) {
        /* the item itself */
        if (it->second.isValid()) {
            t.append(it->second.content());
        }
        /* and now check valid subitems */
           // kdDebug()<<"Appending valid subs"<<endl;
        it->second.appendValidSub(t);
           // kdDebug()<<"Appended valid subs"<<endl;
        return true;
//        }
        return false;
    }
    what.erase(what.begin());
   // kdDebug()<<"Searching "<<what<<" in next stage"<<endl;
    return it->second.find(what,t);
}

template<class C> inline bool cacheEntry<C>::find(QStringList&what)const
{
    if (what.count()==0) {
        return false;
    }
    citer it = m_subMap.find(what[0]);
    if (it==m_subMap.end()) {
        return false;
    }
    if (what.count()==1) {
        return true;
    }
    what.erase(what.begin());
    return it->second.find(what);
}

template<class C> inline bool cacheEntry<C>::findSingleValid(QStringList&what,C&t)const
{
    if (what.count()==0) {
        return false;
    }
   // kdDebug()<<"cacheEntry::findSingleValid(QStringList&what,svn::Status&t)"<< what << endl;
    citer it;
    it = m_subMap.find(what[0]);
    if (it==m_subMap.end()) {
        return false;
    }
    if (what.count()==1) {
        t=it->second.content();
        return it->second.isValid();
    }
    what.erase(what.begin());
    return it->second.findSingleValid(what,t);
}

template<class C> inline bool cacheEntry<C>::findSingleValid(QStringList&what,bool check_valid_subs)const
{
    if (what.count()==0) {
        return false;
    }
   // kdDebug()<<"cacheEntry::findSingleValid(QStringList&what,svn::Status&t)"<< what << endl;
    citer it = m_subMap.find(what[0]);
    if (it==m_subMap.end()) {
        return false;
    }
    if (what.count()==1) {
        return it->second.isValid()||(check_valid_subs&&it->second.hasValidSubs());
    }
    what.erase(what.begin());
    return it->second.findSingleValid(what,check_valid_subs);
}

template<class C> inline void cacheEntry<C>::appendValidSub(QLIST<C>&t)const
{
    citer it;
    for (it=m_subMap.begin();it!=m_subMap.end();++it) {
        if (it->second.isValid()) {
           // kdDebug()<<"Appending single sub"<<endl;
            t.append(it->second.content());
        } else {
           // kdDebug()<<it->second.key()<<" isnt valid"<<endl;
        }
        it->second.appendValidSub(t);
    }
}

template<class C> inline bool cacheEntry<C>::deleteKey(QStringList&what,bool exact)
{
    if (what.count()==0) {
        return true;
    }
    iter it=m_subMap.find(what[0]);
    if (it==m_subMap.end()) {
        return true;
    }
    bool caller_must_check = false;
    /* first stage - we are the one holding the right key */
    if (what.count()==1){
        if (!exact || !it->second.hasValidSubs()) {
            m_subMap.erase(it);
            caller_must_check = true;
        } else {
            it->second.markInvalid();
        }
    } else {
        /* otherwise go trough tree */
        what.erase(what.begin());
        bool b = it->second.deleteKey(what,exact);
        if (b && !it->second.hasValidSubs()) {
            m_subMap.erase(it);
            caller_must_check = true;
        }
    }
    return caller_must_check;
}

template<class C> inline bool cacheEntry<C>::hasValidSubs()const
{
    citer it;
    for (it=m_subMap.begin();it!=m_subMap.end();++it) {
        if (it->second.isValid()||it->second.hasValidSubs()) {
            return true;
        }
    }
    return false;
}

template<class C> inline void cacheEntry<C>::insertKey(QStringList&what,const C&st)
{
    if (what.count()==0) {
        return;
    }
   // kdDebug()<<"inserting "<<what<< "into " << m_key << endl;
    QString m = what[0];

    iter it=m_subMap.find(m);
    if (it==m_subMap.end()) {
        m_subMap[m].m_key=m;
        if (what.count()==1) {
           // kdDebug()<<"Inserting valid key "<< m << endl;
            m_subMap[m].setValidContent(m,st);
           // kdDebug()<<"Inserting valid key done"<< endl;
            return;
        }
       // kdDebug()<<"inserting tree key " << m << endl;
       // kdDebug()<<"inserting tree key done " << m_subMap[m].m_key << endl;
    }

    what.erase(what.begin());
   // kdDebug()<<"Go into loop"<<endl;
    m_subMap[m].insertKey(what,st);
}

template<class C> template<class T> inline void cacheEntry<C>::listsubs_if(QStringList&what,T&oper)const
{
    if (what.count()==0) {
        /* we are the one to get the list for*/
        oper = for_each(m_subMap.begin(),m_subMap.end(),oper);
        return;
    }
    /* otherwise find next */
    citer it = m_subMap.find(what[0]);
    if (it==m_subMap.end()) {
        /* not found */
        return;
    }
    what.erase(what.begin());
    it->second.listsubs_if(what,oper);
}

typedef cacheEntry<svn::Status> statusEntry;

class itemCache
{
protected:
    std::map<QString,statusEntry> m_contentMap;

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
    statusEntry::citer it=m_contentMap.find(what[0]);
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
    void operator()(const std::pair<const QString,helpers::statusEntry>&_data) {
        if(_data.second.content().validReposStatus()&&!_data.second.content().validLocalStatus()) {
            m_List.push_back(_data.second.content());
        }
    }
    const svn::StatusEntries&liste()const{return m_List;}
};

}

#endif
