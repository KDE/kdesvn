#ifndef _KDESVN_PART_CONFIG_H
#define _KDESVN_PART_CONFIG_H

#include <qvariant.h>

class KConfig;
class KIconLoader;

class kdesvnPart_config
{
public:
    kdesvnPart_config(){}
    virtual ~kdesvnPart_config(){}

    static KConfig* config();
    static KIconLoader* iconLoader();
    static QVariant configItem(const QString& name);
};

#endif
