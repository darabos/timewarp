#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../melee.h"
REGISTER_FILE

#include "../melee/mlog.h"
#include "../melee/mcontrol.h"
#include "../melee/mframe.h"
#include "../melee/mview.h"
#include "../melee/mitems.h"

#include "../scp.h"
#include "../util/history.h"

#include "gamestarmap.h"






Star::Star(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite)
:
SpaceObject(creator, opos, oangle, osprite)
{
	id = STAR_ID;
	layer = LAYER_SHOTS;
}

void Star::animate(Frame *f)
{
	//SpaceObject::animate(f);
	//sprite->animate(pos, sprite_index, f);
	Vector2 s = sprite->size(sprite_index);
	double scale = space_zoom;
	if (scale < 0.25)
		scale = 0.25;
	sprite->draw(corner(pos, s ), s * scale, sprite_index, f);
}

void Star::calculate()
{
	SpaceObject::calculate();
}





void GameStarmap::init_menu()
{
	// place the menu into video-memory, cause we're using this as basis for
	// drawing; the game draws onto part of the menu.
	T = new TWindow("gamex/interface/starmap", 0, 0, game_screen, true);

	maparea = new AreaTablet(T, "map_");
}


void GameStarmap::init()
{

	GameBare::init();
	//::physics = this;	// is already in prepare

	double H = 1000;
	scalepos = 100;
	H *= scalepos;

	size = Vector2(H, H);

	prepare();

//	mapwrap = false;
	//wininfo.init( Vector2(400,400), 800.0, tempframe );
	wininfo.zoomlimit(size.x);
	wininfo.center(Vector2(0,0));
	//wininfo.scaletowidth(size.x);	// zoom out to this width.


	// load star sprites
	int i;
	for ( i = 0; i < startypelist->N; ++i )
	{
		char tmp[512];
		sprintf(tmp, "gamex/stars/star_%s_01.bmp", startypelist->type[i].type_string);
		starspr[i] = create_sprite( tmp, SpaceSprite::MASKED );
	}

	// create star objects ?!
	starmap = mapeverything.region[0];	// use the starmap of the 1st region

	for ( i = 0; i < starmap->Nsub; ++i )
	{
		Star *star;
		int k;

		k = starmap->sub[i]->type;
		star = new Star(0, starmap->sub[i]->position * scalepos, 0.0, starspr[k]);
		star->starnum = i;

		add(star);
	}

	SpaceSprite *mousespr;
	mousespr = create_sprite( "gamex/mouse/pointer_starmap.bmp", SpaceSprite::MASKED );
	ptr = new MousePtr (mousespr);
	add(ptr);


	// the player
	playerspr = create_sprite( "gamex/stars/player_01.bmp", SpaceSprite::MASKED );
	player = new ThePlaya(playerspr, &playerinfo);
	add(player);


	mouseper = new Periodics(0.1);
	keyper = new Periodics(0.1);

	selectionstar = 0;
	lastselectionstar = 0;
	maphaschanged = false;	// check if the map changed; if so, it's to be written back to disk
}





void GameStarmap::calculate()
{
	GameBare::calculate();

	if ( mouseper->update() && (mouse_b & 2) )
	{
		if (view && view->frame && view->frame->window && view->frame->window->surface)
			wininfo.centermove((mouse_x - 0.5*view->frame->window->surface->w) / space_zoom,
								(mouse_y - 0.5*view->frame->window->surface->h) / space_zoom);
		//wininfo.center(Vector2(0,0));
	}


	double dt = frame_time * 1E-3;

	if (key[KEY_EQUALS])
		wininfo.zoom(1 + 1*dt);

	if (key[KEY_MINUS])
		wininfo.zoom(1 / (1 + 1*dt));


	// keep track of the last star that was clicked on by the mouse
	if (ptr->selection && (ptr->selection->id == STAR_ID))
	{
		Star *star = (Star*) ptr->selection;
		lastselectionstar = star;
	}


	// select a star for movement (or so ...)
	if ( (!selectionstar) && key[KEY_LCONTROL] && lastselectionstar)
	{
		selectionstar = lastselectionstar;
	}


	// add a new star (or toggle its type if it's still selected)
	if (keyper->update())
	{
		if (key[KEY_A])
		{
			if (!selectionstar)
			{
				selectionstar = new Star(0, 0, 0, starspr[0]);
				add(selectionstar);

				// also ... add it to the map ?? with default settings ..
				selectionstar->starnum = starmap->add(1);	// level 1 = stars

			} else {
				int k;
				k = selectionstar->starnum;
				int &startype = starmap->sub[k]->type;

				++ startype;
				if (startype > startypelist->N - 1)
					startype = 0;

				selectionstar->set_sprite(starspr[startype]);
			}
		}
	}

	// move a star
	if (selectionstar)
	{
		selectionstar->pos = wininfo.mapcenter +
			Vector2(	(mouse_x - 0.5*view->frame->window->surface->w) / space_zoom,
						(mouse_y - 0.5*view->frame->window->surface->h) / space_zoom );

		staywithin(0, &(selectionstar->pos), map_size);
	}


	// place a star
	if ( selectionstar && (mouse_b & 1) )
	{
		// update the map with this star ?
		// yep ...
		int k;
		k = selectionstar->starnum;
		starmap->sub[k]->position = selectionstar->pos / scalepos;
		

		// make sure it's not edited anymore
		selectionstar = 0;

		maphaschanged = true;
	}

}

void GameStarmap::animate(Frame *frame)
{
	if (next)	// shouldn't happen for this game type ?!
		return;

	::space_zoom = wininfo.zoomlevel;
	::space_center = wininfo.mapcenter;


	GameBare::animate(frame);
}



GameStarmap::ThePlaya::ThePlaya(SpaceSprite *osprite, PlayerInfo *playinf)
:
LocalPlayerInfo(osprite, playinf)
{
	pos = 0;		// locate the player somewhere on the local map
}




void GameStarmap::quit()
{
	delete playerspr;

	int i;
	for ( i = 0; i < startypelist->N; ++i )
		delete starspr[i];

	if (maphaschanged)
	{
		// write the map to disk
		mapeverything.save("gamex/mapinfo.txt");
	}

	GameBare::quit();
}









