/* $Id$ */ 
#include <allegro.h>

#ifdef ALLEGRO_MSVC
	#pragma warning (disable:4786)
#endif

#include "../melee.h"
REGISTER_FILE
#include "../melee/mship.h"
//#include "../games/ggob.h"
#include "gevent.h"


EventShipDie::EventShipDie()
{
	victim        = NULL; // ship
	player_killer = NULL;
}

unsigned int EventShipDie::GetEventType()
{
	return GAME_EVENT_SHIP_DIE;
};


unsigned int EventShipGetDamage::GetEventType()
{
	return GAME_EVENT_SHIP_GET_DAMAGE;
}

unsigned int EventEnterStation::GetEventType()
{
	return GAME_EVENT_ENTER_STATION;
}

unsigned int EventCleanQuestTrash::GetEventType()
{
	return GAME_EVENT_CLEANQUESTTRASH;
}

unsigned int EventQuestSuccess::GetEventType()
{
	return GAME_EVENT_QUESTSUCCESS;
}

unsigned int EventQuestFailed::GetEventType()
{
	return GAME_EVENT_QUESTFAILED;
}

unsigned int EventAskForQuest::GetEventType()
{
	return GAME_EVENT_ASKFORQUEST;
}

unsigned int EventPlayerDied::GetEventType()
{
	return GAME_EVENT_PLAYER_DIED;
}

EventListner::EventListner()
{
}
/*! \brief Remove self from various EventHandlers */
EventListner::~EventListner()
{
  for ( EventHandlerList::iterator Ihl = event_handlers.begin();
	Ihl != event_handlers.end(); Ihl++)
    {
      (*Ihl)->UnRegisterEvent( GAME_EVENT_ALL, this );
    }
}
/*! \brief Used by EventHandler destructor */
void EventListner::RemoveEventHandler( EventHandler * eh )
{
  eh->UnRegisterEvent ( GAME_EVENT_ALL, this );
  event_handlers.erase(eh);
}

void EventListner::RegisterEvent( int type, EventHandler * eh )
{
	eh->RegisterEvent ( type, this );
	event_handlers.insert( eh );
}

EventHandler::~EventHandler()
{
  EventListnersList temp = listners;
  
  for (EventListnersList::iterator ill = temp.begin();
       ill != temp.end(); ill++)
    {
      ill->first->RemoveEventHandler( this );
    }
}
/*! \Trigger event
  \param event Information about event
 */
void EventHandler::GenerateEvent ( IEvent* event )
{
  int type = event->GetEventType();
  
  for ( EventListnersList::iterator ill = listners.begin();
	ill != listners.end(); ill++ )
    {
		if ( std::find ( (ill->second).begin(), 
		  (ill->second).end(), 
		  type ) != (ill->second).end() )
	{
	  (ill->first)->ProcessEvent ( event );
	}
    }
}
/*! \brief Add listner
  \param type Event type listner want to be get
  \param listner pointer
*/
void EventHandler::RegisterEvent ( int type, EventListner * listner )
{
  std::set<int> s = listners[listner];
  s.insert(type);
  listners.erase(listner);
  listners[listner] = s;
};

/*! \brief Remove listner
  \param type Event type listner doesn't want to get
  \param listner pointer
*/
void EventHandler::UnRegisterEvent ( int type, EventListner * listner )
{
  std::set<int> s = listners[listner];
  if ( type == GAME_EVENT_ALL )
    {
      listners.erase(listner);
    }
  else
    {
      s.erase( type );
      listners.erase(listner);
      listners[listner] = s;
    }
}
