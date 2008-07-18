#include "kdesvn_events.h"
#include "eventnumbers.h"

FillCacheStatusEvent::FillCacheStatusEvent(Q_LLONG current,Q_LLONG max)
    :QCustomEvent(EVENT_LOGCACHE_STATUS),m_current(current),m_max(max)
{
}
