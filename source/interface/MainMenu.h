/** 
MainMenu.h
$Id$

  Class definition of the main menu.  
  
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

#ifndef MAIN_MENU_HEADER
#define MAIN_MENU_HEADER 1

#include "MASkinG.h"
using namespace MAS;

#include "GameSessionConfiguration.h"
#include "OverlayMenu.h"
#include "Interface.h"
using namespace Interface;


// some forward declarations
class MyMainMenu;
class MyUpdateDriver;

void doMainMenu();


class MyMainMenu : public OverlayDialog {
private:
	PanelRaised panel;
	Button bPlayLocal, bPlayOnline, bHostGame, bOptions, bQuit;
	
public:
	MyMainMenu(GameSessionConfiguration**config, BITMAP *buffer, MenuDialogs prev) : 
	  OverlayDialog(config, buffer, prev)
	  {
		  init();
	  }

	  virtual void init(); 

	  virtual void SelectDriver() { OverlayDialog::SelectDriver(); }
	  
	  // handle key presses mouse moves, etc. in this function
	  virtual void HandleEvent(Widget &w, int msg, int arg1=0, int arg2=0);
	  
	  /** */
	  friend void doMyEngine();
	  friend MyUpdateDriver;


};

//}//namespace MainMenu

#endif