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
        m_contentMap[_keys[0]]=statusEntry(_keys[0]);
        if (_keys.count()==1) {
            m_contentMap[_keys[0]].setValidContent(_keys[0],(*it));
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
    std::map<QString,statusEntry>::iterator it=m_contentMap.find(_keys[0]);

    if (it==m_contentMap.end()) {
        m_contentMap[_keys[0]]=statusEntry(_keys[0]);
    }
    if (_keys.count()==1) {
        m_contentMap[_keys[0]].setValidContent(_keys[0],st);
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
    std::map<QString,statusEntry>::const_iterator it=m_contentMap.find(_keys[0]);
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
    std::map<QString,statusEntry>::const_iterator it=m_contentMap.find(what[0]);
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
    std::map<QString,statusEntry>::iterator it=m_contentMap.find(what[0]);
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
    std::map<QString,statusEntry>::const_iterator it;
    for (it=m_contentMap.begin();it!=m_contentMap.end();++it) {
        std::cout<<it->first.latin1() << " (" << it->second.key().latin1() << ")"<<std::endl;
//        it->second.dump_tree(1);
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
    std::map<QString,statusEntry>::const_iterator it=m_contentMap.find(what[0]);
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
    std::map<QString,statusEntry>::const_iterator it=m_contentMap.find(what[0]);
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
