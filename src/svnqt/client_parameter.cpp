#include "client_parameter.hpp"

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

}
