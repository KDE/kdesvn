/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
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
/*!
 * \file cursorstack.h
 * \brief Defines and implements CursorStack
 */
#ifndef CURSOR_STACK_H
#define CURSOR_STACK_H

#include <kapplication.h>
#include <QCursor>

//! Change cursor on stack.
/*! May used in methods where more than returns exists. Cursor will restored on destruction
 * of class instance.
 */
class CursorStack
{
public:
    //! Constructor.
    /*!
     * Create instance and changes the application cursor to \a c
     * \param c cursortype to set.
     */
    explicit CursorStack(Qt::CursorShape c = Qt::WaitCursor)
    {
        KApplication::setOverrideCursor(QCursor(c));
    }
    //! Destructor.
    /*!
     * Restores the application cursor to value before construction.
     */
    ~CursorStack()
    {
        KApplication::restoreOverrideCursor();
    }
};

#endif
