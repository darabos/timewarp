/** 
OptionsMenu.h
$Id$

  Class definition of the "Options" menu.  This allows the user to view
  and change any global environment settings.
  
  Revision history:
    2004.06.26 yb started
	
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

#ifndef OPTIONS_MENU_HEADER
#define OPTIONS_HEADER 1

#include "MASkinG.h"
using namespace MAS;

#include <vector>
using namespace std;

#include "GameSessionConfiguration.h"
#include "OverlayMenu.h"
#include "Interface.h"
using namespace Interface;

#include "../melee/mgame.h"
#include "SettingsPage.h"


class VideoOptions; class AudioOptions; class ControlOptions; class GameplayOptions;

class EnvironmentSettings;

class OptionsMenu : public OverlayDialog {

private:
	PanelRaised leftPanel;

	Button bQuit, bAccept;

	Button video, audio, gameplay, controls;

	Label label;
	PanelRaised labelBackground;
	
	TextArea descriptionText;

	PanelRaised rightPanel;
	TabPanel tabPanel;

	GameSettingsPage * currentPage;
	vector <GameSettingsPage *> pages;

	VideoOptions * videoOptions;
	AudioOptions * audioOptions;
	GameplayOptions * gameplayOptions;
	ControlOptions * controlOptions;

	enum { VIDEO_PAGE=0, AUDIO_PAGE, CONTROLS_PAGE, GAMEPLAY_PAGE};

public:

	OptionsMenu(GameSessionConfiguration**config, BITMAP *buffer, MenuDialogs prev) : 
	  OverlayDialog(config, buffer, prev),
	  currentPage(NULL)
	{
       init();
	}

    virtual ~OptionsMenu();


	/** */
	virtual void init(); 

	virtual void setSettingsPage(int page);

	/** handle key presses mouse moves, etc. in this function */
	virtual void HandleEvent(Widget &w, int msg, int arg1=0, int arg2=0);

	virtual void MsgIdle() { OverlayDialog::MsgIdle(); }
};


class EnvironmentSettings { 

public:
	EnvironmentSettings() : 
	  _screenResolutionWidth( DEFAULT_SCREEN_WIDTH ),
	  _screenResolutionHeight( DEFAULT_SCREEN_HEIGHT ),
	  _screenBitDepth( DEFAULT_SCREEN_BITDEPTH ),
	  _fullScreen( DEFAULT_FULLSCREEN ),
	  _gammaCorrection( DEFAULT_GAMMACORRECTION )
	{
	}

    /** reads in settings from a config file*/
	void load(const char * filename = "config.ini")
	{
	}

	/** saves settings to a config file*/
	void save(const char * filename = "config.ini")
	{
	}

	enum { DEFAULT_SCREEN_WIDTH=640 };
	enum { DEFAULT_SCREEN_HEIGHT=480 };
	enum { DEFAULT_SCREEN_BITDEPTH=32 };
	enum { DEFAULT_FULLSCREEN=0 };
	enum { DEFAULT_GAMMACORRECTION=128 };

private:
	unsigned _screenResolutionWidth, _screenResolutionHeight;
	unsigned _screenBitDepth;
	bool _fullScreen;
	unsigned short _gammaCorrection;

};


#endif