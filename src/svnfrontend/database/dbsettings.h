#ifndef DB_SETTINGS_H
#define DB_SETTINGS_H

#include "ui_dbsettings.h"

class DbSettingsData;

class DbSettings:public QWidget,Ui::DbSettings
{
    Q_OBJECT
public:
    DbSettings(const QString&repository,QWidget *parent = 0, const char *name = 0);
    virtual ~DbSettings();

    void setRepository(const QString&repository);
    void store();

protected:
    void init();
private:
    DbSettingsData*_data;
};

#endif
