#include "client_parameter.hpp"

namespace svn
{
    //! internal data structure
    struct SVNQT_NOEXPORT CopyParameterData
    {
        CopyParameterData()
            :_srcPath(),_srcRevision(),_pegRevision(),_destPath(),_asChild(false),_makeParent(false),_ignoreExternal(false),_properties()
        {
        }
        Targets  _srcPath;
        Revision _srcRevision;
        Revision _pegRevision;
        Path _destPath;
        bool _asChild;
        bool _makeParent;
        bool _ignoreExternal;
        PropertiesMap _properties;
    };

    CopyParameter::CopyParameter(const Targets&_srcPath,const Path&_destPath)
    {
        _data = new CopyParameterData();
        _data->_srcPath = _srcPath;
        _data->_destPath = _destPath;
    }

    const Targets& CopyParameter::targets()const
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

    CopyParameter&CopyParameter::asChild()
    {
        _data->_asChild=true;return *this;
    }
    CopyParameter&CopyParameter::notAsChild()
    {
        _data->_asChild=false;return *this;
    }
    bool CopyParameter::getAsChild()const
    {
        return _data->_asChild;
    }

    CopyParameter&CopyParameter::makeParent()
    {
        _data->_makeParent=true;return *this;
    }
    CopyParameter&CopyParameter::notMakeParent()
    {
        _data->_makeParent=false;return *this;
    }
    bool CopyParameter::getMakeParent()const
    {
        return _data->_makeParent;
    }

    CopyParameter&CopyParameter::ignoreExternal()
    {
        _data->_ignoreExternal=true;return *this;
    }
    CopyParameter&CopyParameter::notIgnoreExternal()
    {
        _data->_ignoreExternal=false;return *this;
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
