/** 
Interface.h
$Id$

  Some common functions and definitions for handling the menu interface.  In particular, 
  stuff related to drawing the background when the menus are shown.
  
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

#ifndef INTERFACE_HEADER
#define INTERFACE_HEADER 1

#include "MASkinG.h"
using namespace MAS;

#include <vector>
using namespace std;

namespace Interface {
	enum { NSTARS=200 };
	enum MenuDialogs { QUIT=0, MAIN_MENU, PLAY_LOCAL, PLAY_ONLINE, HOST_GAME, OPTIONS, START_GAME };

	enum OverlayDialogState { 
		/** remain in the current state, keep the dialog up, keep processing events*/
		IDLE=0, 

		/** hide this dialog, stop events, follow the "next" link to decide which dialog to bring up next*/
		FOLLOW_NEXT,

		/** hide this dialog, stop events, follow the "prev" link to decide which dialog to bring up next*/
		FOLLOW_PREV//,

		/** close this dialog, stop processing events, start the game defined in the dialog's
		    configuration. */
		//PLAY_GAME 
	}; 
};
using namespace Interface;


namespace Interface {

	// a simple star class
	class Star {
	protected:
		float x, y;
		float vy;
		MAS::Color col;
		
		void NewStar() {
			x = (float)(rand()%SCREEN_W);
			y = 0.0f;
			int l = rand()%128+128;
			vy = (float)(l)/128.0f;
			col = MAS::Color(l,l,l);
		}
		
	public:
		Star() {
			NewStar();
			y = (float)(rand()%SCREEN_H);
		}
		
		void Draw(BITMAP *buffer) {
			putpixel(buffer, (int)x, (int)y, col);
		}
		
		void Update() {
			y += vy;
			if ((int)y >= SCREEN_H) {
				NewStar();
			}
		}
	};

};


#endif