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

#include "OverlayMenu.h"
#include "Interface.h"
using namespace Interface;

#include "../melee/mgame.h"
#include "SettingsPage.h" //remove


class PlayLocalMenu : public OverlayDialog {

private:
	PanelRaised leftPanel;
	ListBox gametypeList;
	Button bQuit;
	Label label, label2;
	PanelRaised labelBackground, labelBackground2;
	Button bStart;
	
	TextArea descriptionText;
	
	SettingsPage * currentPage;
	vector <SettingsPage *> pages;

public:

	PlayLocalMenu(BITMAP *buffer, MenuDialogs prev) : 
	  OverlayDialog(buffer, prev),
	  currentPage(NULL)
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

		while (pages.size() >0) {
			delete pages.back();
		}
		pages.clear();
	}


	/** */
	virtual void init(); 

	/** handle key presses mouse moves, etc. in this function */
	virtual void HandleEvent(Widget &w, int msg, int arg1=0, int arg2=0);

	virtual void setSettingsPage(int page);
	virtual void MsgIdle() { OverlayDialog::MsgIdle(); }
};


#endif