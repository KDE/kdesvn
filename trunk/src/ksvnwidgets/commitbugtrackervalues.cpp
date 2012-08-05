/*
    The Kdesvn project - subversion client for kde http://kdesvn.alwins-world.de/
    Copyright (C) 2010  Rajko Albrecht

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "commitbugtrackervalues.h"
#include <klocalizedstring.h>

class CommitBugtrackerValuesData
{
    public:
        CommitBugtrackerValuesData()
        :_warnifnoissue(false),_label(i18n("Issue-Number")),_message(QString()),_number(true),_append(false)
        {}

        bool _warnifnoissue;
        QString _label;
        QString _message;
        bool _number;
        bool _append;
};

CommitBugtrackerValues::CommitBugtrackerValues()
    :_data(new CommitBugtrackerValuesData())
{
}

CommitBugtrackerValues::CommitBugtrackerValues(const CommitBugtrackerValues&old)
    :_data(new CommitBugtrackerValuesData())
{
    _data->_warnifnoissue = old._data->_warnifnoissue;
    _data->_label = old._data->_label;
    _data->_message = old._data->_message;
    _data->_number = old._data->_number;
    _data->_append = old._data->_append;
}

CommitBugtrackerValues::~CommitBugtrackerValues()
{
    _data = 0;
}

bool CommitBugtrackerValues::Warnifnoissue()const
{
    return _data->_warnifnoissue;
}

CommitBugtrackerValues& CommitBugtrackerValues::Warnifnoissue(bool value)
{
    _data->_warnifnoissue = value;
    return *this;
}

const QString&CommitBugtrackerValues::Label()const
{
    return _data->_label;
}

CommitBugtrackerValues&CommitBugtrackerValues::Label(const QString&value)
{
    _data->_label=value;
    return *this;
}

const QString&CommitBugtrackerValues::Message()const
{
    return _data->_message;
}

CommitBugtrackerValues&CommitBugtrackerValues::Message(const QString&value)
{
    _data->_message = value;
    return *this;
}

bool CommitBugtrackerValues::Number()const
{
    return _data->_number;
}

CommitBugtrackerValues&CommitBugtrackerValues::Number(bool value)
{
    _data->_number=value;
    return *this;
}

bool CommitBugtrackerValues::Append()const
{
    return _data->_append;
}

CommitBugtrackerValues&CommitBugtrackerValues::Append(bool value)
{
    _data->_append=value;
    return *this;
}
