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
#ifndef __TML_H__
#define __TML_H__

#include <string>
#include <list>

extern "C" {
#	include "lua.h"
#	include "lualib.h"
#	include "lauxlib.h"
}

#include "events.h"

namespace tml
{
  struct manager
  {
    manager();
    ~manager();
  };

  // Helpers
  std::string get_string(lua_State* ls, int narg);
  double get_double(lua_State* ls, int narg);
  bool get_boolean(lua_State* ls, int narg);
  

  // Global
  
  int Save(lua_State* ls);
  int Load(lua_State* ls);

  // TML Commands: these function called from TML script
  int Add_Event(lua_State* ls);
  int Add_Event(lua_State* ls);
  int Add_Star(lua_State* ls);
  int Add_Sun(lua_State* ls);
  int Add_Planet(lua_State* ls);
  int Add_Ship(lua_State* ls);

  int Remove_Event(lua_State* ls);
  int Remove_Star(lua_State* ls);
  int Remove_Sun(lua_State* ls);
  int Remove_Planet(lua_State* ls);
  int Remove_Ship(lua_State* ls);

  int To_HyperSpace(lua_State* ls);
  int To_StarSystem(lua_State* ls);
  int To_Planet(lua_State* ls);
  int To_PlanetLander(lua_State* ls);

  // Events
  typedef std::list<TMLEvent*> TML_event_list;

  /// \brief add event to event list, it will be dispatched by pump()
  void add_event(int type, TMLEvent*);

  /// \brief causes events to be dispatched to all handler objects
  void pump();
};

#endif
