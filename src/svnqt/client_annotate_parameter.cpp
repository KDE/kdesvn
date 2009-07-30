#include "client_annotate_parameter.hpp"
#include "svnqt/svnqttypes.hpp"
#include "svnqt/stringarray.hpp"
#include "svnqt/client_parameter_macros.hpp"
#include "svnqt/diffoptions.hpp"

namespace svn
{
    struct AnnotateParameterData
    {
        AnnotateParameterData()
            :_path(),_revisions(Revision::UNDEFINED,Revision::UNDEFINED),_peg(Revision::UNDEFINED),_opts(),
            _ignoreMimeTypes(false),_includeMerged(true)
        {}
        Path _path;
        RevisionRange _revisions;
        Revision _peg;
        DiffOptions _opts;
        bool _ignoreMimeTypes,_includeMerged;
    };

    AnnotateParameter::AnnotateParameter()
    {
        _data = new AnnotateParameterData;
    }
    AnnotateParameter::~AnnotateParameter()
    {
        _data = 0;
    }
    GETSET(AnnotateParameter,Path,_path,path)
    GETSET(AnnotateParameter,RevisionRange,_revisions,revisionRange)
    GETSET(AnnotateParameter,Revision,_peg,pegRevision)
    GETSET(AnnotateParameter,DiffOptions,_opts,diffOptions)

    GETSETSI(AnnotateParameter,bool,_ignoreMimeTypes,ignoreMimeTypes)
    GETSETSI(AnnotateParameter,bool,_includeMerged,includeMerged)
}
