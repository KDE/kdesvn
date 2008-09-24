#ifndef _FRONTEND_TYPES_H
#define _FRONTEND_TYPES_H

#include "svnqt/shared_pointer.hpp"
#include <QList>

class ThreadContextListener;
class SvnItem;

typedef svn::smart_pointer<ThreadContextListener> ThreadContextListenerP;

typedef QList<SvnItem*> SvnItemList;
typedef QList<SvnItem*>::iterator SvnItemListIterator;
typedef QList<SvnItem*>::const_iterator SvnItemListConstIterator;

#endif

