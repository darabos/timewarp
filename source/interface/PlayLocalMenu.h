/** 
PlayLocalMenu.h
$Id$

  Class definition of the "PlayLocal" menu.  This allows the user to configure
  an individual playing session, with the constraint that only local players
  may join the game.
  
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

#ifndef PLAY_LOCAL_MENU_HEADER
#define PLAY_LOCAL_HEADER 1

#include "MASkinG.h"
using namespace MAS;

#include "Interface.h"
using namespace Interface;

#include "../melee/mgame.h"


class PlayLocalMenu : public OverlayDialog {

private:
	PanelRaised leftPanel;
	ListBox gametypeList;
	Button bQuit;
	Label label, label2, label3;
	PanelRaised labelBackground, labelBackground2, labelBackground3;
	Button bOk;
	
	PanelRaised rightPanel;
	TextArea descriptionText;

public:

	PlayLocalMenu(BITMAP *buffer, MenuDialogs prev) : 
	  OverlayDialog(buffer, prev)
	{
       init();
	}

    virtual ~PlayLocalMenu() {
		while (gametypeList.Size() > 0) {
			if (gametypeList.GetItem(0) != NULL)
				delete gametypeList.GetItem(0);
			gametypeList.DeleteItem(0);
		}
		gametypeList.DeleteAllItems();
	}


	virtual void init() {

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
	  }
	  gametypeList.Sort();
  
	  bQuit.SetupNormalized(0, 90, 15, 10, KEY_B, D_EXIT, "&Back");
	  bOk.SetupNormalized(85, 90, 15, 10, KEY_S, D_EXIT, "&Start Game");


	  descriptionText.Shape(15,90,70,10, true);
	  descriptionText.SetText("");

	  // right side stuff -- game settings
	  rightPanel.Shape( 44,11,55,76, true);
	  
	  labelBackground3.Shape(45,12,28,5,true);
	  label3.Shape(46,12,27,5,true);
	  label3.SetText("Game Settings:");
	  label3.SetAlignment(2);

  
	  Add(leftPanel);
	  Add(rightPanel);

	  Add(labelBackground);
	  Add(label);

	  Add(labelBackground2);
	  Add(label2);

	  Add(labelBackground3);
	  Add(label3);


	  Add(gametypeList);
	  Add(bQuit);
	  Add(descriptionText);
	  Add(bOk);
	}

	/** handle key presses mouse moves, etc. in this function */
	virtual void HandleEvent(Widget &w, int msg, int arg1=0, int arg2=0);
};


#endif