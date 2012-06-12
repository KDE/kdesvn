/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this program (in the file LGPL.txt); if not,         *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at http://kdesvn.alwins-world.de.           *
 ***************************************************************************/
#include "client_parameter.h"
#include "svnqt/svnqttypes.h"
#include "svnqt/stringarray.h"

#include "svnqt/client_parameter_macros.h"

namespace svn
{
    //! internal data structure
    struct SVNQT_NOEXPORT CopyParameterData
    {
        CopyParameterData()
            :_srcPath(),_srcRevision(),_pegRevision(),_destPath(),_asChild(false),_makeParent(false),_ignoreExternal(false),_force(false),_properties()
        {
        }
        Targets  _srcPath;
        Revision _srcRevision;
        Revision _pegRevision;
        Path _destPath;
        bool _asChild;
        bool _makeParent;
        bool _ignoreExternal;
        //! used for move operation instead of copy
        bool _force;
        PropertiesMap _properties;
    };

    CopyParameter::CopyParameter(const Targets&_srcPath,const Path&_destPath)
    {
        _data = new CopyParameterData();
        _data->_srcPath = _srcPath;
        _data->_destPath = _destPath;
    }

    CopyParameter::~CopyParameter()
    {
        _data = 0;
    }

    GETSET(CopyParameter,Targets,_srcPath,srcPath);
    GETSET(CopyParameter,Path,_destPath,destination);
    GETSET(CopyParameter,Revision,_srcRevision,srcRevision);
    GETSET(CopyParameter,Revision,_pegRevision,pegRevision);
    GETSET(CopyParameter,PropertiesMap,_properties,properties);

    GETSETSI(CopyParameter,bool,_asChild,asChild);
    GETSETSI(CopyParameter,bool,_makeParent,makeParent);
    GETSETSI(CopyParameter,bool,_force,force);
    GETSETSI(CopyParameter,bool,_ignoreExternal,ignoreExternal);

    struct SVNQT_NOEXPORT DiffParameterData
    {
    public:
        DiffParameterData()
            :_tmpPath(),_path1(),_path2(),_relativeTo(),_changeList(),_ignoreAncestry(false),_noDiffDeleted(false),
                      _depth(DepthInfinity),_peg_revision(Revision::UNDEFINED),
                      _rev1(Revision::START),_rev2(Revision::HEAD),_extra(),_ignore_contenttype(false),
                      _copies_as_adds(false),_git_diff_format(false)
        {
        }

        Path _tmpPath;
        Path _path1;
        Path _path2;
        Path _relativeTo;
        StringArray _changeList;
        bool _ignoreAncestry;
        bool _noDiffDeleted;
        Depth _depth;
        Revision _peg_revision;
        Revision _rev1;
        Revision _rev2;
        StringArray _extra;
        bool _ignore_contenttype;
        // subversion 1.7
        bool _copies_as_adds;
        bool _git_diff_format;
    };

    DiffParameter::DiffParameter()
    {
        _data = new DiffParameterData();
    }
    DiffParameter::~DiffParameter()
    {
        _data = 0;
    }

    GETSET(DiffParameter,Path,_path1,path1);
    GETSET(DiffParameter,Path,_path2,path2);
    GETSET(DiffParameter,Path,_tmpPath,tmpPath);
    GETSET(DiffParameter,Path,_relativeTo,relativeTo);
    GETSET(DiffParameter,Revision,_peg_revision,peg);
    GETSET(DiffParameter,Revision,_rev1,rev1);
    GETSET(DiffParameter,Revision,_rev2,rev2);
    GETSET(DiffParameter,StringArray,_changeList,changeList);
    GETSET(DiffParameter,StringArray,_extra,extra);

    GETSETSI(DiffParameter,Depth,_depth,depth);
    GETSETSI(DiffParameter,bool,_ignoreAncestry,ignoreAncestry);
    GETSETSI(DiffParameter,bool,_ignore_contenttype,ignoreContentType);
    GETSETSI(DiffParameter,bool,_noDiffDeleted,noDiffDeleted);
    GETSETSI(DiffParameter,bool,_copies_as_adds,copies_as_adds);
    GETSETSI(DiffParameter,bool,_git_diff_format,git_diff_format);

    struct StatusParameterData
    {
        StatusParameterData(const Path&path)
            :_path(path),_revision(Revision::UNDEFINED),_depth(DepthInfinity),_getAll(true),_update(true),_noIgnore(false),_ignoreExternals(false)
            ,_detailedRemote(false),_changeList()
        {
        }
        Path _path;
        Revision _revision;
        Depth _depth;
        bool _getAll;
        bool _update;
        bool _noIgnore;
        bool _ignoreExternals;
        bool _detailedRemote;
        StringArray _changeList;
    };

    StatusParameter::StatusParameter(const Path&path)
    {
        _data = new StatusParameterData(path);
    }
    StatusParameter::~StatusParameter()
    {
        _data = 0;
    }
    GETSET(StatusParameter,Path,_path,path);
    GETSET(StatusParameter,Revision,_revision,revision);
    GETSET(StatusParameter,StringArray,_changeList,changeList);

    GETSETSI(StatusParameter,Depth,_depth,depth);
    GETSETSI(StatusParameter,bool,_getAll,all);
    GETSETSI(StatusParameter,bool,_update,update);
    GETSETSI(StatusParameter,bool,_noIgnore,noIgnore);
    GETSETSI(StatusParameter,bool,_ignoreExternals,ignoreExternals);
    GETSETSI(StatusParameter,bool,_detailedRemote,detailedRemote);

    struct LogParameterData
    {
    public:
        LogParameterData()
            :_targets(),_ranges(),_peg(Revision::UNDEFINED),_limit(0),
            _discoverChangedPathes(false),_strictNodeHistory(true),_includeMergedRevisions(false),
            _revisionProperties(),_excludeList()
        {
        }
        Targets _targets;
        RevisionRanges _ranges;
        Revision _peg;
        int _limit;
        bool _discoverChangedPathes,_strictNodeHistory,_includeMergedRevisions;
        StringArray _revisionProperties;
        StringArray _excludeList;
    };

    LogParameter::LogParameter()
    {
        _data = new LogParameterData();
    }
    LogParameter::~LogParameter()
    {
        _data = 0;
    }

    GETSET(LogParameter,Targets,_targets,targets);
    GETSET(LogParameter,RevisionRanges,_ranges,revisions);
    GETSET(LogParameter,Revision,_peg,peg);
    GETSET(LogParameter,StringArray,_revisionProperties,revisionProperties);
    GETSET(LogParameter,StringArray,_excludeList,excludeList);

    GETSETSI(LogParameter,int,_limit,limit);
    GETSETSI(LogParameter,bool,_discoverChangedPathes,discoverChangedPathes);
    GETSETSI(LogParameter,bool,_strictNodeHistory,strictNodeHistory);
    GETSETSI(LogParameter,bool,_includeMergedRevisions,includeMergedRevisions);

    const RevisionRange&LogParameter::revisionRange()const
    {
        if (_data->_ranges.size()<1) {
            const static RevisionRange r(Revision::UNDEFINED,Revision::UNDEFINED);
            return r;
        }
        return _data->_ranges[0];
    }
    LogParameter&LogParameter::revisionRange(const Revision&start,const Revision&end)
    {
        _data->_ranges.clear();
        _data->_ranges.append(RevisionRange(start,end));
        return *this;
    }

    struct PropertiesParameterData
    {
        PropertiesParameterData()
            :_name(QString()),_value(QString()),_originalValue(QString()),
            _path(),_revision(Revision::UNDEFINED),_force(false),_depth(DepthEmpty),_skipCheck(false),_changeList(),_revProperties()
        {}
        QString _name;
        QString _value;
        QString _originalValue;
        Path _path;
        Revision _revision;
        bool _force;
        Depth _depth;
        bool _skipCheck;
        StringArray _changeList;
        PropertiesMap _revProperties;
    };

    PropertiesParameter::PropertiesParameter()
    {
        _data = new PropertiesParameterData();
    }
    PropertiesParameter::~PropertiesParameter()
    {
        _data = 0;
    }

    GETSET(PropertiesParameter,QString,_name,propertyName);
    GETSET(PropertiesParameter,QString,_value,propertyValue);
    GETSET(PropertiesParameter,QString,_originalValue,propertyOriginalValue);
    GETSET(PropertiesParameter,Path,_path,path);
    GETSET(PropertiesParameter,Revision,_revision,revision);
    GETSET(PropertiesParameter,StringArray,_changeList,changeList);
    GETSET(PropertiesParameter,PropertiesMap,_revProperties,revisionProperties);

    GETSETSI(PropertiesParameter,bool,_force,force);
    GETSETSI(PropertiesParameter,Depth,_depth,depth);
    GETSETSI(PropertiesParameter,bool,_skipCheck,skipCheck);

    struct MergeParameterData
    {

    public:
        MergeParameterData()
            :_path1(),_path2(),_localPath(),
             _peg(Revision::UNDEFINED),_ranges(),
             _force(false),_notice_ancestry(true),_dry_run(false),_record_only(false),_reintegrate(false),
             _depth(DepthInfinity),_merge_options()
        {}
        Path _path1,_path2,_localPath;
        Revision _peg;
        RevisionRanges _ranges;
        bool _force,_notice_ancestry,_dry_run,_record_only,_reintegrate;
        Depth _depth;
        StringArray _merge_options;
    };

    MergeParameter::MergeParameter()
    {
        _data = new MergeParameterData;
    }

    MergeParameter::~MergeParameter()
    {
        _data = 0;
    }

    GETSET(MergeParameter,Path,_path1,path1);
    GETSET(MergeParameter,Path,_path2,path2);
    GETSET(MergeParameter,Path,_localPath,localPath);
    GETSET(MergeParameter,Revision,_peg,peg);
    GETSET(MergeParameter,StringArray,_merge_options,merge_options);
    GETSET(MergeParameter,RevisionRanges,_ranges,revisions);

    GETSETSI(MergeParameter,bool,_force,force);
    GETSETSI(MergeParameter,bool,_notice_ancestry,notice_ancestry);
    GETSETSI(MergeParameter,bool,_dry_run,dry_run);
    GETSETSI(MergeParameter,bool,_record_only,record_only);
    GETSETSI(MergeParameter,Depth,_depth,depth);
    GETSETSI(MergeParameter,bool,_reintegrate,reintegrate);

    const RevisionRange&MergeParameter::revisionRange()const
    {
        if (_data->_ranges.size()<1) {
            const static RevisionRange r(Revision::UNDEFINED,Revision::UNDEFINED);
            return r;
        }
        return _data->_ranges[0];
    }
    MergeParameter&MergeParameter::revisionRange(const Revision&start,const Revision&end)
    {
        _data->_ranges.clear();
        _data->_ranges.append(RevisionRange(start,end));
        return *this;
    }
    const Revision&MergeParameter::revision1()const
    {
        return revisionRange().first;
    }
    const Revision&MergeParameter::revision2()const
    {
        return revisionRange().second;
    }

    struct CheckoutParameterData
    {
        CheckoutParameterData()
            :_moduleName(),_destination(),_revision(Revision::UNDEFINED),_peg(Revision::UNDEFINED),_depth(DepthInfinity),
             _ignoreExternals(false),_overWrite(false),_nativeEol(QString())
        {}
        Path _moduleName,_destination;
        Revision _revision,_peg;
        Depth _depth;
        bool _ignoreExternals,_overWrite;
        QString _nativeEol;
    };

    CheckoutParameter::CheckoutParameter()
    {
        _data = new CheckoutParameterData;
    }
    CheckoutParameter::~CheckoutParameter()
    {
        _data = 0;
    }

    GETSET(CheckoutParameter,Path,_moduleName,moduleName)
    GETSET(CheckoutParameter,Path,_destination,destination)
    GETSET(CheckoutParameter,Revision,_revision,revision)
    GETSET(CheckoutParameter,Revision,_peg,peg)
    GETSET(CheckoutParameter,QString,_nativeEol,nativeEol)

    GETSETSI(CheckoutParameter,Depth,_depth,depth)
    GETSETSI(CheckoutParameter,bool,_ignoreExternals,ignoreExternals)
    GETSETSI(CheckoutParameter,bool,_overWrite,overWrite)
}
