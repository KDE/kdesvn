#include "client_parameter.hpp"
#include "svnqt/svnqttypes.hpp"
#include "svnqt/stringarray.hpp"

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

    const Targets& CopyParameter::srcPath()const
    {
        return _data->_srcPath;
    }

    const Path&CopyParameter::destination()const
    {
        return _data->_destPath;
    }

    CopyParameter::~CopyParameter()
    {
        _data = 0;
    }

    CopyParameter&CopyParameter::asChild(bool value)
    {
        _data->_asChild=value;return *this;
    }
    bool CopyParameter::getAsChild()const
    {
        return _data->_asChild;
    }

    CopyParameter&CopyParameter::makeParent(bool value)
    {
        _data->_makeParent=value;return *this;
    }
    bool CopyParameter::getMakeParent()const
    {
        return _data->_makeParent;
    }

    CopyParameter&CopyParameter::force(bool value)
    {
        _data->_force=value;return *this;
    }
    bool CopyParameter::getForce()const
    {
        return _data->_force;
    }

    CopyParameter&CopyParameter::ignoreExternal(bool value)
    {
        _data->_ignoreExternal=value;return *this;
    }
    bool CopyParameter::getIgnoreExternal()const
    {
        return _data->_ignoreExternal;
    }

    CopyParameter&CopyParameter::srcRevision(const Revision&rev)
    {
        _data->_srcRevision=rev;return *this;
    }
    const Revision&CopyParameter::getSrcRevision()const
    {
        return _data->_srcRevision;
    }

    CopyParameter&CopyParameter::pegRevision(const Revision&rev)
    {
        _data->_pegRevision=rev;return *this;
    }
    const Revision&CopyParameter::getPegRevision()const
    {
        return _data->_pegRevision;
    }
    CopyParameter&CopyParameter::properties(const PropertiesMap&props)
    {
        _data->_properties=props;return *this;
    }
    const PropertiesMap&CopyParameter::getProperties()const
    {
        return _data->_properties;
    }

    struct SVNQT_NOEXPORT DiffParameterData
    {
    public:
        DiffParameterData()
            :_tmpPath(),_path1(),_path2(),_relativeTo(),_changeList(),_ignoreAncestry(false),_noDiffDeleted(false),
                      _depth(DepthInfinity),_peg_revision(Revision::UNDEFINED),
                      _rev1(Revision::START),_rev2(Revision::HEAD),_extra(),_ignore_contenttype(false)
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
    };


/*!
    \fn DiffParameter::getPath1()const
 */
const svn::Path& DiffParameter::getPath1()const
{
    return _data->_path1;
}


/*!
    \fn DiffParameter::getPath2()const
 */
const svn::Path& DiffParameter::getPath2()const
{
    return _data->_path2;
}


/*!
    \fn DiffParameter::getTmpPath()const
 */
const svn::Path& DiffParameter::getTmpPath()const
{
    return _data->_tmpPath;
}

/*!
    \fn DiffParameter::getRelativeTo()const
 */
const svn::Path& DiffParameter::getRelativeTo()const
{
    return _data->_relativeTo;
}


/*!
    \fn DiffParameter::getDepth()const
 */
svn::Depth DiffParameter::getDepth()const
{
    return _data->_depth;
}


/*!
    \fn DiffParameter::path1(const Path&path)
 */
DiffParameter& DiffParameter::path1(const svn::Path&path)
{
    _data->_path1 = path;
    return *this;
}


/*!
    \fn DiffParameter::path2(const Path&path)
 */
DiffParameter& DiffParameter::path2(const svn::Path&path)
{
    _data->_path2 = path;
    return *this;
}


/*!
    \fn DiffParameter::tmpPath(const Path&path)
 */
DiffParameter& DiffParameter::tmpPath(const svn::Path&path)
{
    _data->_tmpPath = path;
    return *this;
}

/*!
    \fn DiffParameter::relativeTo(const Path&path)
 */
DiffParameter& DiffParameter::relativeTo(const svn::Path&path)
{
    _data->_relativeTo = path;
    return *this;
}


/*!
    \fn DiffParameter::depth(Depth _depth)
 */
DiffParameter& DiffParameter::depth(svn::Depth _depth)
{
    _data->_depth=_depth;
    return *this;
}


/*!
    \fn DiffParameter::DiffParameter()
 */
 DiffParameter::DiffParameter()
{
    _data = new DiffParameterData();
}

DiffParameter::~DiffParameter()
{
    _data = 0;
}

/*!
    \fn DiffParameter::getIgnoreAncestry()const
 */
bool DiffParameter::getIgnoreAncestry()const
{
    return _data->_ignoreAncestry;
}


/*!
    \fn DiffParameter::getIgnoreContentType()const
 */
bool DiffParameter::getIgnoreContentType()const
{
    return _data->_ignore_contenttype;
}


/*!
    \fn DiffParameter::getNoDiffDeleted()const
 */
bool DiffParameter::getNoDiffDeleted()const
{
    return _data->_noDiffDeleted;
}


/*!
    \fn DiffParameter::getPeg()const
 */
const svn::Revision& DiffParameter::getPeg()const
{
    return _data->_peg_revision;
}


/*!
    \fn DiffParameter::getRev1()const
 */
const svn::Revision& DiffParameter::getRev1()const
{
    return _data->_rev1;
}


/*!
    \fn DiffParameter::getRev2()const
 */
const svn::Revision& DiffParameter::getRev2()const
{
    return _data->_rev2;
}


/*!
    \fn DiffParameter::getChangeList()const
 */
const svn::StringArray& DiffParameter::getChangeList()const
{
    return _data->_changeList;
}


/*!
    \fn DiffParameter::getExtra()const
 */
const svn::StringArray& DiffParameter::getExtra()const
{
    return _data->_extra;
}


/*!
    \fn DiffParameter::changeList(const svn::StringArray&changeList)
 */
DiffParameter& DiffParameter::changeList(const svn::StringArray&changeList)
{
    _data->_changeList=changeList;
    return *this;
}


/*!
    \fn DiffParameter::extra(const svn::StringArray&_extra)
 */
DiffParameter& DiffParameter::extra(const svn::StringArray&_extra)
{
    _data->_extra=_extra;
    return *this;
}


/*!
    \fn DiffParameter::ignoreAncestry(bool value)
 */
DiffParameter& DiffParameter::ignoreAncestry(bool value)
{
    _data->_ignoreAncestry=value;
    return *this;
}


/*!
    \fn DiffParameter::ignoreContentType(bool value)
 */
DiffParameter& DiffParameter::ignoreContentType(bool value)
{
    _data->_ignore_contenttype=value;
    return *this;
}


/*!
    \fn DiffParameter::peg_revision(const svn::Revision&_rev)
 */
DiffParameter& DiffParameter::peg(const svn::Revision&_rev)
{
    _data->_peg_revision=_rev;
    return *this;
}


/*!
    \fn DiffParameter::rev1(const svn::Revision&_rev)
 */
DiffParameter& DiffParameter::rev1(const svn::Revision&_rev)
{
    _data->_rev1=_rev;
    return *this;
}


/*!
    \fn DiffParameter::rev2(const svn::Revision&_rev)
 */
DiffParameter& DiffParameter::rev2(const svn::Revision&_rev)
{
    _data->_rev2=_rev;
    return *this;
}


/*!
    \fn DiffParameter::noDiffDeleted(bool value)
 */
DiffParameter& DiffParameter::noDiffDeleted(bool value)
{
    _data->_noDiffDeleted=value;
    return *this;
}

}
