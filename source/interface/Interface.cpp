/** 
Interface.cpp
$Id$

  Implementations of some common functions and definitions for handling the menu interface.  In particular, 
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


#include "Interface.h"
using namespace Interface;

#include "MainMenu.h"
#include "PlayLocalMenu.h"
#include "PlayOnlineMenu.h"
#include "OverlayMenu.h"



// the timer system variable and callback
static volatile int timer = 0;
static void timer_f() {
	++timer;
} END_OF_STATIC_FUNCTION(timer_f);


// tha Engine
void doMyEngine() {
	int i;
	
	// make the backbuffer for doublebuffering
	BITMAP * buffer = create_bitmap(SCREEN_W, SCREEN_H);
	
	// make some stars;
	Star stars[NSTARS];
	
	
	// make a new dialog and initialize it
	MyMainMenu * mainMenu = new MyMainMenu(buffer, QUIT);
	PlayLocalMenu * playLocalMenu = new PlayLocalMenu(buffer, MAIN_MENU);
	PlayOnlineMenu * playOnlineMenu = new PlayOnlineMenu(buffer, MAIN_MENU);

	OverlayDialog * currentMenu = mainMenu;

	// setup the timer system
	LOCK_VARIABLE(timer);
	LOCK_FUNCTION(timer_f);
	install_int_ex(timer_f, BPS_TO_TIMER(100));
	
	currentMenu->MsgStart();
	
	timer = 0;
	bool done = false;

	while (! done) {
		while (timer) {
			// do the engine logic
			for (i=0; i<NSTARS; i++) {
				stars[i].Update();
			}
			
			--timer;
		}
		
		// draw the engine
		clear(buffer);
		for (i=0; i<NSTARS; i++) {
			stars[i].Draw(buffer);
		}
		
		// update the GUI (will redraw itself)
		currentMenu->MsgIdle();

		// do the double buffering thingy
		blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

		if (currentMenu->getState() != IDLE) {

			MenuDialogs menuToSwitchTo = QUIT;

			if (currentMenu->getState() == FOLLOW_NEXT)
				menuToSwitchTo = currentMenu->getNext();

			if (currentMenu->getState() == FOLLOW_PREV)
				menuToSwitchTo = currentMenu->getPrev();

            currentMenu->MsgEnd();

			switch (menuToSwitchTo) {

			case MAIN_MENU:
				currentMenu = mainMenu;
				break;

			case PLAY_LOCAL:
				currentMenu = playLocalMenu;
				break;

			case PLAY_ONLINE:
				currentMenu = playOnlineMenu;
				break;

			case HOST_GAME:
			    break;

			case OPTIONS:
				break;

			case QUIT:
				done = true;
				break;

			default:
				;
			};

			if (menuToSwitchTo != QUIT)
				currentMenu->MsgStart();
		}
	}
	
	// deinitialize the dialog and delete it
	currentMenu->MsgEnd();
	
	delete mainMenu;
	
	// destory the backbuffer
	destroy_bitmap(buffer);
}


void doMainMenu() {
	Settings::Load("allegro.cfg");
	Settings::antialiasing = false;
	Settings::mouseShadow = false;
	strcpy(Settings::skinPath, "skins/Dinks.ini");
	//strcpy(Settings::skinPath, "skins/Selenium.ini");
	
	Color::OnColorDepthChange();
	alfont_init();
	theSkin = new Skin((char *)Settings::skinPath);
	if (!theSkin || theSkin->GetError()) {
		allegro_message("Error loading skin!\n");
		return;
	}
	
	// do the engine thingy
	doMyEngine();
	
	// clean up and exit (instead of calling ExitMASkinG())
	if (theSkin) {
		delete theSkin;
		theSkin = NULL;
	}
	alfont_exit();
}

