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

#include "Interface.h"


/** Allows GUI elements to be added to an existing MaSKINg Dialog. */
class SettingsPage {

protected:	
	Dialog * _parent;
	vector<Widget *> elements;

public:
	SettingsPage( Dialog * parent ) :
	  _parent(parent)
	{
		init();
	}

	/** subclasses should override this to populate and configure the
	    GUI elements they want added to the Dialog. */
	virtual void init() = 0;


};




#endif