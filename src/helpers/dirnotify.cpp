#include "dirnotify.h"
#include <kio/job.h>
#include <kdirlister.h>
#include <kdebug.h>

DirNotify::DirNotify()
    : QObject(),m_Lister(0)
{
}

DirNotify::~DirNotify()
{
    stop();
}

void DirNotify::setBase(const KURL&_b)
{
    kdDebug()<<"Setting base to " << _b.path()<<endl;
    if (!m_Lister) {
        m_Lister = new KDirLister(true);
    }
    m_Lister->openURL(_b);
}

void DirNotify::stop()
{
    if (!m_Lister) return;
    disconnect(m_Lister);
    delete m_Lister;
    m_Lister = 0;
}

void DirNotify::FilesAdded( const KURL &directory )
{
    kdDebug() << "Files added: " << directory.url()<< endl;
}

void DirNotify::FilesRemoved( const KURL::List &fileList )
{
    kdDebug() << "files removed"<<endl;
}

void DirNotify::FilesChanged( const KURL::List &fileList )
{
    kdDebug()<< "Files changed"<<endl;
}

void DirNotify::FileRenamed( const KURL &src, const KURL &dst )
{
    kdDebug()<< "files removed"<<endl;
}

void DirNotify::slotFileDirty( const QString &_file )
{
    kdDebug()<< "File dirty "<<_file<<endl;
}

void DirNotify::slotFileCreated( const QString &_file )
{
    kdDebug()<< "File new "<<_file<<endl;
}

void DirNotify::slotFileDeleted( const QString &_file )
{
    kdDebug()<< "File deleted "<<_file<<endl;
}
