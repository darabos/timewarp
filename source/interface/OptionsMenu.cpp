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

#include "Allegro.h" // for file-finding routines
#include "MASkinG.h"
using namespace MAS;

#include <vector>
using namespace std;



#include "OptionsMenu.h"
#include "Interface.h"
using namespace Interface;



class VideoOptions : public GameSettingsPage {
private:
	Label res;
	ComboBox resChoices;

	Label bitdepth;
	ComboBox bitdepthChoices;

	RadioButton fullscreen[2];

	Label skin;
	ComboBox skinChoices;


	Button resetToDefaults;

	enum { DEFAULT_RESOLUTION=1 };
	enum { DEFAULT_BITDEPTH=1 };
	enum { DEFAULT_FULLSCREEN=1 };
	enum { DEFAULT_SKIN=1 };

public:
	VideoOptions() :
	  GameSettingsPage("Video Options")
	{ this->init(); }

	virtual void HandleEvent(Widget &w, int msg, int arg1=0, int arg2=0) 
	{
		GameSettingsPage::HandleEvent(w, msg, arg1, arg2);
	
		switch (msg) {
			case MSG_ACTIVATE: 
				if (w==resetToDefaults) {
				}

				if (w==skinChoices) {
					strcpy(Settings::skinPath, "skins/");
					strcat(Settings::skinPath, skinChoices.list.GetSelectedItem()->GetText());
					Skin * aSkin = new Skin((char *)Settings::skinPath);
					if (!aSkin || aSkin->GetError()) {
						allegro_message("Error loading skin!\n");
						return;
					}
					theSkin = aSkin;
					w.GetParent()->SetSkin(theSkin);
				}
				break;
		}
	}

	virtual void init() 
	{ 
		GameSettingsPage::init(); 

		res.Setup(46,17,25,5,0,0,"Screen Resolution:");
		res.Shape(46,17,25,5,true);
		resChoices.list.InsertItem(new ListItemString("320x200"), 0);
		resChoices.list.InsertItem(new ListItemString("640x480"), 1);
		resChoices.list.InsertItem(new ListItemString("800x600"), 2);
		resChoices.list.InsertItem(new ListItemString("1024x768"), 3);
		resChoices.list.InsertItem(new ListItemString("1280x1024"), 4);
		resChoices.list.InsertItem(new ListItemString("1600x1200"), 5);
		resChoices.Setup(71,17,25,5,0,0,DEFAULT_RESOLUTION);
		resChoices.Shape(71,17,25,5,true);

		bitdepth.Setup(46,22,25,5,0,0,"Bit Depth:");
		bitdepth.Shape(46,22,25,5,true);
		bitdepthChoices.list.InsertItem(new ListItemString("8"), 0);
		bitdepthChoices.list.InsertItem(new ListItemString("16"), 1);
		bitdepthChoices.list.InsertItem(new ListItemString("24"), 2);
		bitdepthChoices.list.InsertItem(new ListItemString("32"), 3);
		bitdepthChoices.Setup(71,24,25,5,0,0,DEFAULT_BITDEPTH);
		bitdepthChoices.Shape(71,24,25,5,true);

		fullscreen[0].Setup(46,31,20,5,KEY_F,0,"Full Screen",0);
		fullscreen[0].Shape(46,31,20,5,true);
		fullscreen[1].Setup(66,31,20,5,KEY_W,D_SELECTED,"Windowed",0);
		fullscreen[1].Shape(66,31,20,5,true);
		
		if (DEFAULT_FULLSCREEN==1)
			this->HandleEvent(fullscreen[0],MSG_RADIO);
		else
			this->HandleEvent(fullscreen[1],MSG_RADIO);

		skin.Setup(46,37,15,5,0,0,"Interface Skin:");
		skin.Shape(46,37,15,5,true);

		scanForSkins();

		skinChoices.Setup(66,37,30,5,0,0,DEFAULT_SKIN);
		skinChoices.Shape(66,37,30,5,true);

		resetToDefaults.Setup(46,44,30,5,KEY_D,0,"Reset To Defaults");
		resetToDefaults.Shape(46,44,30,5,true);
	}


	virtual void populate(Dialog * dialog) 
	{ 
		GameSettingsPage::populate(dialog); 

		dialog->Add(fullscreen[0]);
		dialog->Add(fullscreen[1]);

		dialog->Add(resetToDefaults);

		dialog->Add(skin);
		dialog->Add(skinChoices);

		dialog->Add(bitdepth);
		dialog->Add(bitdepthChoices);

		dialog->Add(res);
		dialog->Add(resChoices);
	}

	virtual void unpopulate(Dialog * dialog) 
	{ 
		GameSettingsPage::unpopulate(dialog); 

		dialog->Remove(res);
		dialog->Remove(resChoices);

		dialog->Remove(bitdepth);
		dialog->Remove(bitdepthChoices);

		dialog->Remove(fullscreen[0]);
		dialog->Remove(fullscreen[1]);

		dialog->Remove(skin);
		dialog->Remove(skinChoices);

		dialog->Remove(resetToDefaults);
	}

protected:

	/** checks the skins directory for all available skins; adds each to the list of available
	    skins*/
	virtual void scanForSkins() {
		skinChoices.list.DeleteAllItems();

		al_ffblk results;
		int counter = 0;

		for ( int retCode = al_findfirst("skins/*.ini", &results, 255);
		      retCode == 0;
			  retCode = al_findnext(&results))
		{
			skinChoices.list.InsertItem(new ListItemString(results.name), counter++);
		}
		al_findclose(&results);/**/
	}
};


class AudioOptions : public GameSettingsPage {
public:
	AudioOptions() :
	  GameSettingsPage("Audio Options")
	{ this->init(); }

	virtual void init() 
	{ 
		GameSettingsPage::init(); 

		LsoundVol.Setup(46,17,25,5,0,0,"Sound Volume:");
		LsoundVol.Shape(46,17,25,5,true);
		off[0].Setup(71,17,3,5,0,0,"Off");
		off[0].Shape(71,17,3,5,true);
		soundVol.Setup(74,18,10,1,0,0,0,256,128,1);
		soundVol.Shape(74,18,10,3,true);
		full[0].Setup(85,17,3,5,0,0,"Full");
		full[0].Shape(85,17,3,5,true);

		LbackgroundMusicVol.Setup(46,23,25,5,0,0,"Background Music Volume:");
		LbackgroundMusicVol.Shape(46,23,25,5,true);
		off[1].Setup(71,22,3,5,0,0,"Off");
		off[1].Shape(71,22,3,5,true);
		backgroundMusicVol.Setup(74,23,10,1,0,0,0,256,128,1);
		backgroundMusicVol.Shape(74,23,10,3,true);
		full[1].Setup(85,22,3,5,0,0,"Full");
		full[1].Shape(85,22,3,5,true);

		LdittyVol.Setup(46,28,25,5,0,0,"Victory Ditty Music Volume:");
		LdittyVol.Shape(46,28,25,5,true);
		off[2].Setup(71,28,3,5,0,0,"Off");
		off[2].Shape(71,28,3,5,true);
		dittyVol.Setup(74,29,10,1,0,0,0,256,128,1);
		dittyVol.Shape(74,29,10,3,true);
		full[2].Setup(85,28,3,5,0,0,"Full");
		full[2].Shape(85,28,3,5,true);

		LinterfaceVol.Setup(46,33,25,5,0,0,"Interface Volume:");
		LinterfaceVol.Shape(46,33,25,5,true);
		off[3].Setup(71,33,3,5,0,0,"Off");
		off[3].Shape(71,33,3,5,true);
		interfaceVol.Setup(74,34,10,1,0,0,0,256,128,1);
		interfaceVol.Shape(74,34,10,3,true);
		full[3].Setup(85,33,3,5,0,0,"Full");
		full[3].Shape(85,33,3,5,true);

	}

	virtual void populate(Dialog * dialog) 
	{ 
		GameSettingsPage::populate(dialog); 

		dialog->Add(LsoundVol);
		dialog->Add(LbackgroundMusicVol);
		dialog->Add(LdittyVol);
		dialog->Add(LinterfaceVol);

		dialog->Add(off[0]);
		dialog->Add(full[0]);
		dialog->Add(soundVol);

		dialog->Add(off[1]);
		dialog->Add(full[1]);
		dialog->Add(backgroundMusicVol);

		dialog->Add(off[2]);
		dialog->Add(full[2]);
		dialog->Add(dittyVol);

		dialog->Add(off[3]);
		dialog->Add(full[3]);
		dialog->Add(interfaceVol);
	}

	virtual void unpopulate(Dialog * dialog) 
	{ 
		GameSettingsPage::unpopulate(dialog); 

		dialog->Remove(LsoundVol);
		dialog->Remove(LbackgroundMusicVol);
		dialog->Remove(LdittyVol);
		dialog->Remove(LinterfaceVol);

		dialog->Remove(off[0]);
		dialog->Remove(full[0]);
		dialog->Remove(soundVol);

		dialog->Remove(off[1]);
		dialog->Remove(full[1]);
		dialog->Remove(backgroundMusicVol);

		dialog->Remove(off[2]);
		dialog->Remove(full[2]);
		dialog->Remove(dittyVol);

		dialog->Remove(off[3]);
		dialog->Remove(full[3]);
		dialog->Remove(interfaceVol);
	}

private:
	Label LsoundVol, LbackgroundMusicVol, LdittyVol, LinterfaceVol;
	Slider soundVol, backgroundMusicVol, dittyVol, interfaceVol;

	Label off[4], full[4];

	Button resetToDefaults;
};

class GameplayOptions : public GameSettingsPage {
public:
	GameplayOptions() :
	  GameSettingsPage("Gameplay Options")
	{ this->init(); }

	virtual void init() 
	{ GameSettingsPage::init(); }

	virtual void populate(Dialog * dialog) 
	{ GameSettingsPage::populate(dialog); }

	virtual void unpopulate(Dialog * dialog) 
	{ GameSettingsPage::unpopulate(dialog); }

private:
	//Label title;
};

class ControlOptions : public GameSettingsPage {
public:
	ControlOptions() :
	  GameSettingsPage("Controls")
	{ this->init(); }

	virtual void init() 
	{ GameSettingsPage::init(); }

	virtual void populate(Dialog * dialog) 
	{ GameSettingsPage::populate(dialog); }

	virtual void unpopulate(Dialog * dialog) 
	{ GameSettingsPage::unpopulate(dialog); }

private:
	//Label title;
};

OptionsMenu::~OptionsMenu()
{
	if (currentPage!=NULL)
	   currentPage->unpopulate(this);

	while (pages.size() >0) {
		delete pages.back();
		pages.pop_back();
	}
	pages.clear();
}

void OptionsMenu::HandleEvent(Widget &obj, int msg, int arg1, int arg2) {
	OverlayDialog::HandleEvent(obj, msg, arg1, arg2);

	if (currentPage!=NULL)
		currentPage->HandleEvent(obj,msg,arg1,arg2);
	
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

				//TODO reject changes here, revert back to old settings
			}

			if (obj == bAccept) {

				//TODO accept changes here


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

	bAccept.SetupNormalized(85, 90, 15, 10, KEY_A, D_EXIT, "&Accept Changes");
	
	Add(leftPanel);
	Add(labelBackground);
	Add(label);
	
	Add(video);
	Add(audio);
	Add(gameplay);
	Add(controls);
	
	Add(bQuit);
	Add(bAccept);
	Add(descriptionText);
}



void OptionsMenu::setSettingsPage(int page) {

	//check if the page is different, if it's the same one that's already
	// shown, return.
	int counter = 0;
	for (vector <GameSettingsPage *>::iterator i=pages.begin(); i!=pages.end(); i++) {
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


