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




class SettingsPage {
private:
	PanelRaised rightPanel;

	PanelRaised labelBackground3;
	Label label3;
	
public:
	SettingsPage(char * subtitle)
	{
        init(subtitle);
	}

	virtual void populate(Dialog * dialog);
	virtual void unpopulate(Dialog * dialog);

	virtual void init(char * subtitle);

	// handle key presses mouse moves, etc. in this function 
	virtual void HandleEvent(Widget &w, int msg, int arg1=0, int arg2=0);/**/
};

#endif
