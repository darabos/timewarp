/** 
  MainMenu.cpp
  $Id$

  Class Implementation of the main menu.  

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

#include "MASkinG.h"
using namespace MAS;

#include "MainMenu.h"
#include "Interface.h"
using namespace Interface;



void MyMainMenu::HandleEvent(Widget &obj, int msg, int arg1, int arg2) {
	OverlayDialog::HandleEvent(obj, msg, arg1, arg2);
	
	switch (msg) {
		case MSG_ACTIVATE: 

			if (obj == bPlayLocal) {
				_next = PLAY_LOCAL;
				_state = FOLLOW_NEXT;
			}
					
			if (obj == bPlayOnline) {
				_next = PLAY_ONLINE;
			    _state = FOLLOW_NEXT;
			}
					
			if (obj == bHostGame) {
				_next = HOST_GAME;
			    _state = FOLLOW_NEXT;
			}
					
			if (obj == bOptions) {
				_next = OPTIONS;
			    _state = FOLLOW_NEXT;
			}
					
			if (obj == bQuit) {
				_next = QUIT;
				_state = FOLLOW_NEXT;
			}		
	}
}

void MyMainMenu::init() {
  int x = 10, 
	  y = 10;
  
  panel.Shape(x, y, 32, 52, true);
  
  x++; 
  y++;
  
  bPlayLocal.SetupNormalized(x, y, 30, 10, KEY_P, D_EXIT, "&Play Local");
  bPlayOnline.SetupNormalized(x, y+=10, 30, 10, KEY_L, 0, "Play On&line");
  bHostGame.SetupNormalized(x, y+=10, 30, 10, KEY_H, 0, "&Host Game");
  bOptions.SetupNormalized(x, y+=10, 30, 10, KEY_O, 0, "&Options");
  bQuit.SetupNormalized(x, y+=10, 30, 10, KEY_Q, D_EXIT, "&Quit Game");
  
  Add(panel);
  Add(bPlayLocal);
  Add(bPlayOnline);
  Add(bHostGame);
  Add(bOptions);
  Add(bQuit);
}
