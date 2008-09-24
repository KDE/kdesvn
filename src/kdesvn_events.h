#ifndef __KDESVN_EVENTS_H
#define __KDESVN_EVENTS_H

#include <qevent.h>
#include "src/svnqt/svnqt_defines.hpp"

class FillCacheStatusEvent:public QEvent
{
    public:
        FillCacheStatusEvent(qlonglong current,qlonglong max);
        qlonglong current()const{return m_current;}
        qlonglong max()const{return m_max;}
    private:
        qlonglong m_current,m_max;
};

class DataEvent:public QEvent
{
    public:
        DataEvent(QEvent::Type);

        void setData(void*data);
        void*data()const;
    private:
        void*_data;
};

#endif
