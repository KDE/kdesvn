#ifndef SIMPLE_LOGCB_H
#define SIMPLE_LOGCB_H

namespace svn
{
    class LogEntry;
    class Revision;
}

class QString;

class SimpleLogCb
{
public:
    SimpleLogCb(){}
    virtual ~SimpleLogCb(){}

    virtual bool getSingleLog(svn::LogEntry&,const svn::Revision&,const QString&,const svn::Revision&,QString&root) = 0;
};

#endif
