#ifndef __GEVENT_H__
#define __GEVENT_H__

#include <utility>
#include <functional>
#include <memory>
#include <list>
#include <string>
#include <map>
#include <set>
#include <iterator>
#include <algorithm>



class Ship;

enum EventTypes
  {
    GAME_EVENT_ALL = 0,
    GAME_EVENT_SHIP_DIE,
    GAME_EVENT_SHIP_GET_DAMAGE,
  };

class IEvent
{
 public:
  virtual unsigned int GetEventType() = 0;
};

class EventShipDie : public IEvent
{
  Ship * s;
 public:
	 virtual unsigned int GetEventType();
};

class EventShipGetDamage: public IEvent
{
  Ship * s;
  int total;
 public:
	 virtual unsigned int GetEventType();
};

class EventHandler;

typedef std::set<EventHandler*> EventHandlerList;

class EventListner
{
 protected:
  EventHandlerList event_handlers;
 public:
  EventListner();
  virtual ~EventListner();
  virtual void ProcessEvent( IEvent* event ) = 0;
  void RemoveEventHandler(EventHandler * eh);
  void RegisterEvent( int type, EventHandler * eh );
};

typedef std::map<EventListner*, std::set<int> > EventListnersList;

/*! \brief Gather event from various sources, and pass them to event listrens */
class EventHandler
{
 protected:
  EventListnersList listners;
 public:
  virtual ~EventHandler();
  virtual void GenerateEvent( IEvent* event);
  virtual void RegisterEvent( int type, EventListner * listner );
  virtual void UnRegisterEvent (int type, EventListner * listner );
};



#endif // __GEVENT_H__
