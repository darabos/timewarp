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

void SettingsPage::HandleEvent(Widget &obj, int msg, int arg1, int arg2) {
	switch (msg) {
		case MSG_ACTIVATE: 
			break;
	}
}


void SettingsPage::init(char * subtitle)
{
  enum { MAX_TITLE_STRING_SIZE=100 };
  char title[MAX_TITLE_STRING_SIZE];

  strncpy(title, subtitle, MAX_TITLE_STRING_SIZE);
  strcat(title, " Settings");

  rightPanel.Shape( 44,11,55,76, true);

  labelBackground3.Shape(45,12,53,5,true);
  label3.Shape(46,12,51,5,true);
  label3.SetText(title);

}


void SettingsPage::populate(Dialog * dialog) {
  dialog->Add(rightPanel);
  dialog->Add(labelBackground3);
  dialog->Add(label3);
}

void SettingsPage::unpopulate(Dialog * dialog) {
  dialog->Remove(rightPanel);
  dialog->Remove(labelBackground3);
  dialog->Remove(label3);
}
