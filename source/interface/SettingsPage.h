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

#ifndef SETTINGS_PAGE_HEADER
#define SETTINGS_PAGE_HEADER 1

#include "MASkinG.h"
using namespace MAS;

#include <vector>
using namespace std;

#include "OverlayMenu.h"
#include "Interface.h"


enum { MAX_TITLE_STRING_SIZE=100 };



/** Defines how a page should be laid out to configure a particular gametype. */
class SettingsPage {

public:
	
	/** Constructor.  Creates a new page to configure a gametype.
	    @param gametype the name of the gametype. */
	SettingsPage(const char * gametype)
	{
		strcpy(gametypeName, gametype);
        init();
	}

	/** add any graphical components to the specified diagram. */
	virtual void populate(Dialog * dialog);

	/** remove any graphical components from the specified diagram.  */ 
	virtual void unpopulate(Dialog * dialog);

	/** prepare how any graphical components should be laid out.  */
	virtual void init();

	/** handle key presses mouse moves, etc. in this function  */
	virtual void HandleEvent(Widget &w, int msg, int arg1=0, int arg2=0);/**/

	/** Creates and returns a new configuration based on the settings contained
	    in this class.  */
	virtual GameSessionConfiguration * getConfigInstance();

private:
	PanelRaised rightPanel;

	PanelRaised labelBackground3;
	Label label3;

protected:
	char gametypeName[MAX_TITLE_STRING_SIZE];
	
};


/** Settings for the Melee gametype.  */
class MeleeSettingsPage : public SettingsPage { 

protected:
	Label lMaxPlayers, lNumberOfHumans, lPadSlotsWithBots;
	
	//TODO a known issue is that if you enter an undefined value in the text area,
	// then hit one of the scroll buttons, a crash occurs.
	SpinBox maxPlayers, numberOfHumans;
	CheckBox padSlotsWithBots;

public:

	MeleeSettingsPage(const char * gametype) :
	  SettingsPage(gametype)
	{
        this->init();
	}

	virtual void populate(Dialog * dialog);
	virtual void unpopulate(Dialog * dialog);

	virtual void init();

	virtual void HandleEvent(Widget &w, int msg, int arg1=0, int arg2=0);/**/

	virtual GameSessionConfiguration * getConfigInstance();
};



#endif
