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
	MyMainMenu(BITMAP *buffer, MenuDialogs prev) : 
	  OverlayDialog(buffer, prev)
	  {
		  _state = IDLE;
		  int x=10, 
			  y = 10;
		  
		  panel.Shape(x, y, 32, 52, true);
		  
		  x++; 
		  y++;
		  
		  bPlayLocal.SetupNormalized(x, y, 30, 10, KEY_P, D_EXIT, "&Play Local");
		  bPlayOnline.SetupNormalized(x, y+=10, 30, 10, KEY_L, 0, "Play On&line");
		  bHostGame.SetupNormalized(x, y+=10, 30, 10, KEY_H, 0, "&Host Game");
		  bOptions.SetupNormalized(x, y+=10, 30, 10, KEY_O, 0, "&Options");
		  bQuit.SetupNormalized(x, y+=10, 30, 10, KEY_Q, D_EXIT, "&Quit");
		  
		  Add(panel);
		  Add(bPlayLocal);
		  Add(bPlayOnline);
		  Add(bHostGame);
		  Add(bOptions);
		  Add(bQuit);
	  }

	  void SelectDriver() { OverlayDialog::SelectDriver(); }
	  
	  // handle key presses mouse moves, etc. in this function
	  void HandleEvent(Widget &w, int msg, int arg1=0, int arg2=0);
	  
	  /** creates a main menu, displays it, and makes it interactive. */
	  void doMainMenu();

	  /** */
	  //MainMenuState getState() { return state; }

	  /** */
	  friend void doMyEngine();
	  friend MyUpdateDriver;


};

//}//namespace MainMenu

#endif