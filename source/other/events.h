/* $Id$ */ 
/*
This file is part of "Star Control: TimeWarp" 
                    http://timewarp.sourceforge.net/
Copyright (C) 2001-2004  TimeWarp development team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef __TML_EVENTS_H__
#define __TML_EVENTS_H__

#include "melee/mshot.h"

enum TML_event{
  EVENT_SHIP_DIED = 1,
  EVENT_SHIP_TAKE_DAMAGE,
  EVENT_SHIP_COLIDE,
  EVENT_SHIP_SHOT
};

class TMLEvent
{
 public:
  int type_;
  
  TMLEvent(int type) : type_(type){};
};

class TMLEvent_ShipDied: public TMLEvent
{
 public:
  TMLEvent_ShipDied(SpaceLocation& who, SpaceLocation* source);
  
  SpaceLocation& who_;
  SpaceLocation* source_;
};

class TMLEvent_ShipTakeDamage: public TMLEvent
{
 public:
  TMLEvent_ShipTakeDamage(SpaceLocation& who);
  
  SpaceLocation& who_;
};
class TMLEvent_ShipColide: public TMLEvent
    {
    public:
      TMLEvent_ShipColide(SpaceLocation& who, SpaceLocation& with);
      
      SpaceLocation& who_;
      SpaceLocation& with_;
    };
class TMLEvent_ShipShot: public TMLEvent
{
 public:
  TMLEvent_ShipShot(SpaceLocation& who, Shot& shot);
  
  SpaceLocation& who_;
  Shot& shot_;
};

#endif // __TML_EVENTS_H__
