#include "kdesvn_events.h"
#include "eventnumbers.h"

FillCacheStatusEvent::FillCacheStatusEvent(qlonglong current,qlonglong max)
    :QEvent(EVENT_LOGCACHE_STATUS),m_current(current),m_max(max)
{
}

DataEvent::DataEvent(QEvent::Type _type)
    :QEvent(_type)
{
    _data=0;
}

void DataEvent::setData(void*adata)
{
    _data=adata;
}

void* DataEvent::data()const
{
    return _data;
}
