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

#include "tml.h"
//#include "game_config.h"

namespace tml
{
  /// \brief main game state
  lua_State* L;
  
  manager::manager()
  {
    L = lua_open();
    if(L==NULL)
      {
	tw_error("Unable to init Lua state. Something realy bad happend");
      }
    luaopen_base(L);
    luaopen_table(L);
    luaopen_io(L);
    luaopen_string(L);
    luaopen_math(L);
#ifdef _DEBUG
    luaopen_debug(L);
#endif
    
    lua_register(L, "Save",            tml::Save);
    lua_register(L, "Load",            tml::Load);
    lua_register(L, "Add_Event",       tml::Add_Event);
    lua_register(L, "Add_Star",        tml::Add_Star);
    lua_register(L, "Add_Sun",         tml::Add_Sun);
    lua_register(L, "Add_Planet",      tml::Add_Planet);
    lua_register(L, "Add_Ship",        tml::Add_Ship);

    lua_register(L, "Remove_Event",    tml::Remove_Event);
    lua_register(L, "Remove_Star",     tml::Remove_Star);
    lua_register(L, "Remove_Sun",      tml::Remove_Sun);
    lua_register(L, "Remove_Planet",   tml::Remove_Planet);
    lua_register(L, "Remove_Ship",     tml::Remove_Ship);

    lua_register(L, "To_HyperSpace",   tml::To_HyperSpace);
    lua_register(L, "To_StarSystem",   tml::To_StarSystem);
    lua_register(L, "To_Planet",       tml::To_Planet);
    lua_register(L, "To_PlanetLander", tml::To_PlanetLander);

    //lua_dofile(L, game_config::lua_init);
  }

  manager::~manager()
  {
    lua_close(L);
  }

  // Helpers
  std::string get_string(lua_State* ls, int narg)
  {
    if( !lua_isstring(ls, narg) )
      {
	tw_error("Get string failed");
      }
    return lua_tostring(ls, narg);
  }
  
  double get_double(lua_State* ls, int narg)
  {
    if( !lua_isnumber(ls, narg) )
      {
	tw_error("Get number failed");
      }
    return lua_tonumber(ls, narg);
  }
  bool get_boolean(lua_State* ls, int narg)
  {
    if( !lua_isboolean(ls, narg) )
      {
	tw_error("Get boolean failed");
      }
    return lua_toboolean(ls, narg);
  }

  // Global
  
  int Save(lua_State* ls)
  {
    return -1;
  }
  int Load(lua_State* ls)
  {
    return -1;
  }

  // TML Commands: these function called from TML script
  int Add_Event(lua_State* ls)
  {
    return -1;
  }
  int Add_Star(lua_State* ls)
  {
    return -1;
  }
  int Add_Sun(lua_State* ls)
  {
    return -1;
  }
  int Add_Planet(lua_State* ls)
  {
    return -1;
  }
  int Add_Ship(lua_State* ls)
  {
    return -1;
  }

  int Remove_Event(lua_State* ls)
  {
    return -1;
  }
  int Remove_Star(lua_State* ls)
  {
    return -1;
  }
  int Remove_Sun(lua_State* ls)
  {
    return -1;
  }
  int Remove_Planet(lua_State* ls)
  {
    return -1;
  }
  int Remove_Ship(lua_State* ls)
  {
    return -1;
  }

  int To_HyperSpace(lua_State* ls)
  {
    return -1;
  }
  int To_StarSystem(lua_State* ls)
  {
    return -1;
  }
  int To_Planet(lua_State* ls)
  {
    return -1;
  }
  int To_PlanetLander(lua_State* ls)
  {
    return -1;
  }

  //  std::map<int, TML_event_list> events;

  /// \brief add event to event list, it will be dispatched by pump()
  void add_event(int type, TMLEvent*)
  {
  }

  /// \brief causes events to be dispatched to all handler objects
  void pump()
  {
  }

}
