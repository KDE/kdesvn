#include "dirnotify.h"
#include <kio/job.h>
#include <kdirlister.h>

DirNotify::DirNotify()
    : QObject(),m_Lister(0)
{
    qDebug("Creating dirnotify");
}

DirNotify::~DirNotify()
{
    stop();
}

void DirNotify::setBase(const KURL&_b)
{
    qDebug("Setting base to %s",_b.path().latin1());
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
    qDebug("Files added: %s",directory.url().latin1());
}

void DirNotify::FilesRemoved( const KURL::List &fileList )
{
    qDebug("files removed");
}

void DirNotify::FilesChanged( const KURL::List &fileList )
{
    qDebug("Files changed");
}

void DirNotify::FileRenamed( const KURL &src, const KURL &dst )
{
    qDebug("files removed");
}

void DirNotify::slotFileDirty( const QString &_file )
{
    qDebug("File dirty %s",_file.latin1());
}

void DirNotify::slotFileCreated( const QString &_file )
{
    qDebug("File new %s",_file.latin1());
}

void DirNotify::slotFileDeleted( const QString &_file )
{
    qDebug("File deleted %s",_file.latin1());
}
