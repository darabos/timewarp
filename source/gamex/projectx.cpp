#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../melee.h"
REGISTER_FILE

#include "projectx.h"

#include "gamestarmap.h"
#include "gamehyper.h"
#include "gamesolarview.h"
#include "gameplanetview.h"
#include "gameplanetscan.h"
#include "gamemelee.h"
#include "edit/edit_dialogue.h"
#include "gamedialogue.h"



void play_fg(DATAFILE **scp, int scpguimusic)
{
	// destroy the gui
	bool gui_stuff = false;
	
	if (*scp) {
		gui_stuff = true;
		sound.stop_music();
		if (*scp) unload_datafile(*scp);
		*scp = NULL;
	}
	
	// the new "thing", for the "big game"
	ProjectX *px;
	px = new ProjectX();
	px->play();
	//nm = 1;
	delete px;
	
	// re-initialize the gui
	if (gui_stuff) {
		*scp = load_datafile("scpgui.dat");
		sound.play_music((Music *)((*scp)[scpguimusic].dat), TRUE);
//		showTitle();
	}
}


void ProjectX::init()
{
	GameProject::init();

	playerinfo.init("gamex/player/playerinfo.ini");

	// initialize races info
	racelist.readracelist();


	// initialize the starmap ...
	// This kind of data can also be used in game initialization (star sprites)

	mapeverything.init("gamex/mapinfo.txt");


	// start with something ... solar view mode, for example ... (that already exists...)
	// but before that of course, hyperspace !! They're stacked so that if you
	// leave solar view, you enter hyper space.


	/*
	add( new GameHyperspace() );
	
	if (playerinfo.istar >= 0)
		add( new GameSolarview() );

	if (playerinfo.iplanet >= 0)
		add( new GamePlanetview() );
		*/
		
		
		

	add( new GameStarmap() );
//	add( new GameSolarview() );
//	add( new GamePlanetview() );
//	add( new GamePlanetscan() );
//	add( new GameMelee() );
//	add( new GameDialogue() );		// the editor
//	add( new GameAliendialog() );	// the alien interface -- need additional info, can't just load like this.
//	add( new GameTriggerEdit() );
//	add( new GameTriggerEdit() );
	
}


void ProjectX::quit()
{
	playerinfo.write();

	// save edited races info (only saves changes)
	racelist.writeracelist();

	mapeverything.discard();
}
