/** 
SettingsPage.h
$Id$

  Defines the SettingsPage class, which can add a bunch of configuration
  settings to an existing dialog.  
  
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

#include "SettingsPage.h"
#include "GameSessionConfiguration.h"

void GameSettingsPage::HandleEvent(Widget &obj, int msg, int arg1, int arg2) {
	switch (msg) {
		case MSG_ACTIVATE: 
			break;
	}
}


void GameSettingsPage::init()
{
  rightPanel.Shape( 44,11,55,76, true);

  labelBackground3.Shape(45,12,53,5,true);
  
  char title[MAX_TITLE_STRING_SIZE];

  strcpy(title, gametypeName);
  strcat(title, " Settings");
  label3.SetText(title);
  label3.Shape(46,12,51,5,true);
}
 


void GameSettingsPage::populate(Dialog * dialog) {
  dialog->Add(rightPanel);
  dialog->Add(labelBackground3);
  dialog->Add(label3);
}

void GameSettingsPage::unpopulate(Dialog * dialog) {
  dialog->Remove(rightPanel);
  dialog->Remove(labelBackground3);
  dialog->Remove(label3);
}

GameSessionConfiguration * GameSettingsPage::getConfigInstance() {
	return new GameSessionConfiguration(gametypeName);
}


void MeleeSettingsPage::init() {
	GameSettingsPage::init();

	lMaxPlayers.Shape(46,17,25,5,true);
	lMaxPlayers.SetText("Max players");
	maxPlayers.Setup(71,17,15,5,0,0,(int)1,(int)20,(int)2,(int)1);
	maxPlayers.Shape(71,17,15,5,true);

	lNumberOfHumans.Shape(46,23,25,5,true);
	lNumberOfHumans.SetText("Human Players");
	numberOfHumans.Setup(71,23,15,5,0,0,(int)1,(int)20,(int)2,(int)1);
	numberOfHumans.Shape(71,23,15,5,true);

	padSlotsWithBots.SetText("Pad slots with Bots?");
	padSlotsWithBots.Shape(46,28,25,5,true);
}


void MeleeSettingsPage::populate(Dialog * dialog) {
	GameSettingsPage::populate(dialog);

	dialog->Add(lMaxPlayers);
	dialog->Add(maxPlayers);
	
	dialog->Add(lNumberOfHumans);
	dialog->Add(numberOfHumans);

	dialog->Add(padSlotsWithBots);
}

void MeleeSettingsPage::unpopulate(Dialog * dialog) {
	GameSettingsPage::unpopulate(dialog);
	
	dialog->Remove(lMaxPlayers);
	dialog->Remove(maxPlayers);
	
	dialog->Remove(lNumberOfHumans);
	dialog->Remove(numberOfHumans);

	dialog->Remove(padSlotsWithBots);
}


void MeleeSettingsPage::HandleEvent(Widget &w, int msg, int arg1, int arg2) {
	GameSettingsPage::HandleEvent(w,msg,arg1,arg2);

    switch (msg) {
		case MSG_SCROLL: 

			// make sure that the number of human players doesn't exceed the
			// total number of players.
			if ( (w==numberOfHumans) ||
				 (w==maxPlayers) ) 
			{
				
				if (numberOfHumans.GetPosition() > maxPlayers.GetPosition()) {
					numberOfHumans.SetPosition(maxPlayers.GetPosition());
				}
			}
		break;
	}/**/
	
}


GameSessionConfiguration * MeleeSettingsPage::getConfigInstance() {
	return new MeleeSessionConfiguration(gametypeName);
}









