/** 
PlayOnlineMenu.h
$Id$

  Class definition of the "PlayOnline" menu.  This allows the user to choose
  from various online games already in session.  At present, this will take place
  with the user entering the host IP, but someday, it would be very nice to have it
  instead use a server browser, with a master server which is always listing active
  game servers.  
  
  Revision history:
    2004.06.16 yb started
	
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

#ifndef PLAY_ONLINE_MENU_HEADER
#define PLAY_ONLINE_MENU_HEADER 1

#include "MASkinG.h"
using namespace MAS;

#include "Interface.h"
using namespace Interface;



class PlayOnlineMenu : public OverlayDialog {
private:
	PanelRaised panel;
	ListBox gametypeList;
	Button bQuit;
	
public:

	PlayOnlineMenu(BITMAP *buffer, MenuDialogs prev) : 
	  OverlayDialog(buffer, prev)
	{
	  init();
	}

	virtual void init()
	{
	  int x = 10, 
		  y = 10;
  
	  panel.Shape(x, y, 32, 52, true);
  
	  x++;
	  y++;
  
	  gametypeList.Shape(x, y, 32, 30, true);
  
	  gametypeList.InsertItem( new ListItemString("Melee1") );
	  gametypeList.InsertItem( new ListItemString("Melee2") );
	  gametypeList.InsertItem( new ListItemString("Melee3") );
  
	  bQuit.SetupNormalized(x, y+=10, 30, 10, KEY_Q, D_EXIT, "&Quit");
  
	  Add(bQuit);
	}

	/** handle key presses mouse moves, etc. in this function */
	virtual void HandleEvent(Widget &w, int msg, int arg1=0, int arg2=0);
};

#endif