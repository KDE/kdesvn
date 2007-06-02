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
#include "cacheentry.h"

#include <kdebug.h>

#include <iostream>

namespace helpers {

cacheEntry::cacheEntry()
    : m_key(""),m_isValid(false)
{
}

cacheEntry::cacheEntry(const QString&key)
    : m_key(key),m_isValid(false)
{
}

cacheEntry::cacheEntry(const cacheEntry&other)
    : m_key(other.m_key),m_isValid(other.m_isValid),
        m_content(other.m_content),m_subMap(other.m_subMap)
{
}

cacheEntry& cacheEntry::operator=(const cacheEntry&other)
{
    m_key=other.m_key;
    m_isValid = other.m_isValid;
    m_content = other.m_content;
    m_subMap = other.m_subMap;
    return *this;
}

cacheEntry::~cacheEntry()
{
}

bool cacheEntry::find(QStringList&what,svn::StatusEntries&t)const
{
    if (what.count()==0) {
        return false;
    }
    std::map<QString,cacheEntry>::const_iterator it;
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

bool cacheEntry::find(QStringList&what)const
{
    if (what.count()==0) {
        return false;
    }
    std::map<QString,cacheEntry>::const_iterator it;
    it = m_subMap.find(what[0]);
    if (it==m_subMap.end()) {
        return false;
    }
    if (what.count()==1) {
        return true;
    }
    what.erase(what.begin());
    return it->second.find(what);
}

bool cacheEntry::findSingleValid(QStringList&what,svn::Status&t)const
{
    if (what.count()==0) {
        return false;
    }
   // kdDebug()<<"cacheEntry::findSingleValid(QStringList&what,svn::Status&t)"<< what << endl;
    std::map<QString,cacheEntry>::const_iterator it;
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

bool cacheEntry::findSingleValid(QStringList&what,bool check_valid_subs)const
{
    if (what.count()==0) {
        return false;
    }
   // kdDebug()<<"cacheEntry::findSingleValid(QStringList&what,svn::Status&t)"<< what << endl;
    std::map<QString,cacheEntry>::const_iterator it;
    it = m_subMap.find(what[0]);
    if (it==m_subMap.end()) {
        return false;
    }
    if (what.count()==1) {
        return it->second.isValid()||(check_valid_subs&&it->second.hasValidSubs());
    }
    what.erase(what.begin());
    return it->second.findSingleValid(what,check_valid_subs);
}


bool cacheEntry::isValid()const
{
    return m_isValid;
}

void cacheEntry::appendValidSub(svn::StatusEntries&t)const
{
    std::map<QString,cacheEntry>::const_iterator it;
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

const svn::Status&cacheEntry::content()const
{
    return m_content;
}

const QString&cacheEntry::key()const
{
    return m_key;
}

bool cacheEntry::deleteKey(QStringList&what,bool exact)
{
    if (what.count()==0) {
        return true;
    }
    std::map<QString,cacheEntry>::iterator it=m_subMap.find(what[0]);
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

bool cacheEntry::hasValidSubs()const
{
    std::map<QString,cacheEntry>::const_iterator it;
    for (it=m_subMap.begin();it!=m_subMap.end();++it) {
        if (it->second.isValid()||it->second.hasValidSubs()) {
            return true;
        }
    }
    return false;
}

void cacheEntry::markInvalid()
{
    m_content = svn::Status();
    m_isValid=false;
}

void cacheEntry::insertKey(QStringList&what,const svn::Status&st)
{
    if (what.count()==0) {
        return;
    }
   // kdDebug()<<"inserting "<<what<< "into " << m_key << endl;
    QString m = what[0];

    std::map<QString,cacheEntry>::iterator it=m_subMap.find(m);
    if (it==m_subMap.end()) {
        m_subMap[m].m_key=m;
        if (what.count()==1) {
           // kdDebug()<<"Inserting valid key "<< m << endl;
            m_subMap[m].setValidStatus(m,st);
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

void cacheEntry::setValidStatus(const QString&key,const svn::Status&st)
{
    m_key = key;
    m_isValid=true;
    m_content=st;
}

void cacheEntry::dump_tree(int level)const
{
    QString pre;
    pre.fill('-',level);
    std::map<QString,cacheEntry>::const_iterator it;
    for (it=m_subMap.begin();it!=m_subMap.end();++it) {
        std::cout<<pre.latin1()<<it->first.latin1() << " (" << it->second.m_key.latin1() << ")"<<std::endl;
        it->second.dump_tree(level+1);
    }
}

/* cache box itself */
itemCache::itemCache()
    :m_contentMap()
{
   // kdDebug()<<"itemCache::itemCache()"<<endl;
}

itemCache::~itemCache()
{
   // kdDebug()<<"itemCache::~itemCache()"<<endl;
}

void itemCache::clear()
{
    m_contentMap.clear();
}

void itemCache::setContent(const svn::StatusEntries&dlist)
{
    m_contentMap.clear();
    svn::StatusEntries::const_iterator it;
    for (it=dlist.begin();it!=dlist.end();++it) {
        QStringList _keys = QStringList::split("/",(*it).path());
        if (_keys.count()==0) {
            continue;
        }
        m_contentMap[_keys[0]]=cacheEntry(_keys[0]);
        if (_keys.count()==1) {
            m_contentMap[_keys[0]].setValidStatus(_keys[0],(*it));
        } else {
            _keys.erase(_keys.begin());
            m_contentMap[_keys[0]].insertKey(_keys,(*it));
        }
    }
}

void itemCache::insertKey(const svn::Status&st)
{
   // kdDebug()<<"Inserting "<<st.path()<<endl;
    QStringList _keys = QStringList::split("/",st.path());
    if (_keys.count()==0) {
        return;
    }
    std::map<QString,cacheEntry>::iterator it=m_contentMap.find(_keys[0]);

    if (it==m_contentMap.end()) {
        m_contentMap[_keys[0]]=cacheEntry(_keys[0]);
    }
    if (_keys.count()==1) {
        m_contentMap[_keys[0]].setValidStatus(_keys[0],st);
    } else {
        QString m = _keys[0];
        _keys.erase(_keys.begin());
        m_contentMap[m].insertKey(_keys,st);
    }
}

bool itemCache::find(const QString&what)const
{
    if (m_contentMap.size()==0) {
        return false;
    }
    QStringList _keys = QStringList::split("/",what);
    if (_keys.count()==0) {
        return false;
    }
    std::map<QString,cacheEntry>::const_iterator it=m_contentMap.find(_keys[0]);
    if (it==m_contentMap.end()) {
        return false;
    }
    if (_keys.count()==1) {
        return true;
    }
    _keys.erase(_keys.begin());
    return it->second.find(_keys);
}

bool itemCache::find(const QString&_what,svn::StatusEntries&dlist)const
{
    if (m_contentMap.size()==0) {
        return false;
    }
    QStringList what = QStringList::split("/",_what);
    if (what.count()==0) {
        return false;
    }
    std::map<QString,cacheEntry>::const_iterator it=m_contentMap.find(what[0]);
    if (it==m_contentMap.end()) {
        return false;
    }
    what.erase(what.begin());
   // kdDebug()<<"itemCache::find(const QString&_what,svn::StatusEntries&dlist) "<<what<<endl;
    return it->second.find(what,dlist);
}

void itemCache::deleteKey(const QString&_what,bool exact)
{
    if (m_contentMap.size()==0) {
        return;
    }
    QStringList what = QStringList::split("/",_what);
    if (what.count()==0) {
        return;
    }
    std::map<QString,cacheEntry>::iterator it=m_contentMap.find(what[0]);
    if (it==m_contentMap.end()) {
        return;
    }
    /* first stage - we are the one holding the right key */
    if (what.count()==1){
        if (!exact || !it->second.hasValidSubs()) {
            /* if it has no valid subs delete it */
            m_contentMap.erase(it);
        } else {
            /* otherwise mark as invalid */
            it->second.markInvalid();
        }
        return;
    } else {
        /* otherwise go trough tree */
        what.erase(what.begin());
        bool b = it->second.deleteKey(what,exact);
        if (b && !it->second.hasValidSubs()) {
            m_contentMap.erase(it);
        }
    }
}

void itemCache::dump_tree()
{
    std::map<QString,cacheEntry>::const_iterator it;
    for (it=m_contentMap.begin();it!=m_contentMap.end();++it) {
        std::cout<<it->first.latin1() << " (" << it->second.key().latin1() << ")"<<std::endl;
        it->second.dump_tree(1);
    }
}

bool itemCache::findSingleValid(const QString&_what,svn::Status&st)const
{
    if (m_contentMap.size()==0) {
        return false;
    }
    QStringList what = QStringList::split("/",_what);
    if (what.count()==0) {
        return false;
    }
    std::map<QString,cacheEntry>::const_iterator it=m_contentMap.find(what[0]);
    if (it==m_contentMap.end()) {
        return false;
    }
    if (what.count()==1) {
        if (it->second.isValid()) {
            st=it->second.content();
            return true;
        }
        return false;
    }
    what.erase(what.begin());
    return it->second.findSingleValid(what,st);
}

bool itemCache::findSingleValid(const QString&_what,bool check_valid_subs)const
{
    if (m_contentMap.size()==0) {
        return false;
    }
    QStringList what = QStringList::split("/",_what);
    if (what.count()==0) {
        return false;
    }
    std::map<QString,cacheEntry>::const_iterator it=m_contentMap.find(what[0]);
    if (it==m_contentMap.end()) {
        return false;
    }
    if (what.count()==1) {
        return it->second.isValid()||(check_valid_subs&&it->second.hasValidSubs());
    }
    what.erase(what.begin());
    return it->second.findSingleValid(what,check_valid_subs);
}

}
