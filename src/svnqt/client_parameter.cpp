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
}


/*!
    \fn svn::DiffParameter::getPath1()const
 */
const svn::Path& svn::DiffParameter::getPath1()const
{
    return _data->_path1;
}


/*!
    \fn svn::DiffParameter::getPath2()const
 */
const svn::Path& svn::DiffParameter::getPath2()const
{
    return _data->_path2;
}


/*!
    \fn svn::DiffParameter::getTmpPath()const
 */
const svn::Path& svn::DiffParameter::getTmpPath()const
{
    return _data->_tmpPath;
}

/*!
    \fn svn::DiffParameter::getRelativeTo()const
 */
const svn::Path& svn::DiffParameter::getRelativeTo()const
{
    return _data->_relativeTo;
}


/*!
    \fn svn::DiffParameter::getDepth()const
 */
svn::Depth svn::DiffParameter::getDepth()const
{
    return _data->_depth;
}


/*!
    \fn svn::DiffParameter::path1(const Path&path)
 */
svn::DiffParameter& svn::DiffParameter::path1(const svn::Path&path)
{
    _data->_path1 = path;
    return *this;
}


/*!
    \fn svn::DiffParameter::path2(const Path&path)
 */
svn::DiffParameter& svn::DiffParameter::path2(const svn::Path&path)
{
    _data->_path2 = path;
    return *this;
}


/*!
    \fn svn::DiffParameter::tmpPath(const Path&path)
 */
svn::DiffParameter& svn::DiffParameter::tmpPath(const svn::Path&path)
{
    _data->_tmpPath = path;
    return *this;
}

/*!
    \fn svn::DiffParameter::relativeTo(const Path&path)
 */
svn::DiffParameter& svn::DiffParameter::relativeTo(const svn::Path&path)
{
    _data->_relativeTo = path;
    return *this;
}


/*!
    \fn svn::DiffParameter::depth(Depth _depth)
 */
svn::DiffParameter& svn::DiffParameter::depth(svn::Depth _depth)
{
    _data->_depth=_depth;
    return *this;
}


/*!
    \fn svn::DiffParameter::DiffParameter()
 */
 svn::DiffParameter::DiffParameter()
{
    _data = new DiffParameterData();
}

svn::DiffParameter::~DiffParameter()
{
    _data = 0;
}

/*!
    \fn svn::DiffParameter::getIgnoreAncestry()const
 */
bool svn::DiffParameter::getIgnoreAncestry()const
{
    return _data->_ignoreAncestry;
}


/*!
    \fn svn::DiffParameter::getIgnoreContentType()const
 */
bool svn::DiffParameter::getIgnoreContentType()const
{
    return _data->_ignore_contenttype;
}


/*!
    \fn svn::DiffParameter::getNoDiffDeleted()const
 */
bool svn::DiffParameter::getNoDiffDeleted()const
{
    return _data->_noDiffDeleted;
}


/*!
    \fn svn::DiffParameter::getPeg()const
 */
const svn::Revision& svn::DiffParameter::getPeg()const
{
    return _data->_peg_revision;
}


/*!
    \fn svn::DiffParameter::getRev1()const
 */
const svn::Revision& svn::DiffParameter::getRev1()const
{
    return _data->_rev1;
}


/*!
    \fn svn::DiffParameter::getRev2()const
 */
const svn::Revision& svn::DiffParameter::getRev2()const
{
    return _data->_rev2;
}


/*!
    \fn svn::DiffParameter::getChangeList()const
 */
const svn::StringArray& svn::DiffParameter::getChangeList()const
{
    return _data->_changeList;
}


/*!
    \fn svn::DiffParameter::getExtra()const
 */
const svn::StringArray& svn::DiffParameter::getExtra()const
{
    return _data->_extra;
}


/*!
    \fn svn::DiffParameter::changeList(const svn::StringArray&changeList)
 */
svn::DiffParameter& svn::DiffParameter::changeList(const svn::StringArray&changeList)
{
    _data->_changeList=changeList;
    return *this;
}


/*!
    \fn svn::DiffParameter::extra(const svn::StringArray&_extra)
 */
svn::DiffParameter& svn::DiffParameter::extra(const svn::StringArray&_extra)
{
    _data->_extra=_extra;
    return *this;
}


/*!
    \fn svn::DiffParameter::ignoreAncestry(bool value)
 */
svn::DiffParameter& svn::DiffParameter::ignoreAncestry(bool value)
{
    _data->_ignoreAncestry=value;
    return *this;
}


/*!
    \fn svn::DiffParameter::ignoreContentType(bool value)
 */
svn::DiffParameter& svn::DiffParameter::ignoreContentType(bool value)
{
    _data->_ignore_contenttype=value;
    return *this;
}


/*!
    \fn svn::DiffParameter::peg_revision(const svn::Revision&_rev)
 */
svn::DiffParameter& svn::DiffParameter::peg(const svn::Revision&_rev)
{
    _data->_peg_revision=_rev;
    return *this;
}


/*!
    \fn svn::DiffParameter::rev1(const svn::Revision&_rev)
 */
svn::DiffParameter& svn::DiffParameter::rev1(const svn::Revision&_rev)
{
    _data->_rev1=_rev;
    return *this;
}


/*!
    \fn svn::DiffParameter::rev2(const svn::Revision&_rev)
 */
svn::DiffParameter& svn::DiffParameter::rev2(const svn::Revision&_rev)
{
    _data->_rev2=_rev;
    return *this;
}


/*!
    \fn svn::DiffParameter::noDiffDeleted(bool value)
 */
svn::DiffParameter& svn::DiffParameter::noDiffDeleted(bool value)
{
    _data->_noDiffDeleted=value;
    return *this;
}
