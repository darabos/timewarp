/** 
  OptionsMenu.cpp
  $Id$

  Class Implementation of the "Options" menu.  

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

#include <vector>
using namespace std;


#include "OptionsMenu.h"
#include "Interface.h"
using namespace Interface;


OptionsMenu::~OptionsMenu()
{
	currentPage->unpopulate(this);
	while (pages.size() >0) {
		delete pages.back();
		pages.pop_back();
	}
	pages.clear();
}

void OptionsMenu::HandleEvent(Widget &obj, int msg, int arg1, int arg2) {
	OverlayDialog::HandleEvent(obj, msg, arg1, arg2);
	
	switch (msg) {
		case MSG_ACTIVATE: 

			if (obj == video) {
				setSettingsPage(VIDEO_PAGE);
			}
			if (obj == audio) {
				setSettingsPage(AUDIO_PAGE);
			}
			if (obj == controls) {
				setSettingsPage(CONTROLS_PAGE);
			}
			if (obj == gameplay) {
				setSettingsPage(GAMEPLAY_PAGE);
			}

			if (obj == bQuit) {
			    _state = FOLLOW_PREV;
			}

			break;

		case MSG_REMOVEME:
			Remove(obj);
			break;

		case MSG_SCROLL:
			break;
	}
}


void OptionsMenu::init() {
	videoOptions = new VideoOptions();
	audioOptions = new AudioOptions();
	controlOptions = new ControlOptions();
	gameplayOptions = new GameplayOptions();
	

	pages.push_back(videoOptions);
	pages.push_back(audioOptions);
	pages.push_back(controlOptions);
	pages.push_back(gameplayOptions);
	
	setSettingsPage(0);
	
	//left side stuff -- select game type
	leftPanel.Shape(1, 11, 29, 33, true);
	
	labelBackground.Shape(1,1,32,8,true);
	label.Shape(1,1,32,7,true);
	label.SetText("Options");
	label.SetAlignment(2);//this means centered... MASKING should really enum this 
	
	bQuit.SetupNormalized(0, 90, 15, 10, KEY_B, D_EXIT, "&Back");
	
	descriptionText.Shape(15,90,70,10, true);
	descriptionText.SetText("");

	rightPanel.Shape( 44,11,55,76, true);


	video.SetupNormalized(3,13,25,7,KEY_V,0,"Video");
	audio.SetupNormalized(3,20,25,7,KEY_A,0,"Audio");
	gameplay.SetupNormalized(3,27,25,7,KEY_G,0,"Gameplay");
	controls.SetupNormalized(3,34,25,7,KEY_C,0,"Controls");


	
	Add(leftPanel);
	Add(labelBackground);
	Add(label);
	
	Add(video);
	Add(audio);
	Add(gameplay);
	Add(controls);
	
	Add(bQuit);
	Add(descriptionText);
}



void OptionsMenu::setSettingsPage(int page) {

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
