kcoreaddons_add_plugin(ksvn_fileitemactions SOURCES kdesvnactionsplugin.cpp INSTALL_NAMESPACE kf${QT_MAJOR_VERSION}/kfileitemaction)
target_link_libraries(ksvn_fileitemactions KF${QT_MAJOR_VERSION}::I18n KF${QT_MAJOR_VERSION}::KIOWidgets)
