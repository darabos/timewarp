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
#include "../other/configrw.h"



void play_fg(DATAFILE **scp, int scpguimusic, const char *id)
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
	px->startoption = id;
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


char new_init_dir[512];
char new_source_dir[512];
char new_target_dir[512];

void ProjectX::init()
{
	GameProject::init();

	// setup directories for default/ loading/ saving.
	strcpy(new_init_dir, "gamex");
	init_dir = new_init_dir;

	strcpy(new_source_dir, "gamex");
	source_dir = new_source_dir;

	strcpy(new_target_dir, "gamex"); // /save/save01");
	target_dir = new_target_dir;



	playerinfo.config("player/playerinfo.ini", CONFIG_READ);

	// initialize races info
	racelist.readracelist();


	// initialize the starmap ...
	// This kind of data can also be used in game initialization (star sprites)

	mapeverything.init("gamex/mapinfo.txt");


	// start with something ... solar view mode, for example ... (that already exists...)
	// but before that of course, hyperspace !! They're stacked so that if you
	// leave solar view, you enter hyper space.


	if (strcmp(startoption, "normal") == 0)
	{
		add( new GameHyperspace() );
		
		if (playerinfo.istar >= 0)
			add( new GameSolarview() );
		
		if (playerinfo.iplanet >= 0)
			add( new GamePlanetview() );
	}
		
		
		

	if (strcmp(startoption, "starmap") == 0)
		add( new GameStarmap() );

	if (strcmp(startoption, "solar") == 0)
		add( new GameSolarview() );

	if (strcmp(startoption, "planet") == 0)
		add( new GamePlanetview() );

	if (strcmp(startoption, "scan") == 0)
		add( new GamePlanetscan() );

	if (strcmp(startoption, "melee") == 0)
		add( new GameMelee() );

	// the dialog editor
	if (strcmp(startoption, "editor") == 0)
		add( new GameDialogue() );

	// the alien dialog interface
	if (strcmp(startoption, "dialog") == 0)
		add( new GameAliendialog() );
}


void ProjectX::quit()
{
	//playerinfo.write();
	playerinfo.config("player/playerinfo.ini", CONFIG_WRITE);

	// save edited races info (only saves changes)
	racelist.writeracelist();

	mapeverything.discard();
}
