/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
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
#ifndef COMMANDEXEC_H
#define COMMANDEXEC_H

#include <qobject.h>
#include <qstring.h>

class KCmdLineArgs;
class pCPart;

namespace svn
{
class Revision;
}
class QCommandLineParser;

/**
@author Rajko Albrecht
*/
class CommandExec : public QObject
{
    Q_OBJECT
public:
    explicit CommandExec(QObject *parent);
    ~CommandExec();
    virtual int exec(const QCommandLineParser *parser);

protected slots:
    virtual void clientException(const QString &);
    virtual void slotNotifyMessage(const QString &);
    virtual void slotCmd_log();
    virtual void slotCmd_update();
    virtual void slotCmd_diff();
    virtual void slotCmd_blame();
    virtual void slotCmd_info();
    virtual void slotCmd_commit();
    virtual void slotCmd_cat();
    virtual void slotCmd_get();
    virtual void slotCmd_list();
    virtual void slotCmd_copy();
    virtual void slotCmd_move();
    virtual void slotCmd_checkout();
    virtual void slotCmd_checkoutto();
    virtual void slotCmd_export();
    virtual void slotCmd_exportto();
    virtual void slotCmd_delete();
    virtual void slotCmd_add();
    virtual void slotCmd_revert();
    virtual void slotCmd_addnew();
    virtual void slotCmd_tree();
    virtual void slotCmd_lock();
    virtual void slotCmd_unlock();

signals:
    void executeMe();
protected:
    virtual bool scanRevision();
    virtual bool askRevision();

    QString m_lastMessages;
    unsigned int m_lastMessagesLines;

private:
    pCPart *m_pCPart;

protected slots:
    void slotCmd_switch();
};

#endif
