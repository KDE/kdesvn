/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht                                  *
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
#include "kiosvn.h"

#include "svnfrontend/svnactions.h"
#include "svnfrontend/dummydisplay.h"
#include "svncpp/dirent.hpp"
#include "svncpp/url.hpp"
#include "svncpp/status.hpp"
#include "helpers/sub2qt.h"

#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kdemacros.h>
#include <kmessagebox.h>
#include <kinstance.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kurl.h>
#include <ksock.h>
#include <dcopclient.h>
#include <qcstring.h>
#include <kmimetype.h>

#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

class KioSvnData
{
public:
    KioSvnData();
    virtual ~KioSvnData();

    SvnActions*m_SvnWrapper;
    DummyDisplay * disp;
};

KioSvnData::KioSvnData()
{
    disp = new DummyDisplay();
    m_SvnWrapper = new SvnActions(disp);
    m_SvnWrapper->reInitClient();
}

KioSvnData::~KioSvnData()
{
    delete m_SvnWrapper;
    delete disp;
}

kio_svnProtocol::kio_svnProtocol(const QCString &pool_socket, const QCString &app_socket)
    : SlaveBase("kio_svn", pool_socket, app_socket)
{
    m_pData=new KioSvnData;
}

kio_svnProtocol::~kio_svnProtocol()
{
    delete m_pData;
}

extern "C"
{
    KDE_EXPORT int kdemain(int argc, char **argv);
}

int kdemain(int argc, char **argv)    {
    kdDebug()<<"kdemain" << endl;
    KInstance instance( "kio_svn" );

    kdDebug(7101) << "*** Starting kio_svn " << endl;

    if (argc != 4) {
        kdDebug(7101) << "Usage: kio_svn  protocol domain-socket1 domain-socket2" << endl;
        exit(-1);
    }

    kio_svnProtocol slave(argv[2], argv[3]);
    slave.dispatchLoop();

    kdDebug(7101) << "*** kio_svn Done" << endl;
    return 0;
}


/*!
    \fn kio_svnProtocol::listDir (const KURL&url)
 */
void kio_svnProtocol::listDir(const KURL&url)
{
    kdDebug() << "kio_svn::listDir(const KURL& url) : " << url.url() << endl ;
    m_pData->m_SvnWrapper->reInitClient();
    svn::DirEntries dlist;
    svn::Revision rev = helpers::sub2qt::urlToRev(url);
    if (rev == svn::Revision::UNDEFINED) {
        rev = svn::Revision::HEAD;
    }
    bool ret = m_pData->m_SvnWrapper->makeList(makeSvnUrl(url),dlist,rev,false);
    if (!ret) {
        return;
    }
    KIO::UDSEntry entry;
    for (unsigned int i=0; i < dlist.size();++i) {
        if ( createUDSEntry(dlist[i].name(),dlist[i].lastAuthor(),dlist[i].size(),
            dlist[i].kind()==svn_node_dir?true:false,0,entry) ) {
            listEntry(entry,false);
        }
    }
}

void kio_svnProtocol::stat(const KURL& url)
{
    kdDebug()<<"kio_svn::stat"<<endl;
    m_pData->m_SvnWrapper->reInitClient();
    svn::Revision rev = helpers::sub2qt::urlToRev(url);
    if (rev == svn::Revision::UNDEFINED) {
        rev = svn::Revision::HEAD;
    }
    svn::StatusEntries dlist;
    bool ret = m_pData->m_SvnWrapper->makeStatus(makeSvnUrl(url),dlist,rev,false,true,false,false);
    if (!ret || dlist.size()<1) {
        return;
    }
    KIO::UDSEntry entry;
    if (dlist[0].entry().kind()==svn_node_file) {
        createUDSEntry(url.filename(),"",0,false,0,entry);
    } else {
        createUDSEntry(url.filename(),"",0,true,0,entry);
    }
    finished();
}

QString kio_svnProtocol::makeSvnUrl(const KURL&url)
{
    QString res;
    QString proto = svn::Url::transformProtokoll(url.protocol());
    KURL _url = url;
    _url.cleanPath(true);
    _url.setProtocol(proto);
    res = _url.url(-1);
    return res;
}

bool kio_svnProtocol::createUDSEntry( const QString& filename, const QString& user, long int size, bool isdir, time_t mtime, KIO::UDSEntry& entry) {
        kdDebug() << "MTime : " << ( long )mtime << endl;
        kdDebug() << "UDS filename : " << filename << endl;
        KIO::UDSAtom atom;
        atom.m_uds = KIO::UDS_NAME;
        atom.m_str = filename;
        entry.append( atom );

        atom.m_uds = KIO::UDS_FILE_TYPE;
        atom.m_long = isdir ? S_IFDIR : S_IFREG;
        entry.append( atom );

        atom.m_uds = KIO::UDS_SIZE;
        atom.m_long = size;
        entry.append( atom );

        atom.m_uds = KIO::UDS_MODIFICATION_TIME;
        atom.m_long = mtime;
        entry.append( atom );

        atom.m_uds = KIO::UDS_USER;
        atom.m_str = user;
        entry.append( atom );

        return true;
}
