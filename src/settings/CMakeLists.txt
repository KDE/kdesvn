
KDE3_ADD_KCFG_FILES(cfgsrc
  kdesvnsettings.kcfgc)
ADD_LIBRARY(kdesvncfgreader STATIC ${cfgsrc})

KDE3_ADD_UI_FILES(
    settings_ui_sources
	dispcolor_settings.ui
    display_settings.ui
    revisiontree_settings.ui
    subversion_settings.ui
)

SET(settings_sources
    dispcolorsettings_impl.cpp
    displaysettings_impl.cpp
    revisiontreesettingsdlg_impl.cpp 
    subversionsettings_impl.cpp)

KDE3_AUTOMOC(${settings_sources})
ADD_LIBRARY(settingsdlgs STATIC ${settings_ui_sources} ${settings_sources})
ADD_DEPENDENCIES(settingsdlgs kdesvncfgreader)
