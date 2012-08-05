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

#ifndef SVNQT_SHARED_POINTER_HPP
#define SVNQT_SHARED_POINTER_HPP

#include "svnqt/smart_pointer.h"

/*!
 * \file shared_pointer.h
 * \brief shared pointer adapter
 * \sa smart_pointer.h
 */

namespace svn
{

template<class T> class SharedPointer;

/*!
 * Data container for svn::SharedPointer
 */
template<class T> class SharedPointerData:public ref_count
{
    friend class SharedPointer<T>;
protected:
    //! The protected pointer
    T*data;
public:
    //! Constructor
    /*!
     * Take ownership of pointer dt
     * \param dt the data to wrap
     **/
    SharedPointerData(T*dt){
        data = dt;
    }
    //! Destructor
    /*!
     * Release content data
     */
    ~SharedPointerData() {
        delete data;
    }
};

//! Shared pointer adapter
/*!
 * Implements a thread safe reference counter around any pointer.
 * This class takes ownership of data, eg., last reference will delete
 * the data it inspects.
 */
template<class T> class SharedPointer
{
    typedef SharedPointerData<T> Data;
    Data*data;

    //! count down reference of data and release if it was the last share
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
    //! empty constructor
    SharedPointer():data(0){}
    //! copy constructor
    /*!
     * \param p Data to increase reference for
     */
    SharedPointer(const SharedPointer<T>& p)
    {
        if ( (data = p.data) ) data->Incr();
    }
    //! assignment constructor
    /*!
     * Take ownership of data pointer t
     * \param t data pointer to store inside
     */
    SharedPointer(T*t)
    {
        data = new Data(t);data->Incr();
    }
    //! destructor
    /*!
     * decrease reference, if reference == 0 release data
     */
    ~SharedPointer()
    {
        unref();
    }

    //! assignment operator
    /*!
     * \param p Data to increase reference for
     */
    SharedPointer<T> &operator=(const SharedPointer<T>&p)
    {
        // we always have a reference to the data
        if (data==p.data) return *this;
        unref();
        if ((data=p.data)) data->Incr();
        return *this;
    }
    //! assignment operator
    /*!
     * \param p Data to increase reference for
     */
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

    //! access operator
    /*!
     * Use this operator with care!
     * \return pointer to wrapped data
     */
    operator T*()const       {return data->data;}
    //! access operator
    /*!
     * \return reference to wrapped data
     */
    T& operator*()           {return *data->data;}
    //! access operator
    /*!
     * \return const reference to wrapped data
     */
    const T& operator*()const{return *data->data;}
    //! access operator
    /*!
     * \return pointer to wrapped data
     */
    T*operator->()           {return data->data;}
    //! access operator
    /*!
     * \return const pointer to wrapped data
     */
    const T*operator->()const{return data->data;}

    //! Bool operator
    /*!
     * \return true if content set and not a null-pointer, otherwise false
     */
    operator bool () const { return (data != 0 && data->data != 0); }
    //! Bool operator
    /*!
     * \return true if content set and not a null-pointer, otherwise false
     */
    operator bool () { return ( data != 0 && data->data != 0 );}

    //! Negation operator
    /*!
     * \return true if content not set or a null-pointer, otherwise false
     */
    bool operator! () const { return (data == 0 || data->data == 0); }
    //! Negation operator
    /*!
     * \return true if content not set or a null-pointer, otherwise false
     */
    bool operator! () { return (data == 0 || data->data == 0); }

    //! required for searches in lists etc.
    bool operator==(const T*p)const{return (data != 0 && data->data==p);}
    //! required for searches in lists etc.
    bool operator==(T*p){return (data && data->data==p);}
};

}

#endif
