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

#include "events.h"

  
TMLEvent_ShipDied::TMLEvent_ShipDied(SpaceLocation& who, SpaceLocation* source): TMLEvent(EVENT_SHIP_DIED), who_(who), source_(source)
{
}

TMLEvent_ShipTakeDamage::TMLEvent_ShipTakeDamage(SpaceLocation& who): TMLEvent(EVENT_SHIP_TAKE_DAMAGE), who_(who)
{
}


TMLEvent_ShipColide::TMLEvent_ShipColide(SpaceLocation& who, SpaceLocation& with): TMLEvent(EVENT_SHIP_COLIDE), who_(who), with_(with)
{
}

TMLEvent_ShipShot::TMLEvent_ShipShot(SpaceLocation& who, Shot& shot): TMLEvent(EVENT_SHIP_SHOT), who_(who), shot_(shot)
{
}
