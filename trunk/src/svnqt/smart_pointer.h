/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
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
#ifndef smart_pointer_h
#define smart_pointer_h

#include <QMutex>

#include "svnqt/svnqt_defines.h"

/*!
 * \file smart_pointer.h
 * \brief smart pointer and reference counter
 * \author Rajko Albrecht (ral@alwins-world.de)
 *
 */

namespace svn
{

//! simple reference counter class
class ref_count {
protected:
    //! reference count member
    long m_RefCount;
    QMutex m_RefcountMutex;
public:
    //! first reference must be added after "new" via Pointer()
    ref_count() : m_RefCount(0)
                  ,m_RefcountMutex()
    {}
    virtual ~ref_count() {}
    //! add a reference
    void Incr() {
        QMutexLocker a(&m_RefcountMutex);
        ++m_RefCount;
    }
    //! delete a reference
    bool Decr() {
        QMutexLocker a(&m_RefcountMutex);
        --m_RefCount;
        return Shared();
    }
    //! is it referenced
    bool Shared() { return (m_RefCount > 0); }
};

//! reference counting wrapper class
template<class T> class smart_pointer {
    //! pointer to object
    /*!
     * this object must contain Incr(), Decr() and Shared()
     * methode as public members. The best way is, that it will be a child
     * class of RefCount
     */
    T *ptr;
public:
    //! standart constructor
    smart_pointer() { ptr = 0; }
    //! standart destructor
    /*!
     * release the reference, if it were the last reference, destroys
     * ptr
     */
    ~smart_pointer()
    {
        if (ptr && !ptr->Decr()) {
            delete ptr;
        }
    }
    //! construction
    smart_pointer(T* t) { if ( (ptr = t) ) ptr->Incr(); }
    //! Pointer copy
    smart_pointer(const smart_pointer<T>& p)
    { if ( (ptr = p.ptr) ) ptr->Incr(); }
    //! pointer copy by assignment
    smart_pointer<T>& operator= (const smart_pointer<T>& p)
    {
        // already same: nothing to do
        if (ptr == p.ptr) return *this;
        // decouple reference
        if ( ptr && !ptr->Decr()) delete ptr;
        // establish new reference
        if ( (ptr = p.ptr) ) ptr->Incr();
        return *this;
    }
    smart_pointer<T>& operator= (T*p)
    {
        if (ptr==p)return *this;
        if (ptr && !ptr->Decr()) delete ptr;
        if ( (ptr=p) ) ptr->Incr();
        return *this;
    }

    //! cast to conventional pointer
    operator T* () const { return ptr; }

    //! deref: fails for 0 pointer
    T& operator* () {return *ptr; }
    //! deref: fails for 0 pointer
    const T& operator* ()const {return *ptr; }

    //! deref with method call
    T* operator-> () {return ptr; }
    //! deref with const method call
    const T* operator-> ()const {return ptr; }

    //! supports "if (pointer)"
    operator bool () const { return (ptr != 0); }
    //! "if (pointer)" as non const
    operator bool () { return ptr != 0;}

    //! support if (!pointer)"
    bool operator! () const { return (ptr == 0); }
    //! support if (!pointer)" as non const
    bool operator! () { return (ptr == 0); }
};

} // namespace svn
#endif
