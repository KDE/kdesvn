#include "client_commit_parameter.hpp"
#include "svnqt/svnqttypes.hpp"
#include "svnqt/stringarray.hpp"
#include "svnqt/client_parameter_macros.hpp"

namespace svn
{
    struct CommitParameterData
    {
        CommitParameterData()
            :_targets(),_message(QString()),_depth(DepthInfinity),_changeList(StringArray()),
             _revProps(PropertiesMap()),_keepLocks(false),_keepChangeList(false)
        {}
        Targets _targets;
        QString _message;
        Depth _depth;
        StringArray _changeList;
        PropertiesMap _revProps;
        bool _keepLocks,_keepChangeList;
    };

    CommitParameter::CommitParameter()
    {
        _data = new CommitParameterData;
    }
    CommitParameter::~CommitParameter()
    {
        _data = 0;
    }
    GETSET(CommitParameter,Targets,_targets,targets)
    GETSET(CommitParameter,QString,_message,message)
    GETSET(CommitParameter,StringArray,_changeList,changeList)
    GETSET(CommitParameter,PropertiesMap,_revProps,revisionProperties)
    GETSETSI(CommitParameter,Depth,_depth,depth)
    GETSETSI(CommitParameter,bool,_keepLocks,keepLocks)
    GETSETSI(CommitParameter,bool,_keepChangeList,keepChangeList)
}
