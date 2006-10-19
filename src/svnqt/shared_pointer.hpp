/***************************************************************************
 *   Copyright (C) 2006 by Rajko Albrecht                                  *
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

#ifndef SVNQT_SHARED_POINTER_HPP
#define SVNQT_SHARED_POINTER_HPP

#include "svnqt/smart_pointer.hpp"

/*!
 * \file shared_pointer.hpp
 * \brief shared pointer adapter
 */

namespace svn
{

template<class T> class SharedPointer;

template<class T> class SharedPointerData:public ref_count
{
    friend class SharedPointer<T>;
protected:
    T*data;
public:
    SharedPointerData(T*dt){
        data = dt;
    }
    ~SharedPointerData() {
        delete data;
    }
};

template<class T> class SharedPointer
{
    typedef SharedPointerData<T> Data;
    Data*data;
    void unref(){
        if (data) {
            data->Decr();
            if (!data->Shared()) {
                delete data;
            }
            data = 0;
        }
    }
public:
    SharedPointer():data(0){}
    SharedPointer(const SharedPointer<T>& p) {if ( (data = p.data) ) data->Incr();}
    SharedPointer(T*t){data = new Data(t);data->Incr();}
    ~SharedPointer(){unref();}

    SharedPointer<T> &operator=(const SharedPointer<T>&p)
    {
        if (data==p.data) return *this;
        unref();
        if ((data=p.data)) data->Incr();
        return *this;
    }
    SharedPointer<T> &operator=(T*p)
    {
        if (data && data->data==p) {
            return *this;
        }
        unref();
        data = new Data(p);
        data->Incr();
        return *this;
    }

    operator T*()const       {return data->data;}
    T& operator*()           {return *data->data;}
    const T& operator*()const{return *data->data;}
    T*operator->()           {return data->data;}
    const T*operator->()const{return data->data;}

    operator bool () const { return (data != 0 && data->data != 0); }
    operator bool () { return ( data != 0 && data->data != NULL );}

    bool operator! () const { return (data == 0 || data->data == 0); }
    bool operator! () { return (data == 0 || data->data == 0); }
};

}

#endif
