#include "runtempfile.h"
#include "src/svnfrontend/fronthelpers/settings.h"

#include <kdebug.h>
#include <sys/wait.h>

RunTempfile::RunTempfile(const QByteArray&content)
    : QObject(),_job(0),_file(0),_stat(true),m_timer(this,"RunTempfile::timer"),initdone(false)
{
    _pid = 0;
    _file = new KTempFile();
    _file->setAutoDelete(true);
    _stat = true;
    QDataStream*ds = _file->dataStream();
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    if (!ds) {
        initdone = true;
        m_timer.start(10,true);
        _stat = false;
        return;
    }
    ds->writeRawBytes(content,content.size());
    _file->close();

    QString feditor = Settings::external_display();
    if ( feditor.compare("default") != 0 )
    {
        _pid = KRun::runCommand(feditor + " " +  _file->name());
    }

    m_timer.start(10,true);
}

RunTempfile::~RunTempfile()
{
    kdDebug()<<"Killing run"<<endl;
    delete _file;
}

void RunTempfile::finished()
{
    kdDebug()<<"Finished"<<endl;
    m_timer.start(10000,true);
}

void RunTempfile::timeout()
{
    if (!initdone) {
        if (_pid == 0)
        {
            _job = new KRun(_file->name(),0,true,true);
            connect(_job,SIGNAL(finished()),this,SLOT(finished()));
            initdone=true;
        }
        else
        {
            if ( waitpid(_pid, NULL, WNOHANG) > 0)
            {
                initdone = true;
            }
            else
            {
                m_timer.start(10000,true);
            }
        }
    } else {
        delete this;
    }
}

#include "runtempfile.moc"

