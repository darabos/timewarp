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

#include "../twgui/twpopup.h"






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

	maparea = new Area(T, "map_");
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


	// define another (sub)menu

	Tedit = new Popup("gamex/interface/starmap/edit", 400, 200, game_screen);
	Tedit->exclusive = true;


	bdec = new Button(Tedit, "dec_");
	binc = new Button(Tedit, "inc_");
	bnew = new Button(Tedit, "new_");
	breplace = new Button(Tedit, "replace_");
	bplot = new Button(Tedit, "plot_");

	istarselect = 0;
	update_bplot();


	T->add(Tedit);
	T->tree_doneinit();
}


void GameStarmap::update_bplot()
{
	BITMAP *dest, *src;
	src  = starspr[istarselect]->get_bitmap(0);
	dest = bplot->bmp_default;
	clear_to_color(dest, 0);
	blit(src, dest, 0, 0, 0, 0, src->w, src->h);
}


void GameStarmap::mapeditor_stuff()
{
	// keep track of the last star that was clicked on by the mouse
	if (ptr->selection && (ptr->selection->id == STAR_ID))
	{
		Star *star = (Star*) ptr->selection;
		lastselectionstar = star;
	}


	// move a star
	if (selectionstar)
	{

		selectionstar->pos = ptr->pos;
		staywithin(0, &(selectionstar->pos), map_size);
	}


	// delete a star
	if (selectionstar && keyhandler.keyhit[KEY_DEL])
	{
		// remove from game physics
		remove(selectionstar);

		// remove from the map
		starmap->rem(selectionstar->starnum);


		// remove from memory
		delete selectionstar;

		selectionstar = 0;
	}


	// place a star
	if ( selectionstar && maparea->flag.left_mouse_press )//(mouse_b & 1) )
	{
		// update the map with this star ?
		// yep ...
		int k;
		k = selectionstar->starnum;
		starmap->sub[k]->position = selectionstar->pos / scalepos;
		

		// make sure it's not edited anymore
		selectionstar = 0;

		maphaschanged = true;

		maparea->flag.left_mouse_press = false;	// cause you've used it now
	}


	// select a star for movement (or so ...)
	//if ( (!selectionstar) && key[KEY_LCONTROL] && lastselectionstar)
	// using left-click of the mouse on the starmap area of the menu
	if ( (!selectionstar) && lastselectionstar && maparea->flag.left_mouse_press )
	{
		selectionstar = lastselectionstar;

		maparea->flag.left_mouse_press = false;
	}



	// ---------------

	// by pressing "space", you initialize the menu
	if (keyhandler.keyhit[KEY_SPACE])
	{
		Tedit->show();
		Tedit->center_abs(mouse_x, mouse_y);
	}

	if (breplace->flag.left_mouse_press)
	{
		Tedit->hide();
		T->show();

		// change the picture of the selected star
		if (selectionstar)
		{
			int k;
			k = selectionstar->starnum;
			starmap->sub[k]->type = istarselect;
			selectionstar->set_sprite(starspr[istarselect]);
		}
	}

	if (bnew->flag.left_mouse_press)
	{
		Tedit->hide();
		T->show();

		// select this picture for a new star ?
		if (!selectionstar)
		{
			selectionstar = new Star(0, 0, 0, starspr[istarselect]);
			add(selectionstar);
			
			// also ... add it to the map ?? with default settings ..
			selectionstar->starnum = starmap->add(1);	// level 1 = stars
			
		}
	}

	if (binc->flag.left_mouse_press || bdec->flag.left_mouse_press)
	{
		if (binc->flag.left_mouse_press)
			++istarselect;

		if (bdec->flag.left_mouse_press)
			--istarselect;

		if (istarselect < 0 )
			istarselect = startypelist->N - 1;
		
		if (istarselect >= startypelist->N)
			istarselect = 0;

		update_bplot();

	}
}

void GameStarmap::calculate()
{
	if (next)
		return;

	::space_view_size = wininfo.framesize;
	::space_zoom = wininfo.zoomlevel;
	::space_center = wininfo.mapcenter;

	ptr->newpos(mouse_x - maparea->pos.x, mouse_y - maparea->pos.y);


	GameBare::calculate();

	if ( mouseper->update() && (mouse_b & 2) )
	{
		if (view && view->frame && view->frame->window && view->frame->window->surface)
			wininfo.centermove((mouse_x - maparea->pos.x - 0.5*view->frame->window->surface->w) / space_zoom,
								(mouse_y - maparea->pos.y - 0.5*view->frame->window->surface->h) / space_zoom);
		//wininfo.center(Vector2(0,0));
	}


	double dt = frame_time * 1E-3;

	if (key[KEY_EQUALS])
		wininfo.zoom(1 + 1*dt);

	if (key[KEY_MINUS])
		wininfo.zoom(1 / (1 + 1*dt));


	mapeditor_stuff();

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









