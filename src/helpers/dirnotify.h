#ifndef __DIRNOTIFY_H
#define __DIRNOTIFY_H

#include <kdirnotify.h>
#include <qstring.h>
#include <kurl.h>
#include <kdirwatch.h>

class KDirLister;

class DirNotify:public QObject
{
    Q_OBJECT
public:
    DirNotify();
    virtual ~DirNotify();
    virtual void setBase(const KURL&);
    virtual void stop();

    virtual void FilesAdded( const KURL &directory );
    virtual void FilesRemoved( const KURL::List &fileList );
    virtual void FilesChanged( const KURL::List &fileList );
    virtual void FileRenamed( const KURL &src, const KURL &dst );

protected:
    KDirLister*m_Lister;

protected slots:

    virtual void slotFileDirty( const QString &);
    virtual void slotFileCreated( const QString &);
    virtual void slotFileDeleted( const QString & );
};

#endif
