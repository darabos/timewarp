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

#include "PlayLocalMenu.h"
#include "MainMenu.h" // remove
#include "Interface.h"
using namespace Interface;

void PlayLocalMenu::HandleEvent(Widget &obj, int msg, int arg1, int arg2) {
	OverlayDialog::HandleEvent(obj, msg, arg1, arg2);
	
	switch (msg) {
		case MSG_ACTIVATE: 

			if (obj == gametypeList) {				
				descriptionText.SetText(gametypeList.GetSelectedItem()->GetText());

				switch (gametypeList.Selection()) {
				default:
					;
				}
			}
					
			if (obj == bQuit) {
			    _state = FOLLOW_PREV;
			}

			break;

		case MSG_REMOVEME:
			Remove(obj);
			break;

/*		case MSG_TICK:
		case MSG_TIMER:
			;

		default:
			int td = 3;*/
	}
}
