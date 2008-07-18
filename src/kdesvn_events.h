#ifndef __KDESVN_EVENTS_H
#define __KDESVN_EVENTS_H

#include <qevent.h>
#include "src/svnqt/svnqt_defines.hpp"

class FillCacheStatusEvent:public QCustomEvent
{
    public:
        FillCacheStatusEvent(Q_LLONG current,Q_LLONG max);
        Q_LLONG current()const{return m_current;}
        Q_LLONG max()const{return m_max;}
    private:
        Q_LLONG m_current,m_max;
};

#endif
