/** 
  PlayLocalMenu.cpp
  $Id$

  Class Implementation of the "Play Local" menu.  

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

#include "../scp.h"

#include "PlayLocalMenu.h"
#include "Interface.h"
using namespace Interface;

void PlayLocalMenu::HandleEvent(Widget &obj, int msg, int arg1, int arg2) {
	OverlayDialog::HandleEvent(obj, msg, arg1, arg2);
	
	switch (msg) {
		case MSG_ACTIVATE: 

			if (obj == gametypeList) {
				descriptionText.SetText(gametypeList.GetSelectedItem()->GetText());
				setSettingsPage(gametypeList.Selection());
			}
					
			if (obj == bStart) {
				// start the game
				_state = FOLLOW_PREV;
				_next = QUIT;
				play_single("Melee");
			}
			
			if (obj == bQuit) {
			    _state = FOLLOW_PREV;
			}

			break;

		case MSG_REMOVEME:
			Remove(obj);
			break;
	}
	if (currentPage != NULL)
		currentPage->HandleEvent(obj, msg, arg1, arg2);
}


void PlayLocalMenu::init() {
	
	//left side stuff -- select game type
	leftPanel.Shape(1, 11, 42, 76, true);
	
	labelBackground.Shape(1,1,32,8,true);
	label.Shape(1,1,32,7,true);
	label.SetText("Play Local");
	label.SetAlignment(2);//this means centered... MASKING should really enum this 
	
	labelBackground2.Shape(2,12,28,5,true);
	label2.Shape(2,12,27,5,true);
	label2.SetText("Select Gametype:");
	label2.SetAlignment(2);
	
	gametypeList.Shape(2, 18, 40, 68, true);
	
	gametypeList.DeleteAllItems();
	for (int i=0; i<num_games; i++) {
		gametypeList.InsertItem( new ListItemString(game_names[i]),0 );
        pages.insert( pages.begin(), new SettingsPage(game_names[i]) );
	}
	//gametypeList.Sort();
	setSettingsPage(0);

	
	bQuit.SetupNormalized(0, 90, 15, 10, KEY_B, D_EXIT, "&Back");
	bStart.SetupNormalized(85, 90, 15, 10, KEY_S, D_EXIT, "&Start Game");
	
	
	descriptionText.Shape(15,90,70,10, true);
	descriptionText.SetText("");
	
	Add(leftPanel);
	Add(labelBackground);
	Add(label);	
	Add(labelBackground2);
	Add(label2);
	Add(gametypeList);
	
	Add(bQuit);
	Add(descriptionText);
	Add(bStart);
}


void PlayLocalMenu::setSettingsPage(int page) {

	//check if the page is different, if it's the same one that's already
	// shown, return.
	int counter = 0;
	for (vector <SettingsPage *>::iterator i=pages.begin(); i!=pages.end(); i++) {
		if ( (currentPage == *i) &&
			 (counter == page) )
		{
			return;
		}
		counter ++;
	}

	if (currentPage != NULL)
		currentPage->unpopulate(this);
	currentPage = pages[page];
	pages[page]->populate(this);
}


