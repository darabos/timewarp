
#include <allegro.h>
#include <stdio.h>
#include <string.h>
//#include <string.h>

#include "../melee.h"
REGISTER_FILE


//#include "melee/mframe.h"
//#include "melee/mview.h"
//#include "melee/mitems.h"

#include "gamesolarview.h"
#include "gameplanetview.h"

#include "stuff/space_body.h"
#include "stuff/backgr_stars.h"

#include "twgui/twgui.h"

#include "general/sprites.h"
#include "gamestarmap.h"


static GameSolarview *gsolar;


void load_planettypes(SpaceSprite ***planettypespr)
{
	// load planet sprites
	(*planettypespr) = new SpaceSprite* [planettypelist->N];
	int i;
	for ( i = 0; i < planettypelist->N; ++i )
	{
		char tmp[512];
		sprintf(tmp, "%s%s_01.bmp",
			"gamex/solarview/planet_",
			planettypelist->type[i].type_string);
		(*planettypespr)[i] = create_sprite( tmp, SpaceSprite::MASKED );
	}
}

void load_surfacetypes(BITMAP ***surfacebmp)
{
	// load surface bitmaps
	(*surfacebmp) = new BITMAP* [surfacetypelist->N];
	int i;
	for ( i = 0; i < surfacetypelist->N; ++i )
	{
		char tmp[512];
		sprintf(tmp, "%s%s_01.bmp",
			surfacetypelist->basename,
			surfacetypelist->type[i].type_string);
		load_bitmap32(&(*surfacebmp)[i], tmp);
		scale_bitmap32(&(*surfacebmp)[i], 0.2);
	}
}


// scan the normal files:

void set_filelist(TextList *tl, char *scanname)
{
	int err;
	al_ffblk info;
	
	tl->clear_optionlist();
	
	err = al_findfirst(scanname, &info, FA_ARCH);	
	
	while (!err)
	{
		if (strcmp(info.name, ".") && strcmp(info.name, "..") )
			tl->add_optionlist(info.name);
		
		err = al_findnext(&info);
	}
	
	al_findclose(&info);
}





void ellipsparams(Vector2 relpos, double ellb,
				  double &R, Vector2 &Poffs, int &col)
{
	//double R;
	double b_sqrt;
	
	
	b_sqrt = sqrt(ellb);
	
	// pure ellips that hits the point has radius R
//	Vector2 relpos;
//	relpos = pos - 0.5*map_size;
	relpos.y *= b_sqrt;

	double offs = 0.5;	// offset of the ellips
	double a;
	a = offs * offs - 1;

	double b;
	b = -2 * offs * relpos.y;

	double c;
	c = magnitude_sqr(relpos);

	// solve quadr. eq.
	double det;
	det = b*b - 4*a*c;
	if (det < 0)
	{
		tw_error("negative value for ellips root");
	}
	det = sqrt(det);

	// two solutions:
	double r1, r2;
	r1 = (-b + det) / (2*a);
	r2 = (-b - det) / (2*a);

	if (r1 > 0)
		R = r1;
	else
		R = r2;

	// shift the ellips down (so, scale y-size down, hence R is scaled linearly cause y~Rsin(a))
	Poffs = Vector2(0, offs * R / b_sqrt);
	//R *= 0.75;
	
	
	//int col;
	if (R < 500.0)
		col = makecol(115,0,0);
	else if (R < 1000.0)
		col = makecol(0,85,0);
	else if (R < 1500.0)
		col = makecol(0,44,66);
	else
		col = makecol(0,0,115);
}



void MapEditor2::define_stats()
{
	ved->values[0]->set(vtype_float, "atmospheric pressure", 0.0, 100.0);
	ved->values[1]->set(vtype_float, "radius (es)", 0.1, 100.0);
	ved->values[2]->set(vtype_float, "density (kg/m3)", 0.5, 6.0);
	ved->values[3]->set(vtype_float, "day (es))", 0.0, 100.0);
	ved->values[4]->set(vtype_float, "tilt (degr)", 0.0, 90.0);
	ved->values[5]->set(vtype_float, "albedo", 0.0, 1.0);
	ved->values[6]->set(vtype_float, "greenhouse", 0.0, 1.0);
	ved->values[7]->set(vtype_int, "weather", 0, 9);
	ved->values[8]->set(vtype_int, "tectonics", 0, 9);
}



void MapEditor2::init_interface(TWindow *T,
								FONT *usefont, SpaceSprite **planettypespr, int N1,
								BITMAP **surfacebmp, int N2)
{
	Tedit = new IconTV("gamex/interface/planetview/edit", 1400, 900, game_screen);
	Tedit->exclusive = false;
	bnew = new Button(Tedit, "new_");
	breplace = new Button(Tedit, "replace_");
	Tedit->tv->set(planettypespr, N1);

	tvsurf = new TVarea(Tedit, "plot2_");
	tvsurf->set(surfacebmp, N2);

	ved = new ValueEdit(Tedit, "values_", usefont, 64);

	ved->info->text_color = makecol(200,200,200);
	ved->edit->text_color = makecol(200,200,200);

	define_stats();

	T->add(Tedit);

	Tedit->show();
	Tedit->focus();

	Tedit->layer = 1;
}


MapObj *MapEditor2::create_mapobj(Vector2 pos)
{
	

	Vector2 Poffs;
	int col;
	double R;
	
	double b = b_default;
	ellipsparams(pos - mapcenter, b,
				  R, Poffs, col);

	SolarBody	*s;

	s = new SolarBody(0, pos, 0.0, Tedit->tv->makespr(), mapcenter, Tedit->tv->isel,
		mapcenter+Poffs, R, b, col	// ellips information
		);

	return s;
}



void MapEditor2::move()
{
	MapEditor::move();


	SolarBody	*s;
	s = (SolarBody*) selection;
	
	s->stayhere = s->pos;	// in this case, movement is allowed ...
	
	Vector2 Poffs;
	int col;
	double R;
	
	ellipsparams(s->pos - mapcenter, s->ellipsb,
		R, Poffs, col);
	
	s->ellipsR = R;
	s->ellipscol = col;
	s->ellipscenter = mapcenter + Poffs;
	
//	s->drawshadow();
	colorizeobj(s);	// well ... to mimic changing shadows wrt the sun
}


void MapEditor2::replace()
{
	MapEditor::replace();

	isurfacetype = tvsurf->isel;

	colorizeobj((SolarBody*) selection);

	save_stats();
}


void MapEditor2::newselection()
{
	// if a new planet is selected, read its info from the star file !
	MapEditor::newselection();

	init_stats();

	isurfacetype = tvsurf->isel;

	// also ... the planet picture is ...
	int k;
	k = selection->starnum;
	Tedit->tv->set_sel( objmap->sub[k]->type );
}



void MapEditor2::colorizeobj(SolarBody *s)
{
	s->drawshadow();
	// this resets the sprite and adds shade ?

	SpaceSprite *spr = s->get_sprite();

	double rc, gc, bc, r_ref, g_ref, b_ref;
	avcolor( gsolar->surfacebmp[isurfacetype], &r_ref, &g_ref, &b_ref);
	double temperature = 5500.0;
	
	double rat, gat, bat;	// extra atmospheric weight.
	rat = 0.7;	// you don't have that on a starship,
	gat = 0.6;	// but well, this makes it more understandable
	bat = 0.4;	// for us earthdwellers ... otherwise the sun would look slightly blueish ...
	// the blue reduction is somewhat exaggerated
	
	rc = spec_r(temperature) * rat * r_ref;
	gc = spec_g(temperature) * gat * g_ref;
	bc = spec_b(temperature) * bat * b_ref;
	
	balance(&rc, &gc, &bc);
	colorize(spr, rc, gc, bc);
	brighten(spr);
}


void MapEditor2::add()
{
	//GameSolarview *gs = (GameSolarview*)physics;

	MapEditor::add();
	
	// you need a new random number (plus check it doesn't exist yet)
	objmap->sub[selection->starnum]->id = mapeverything.gen_id();


	// set other stuff as well ... the surface parameter, eg... and color
	// the sprite based on this thingy !!

	isurfacetype = tvsurf->isel;

	colorizeobj((SolarBody*) selection);


	// write stuff to a .ini file ...

	save_stats();
}





void MapEditor2::set_config()
{
	set_config_float(0, "atmo",    ved->values[0]->value);
	set_config_float(0, "radius",  ved->values[1]->value);
	set_config_float(0, "density", ved->values[2]->value);
	set_config_float(0, "day",     ved->values[3]->value);
	set_config_float(0, "tilt",    ved->values[4]->value);
	set_config_float(0, "albedo",  ved->values[5]->value);
	set_config_float(0, "greenhouse",    ved->values[6]->value);
	set_config_int(0, "weather",   (int) ved->values[7]->value);
	set_config_int(0, "tectonics", (int) ved->values[8]->value);

	// and the surface type string ?
	char *t;
	t = surfacetypelist->type[tvsurf->isel].type_string;
	set_config_string(0, "surface", t);
}


void MapEditor2::save_stats()
{
	if (!selection)
		return;

	int id;
	id = objmap->sub[ selection->starnum ]->id;

	char tmp[512];
	sprintf(tmp, "gamex/gamedata/surface/%08X.ini", id);
	set_config_file(tmp);

	set_config();

	flush_config_file();
}


void MapEditor2::get_config()
{
	ved->values[0]->value = get_config_float(0, "atmo", 0);
	ved->values[1]->value = get_config_float(0, "radius", 0);
	ved->values[2]->value = get_config_float(0, "density", 0);
	ved->values[3]->value = get_config_float(0, "day", 0);
	ved->values[4]->value = get_config_float(0, "tilt", 0);
	ved->values[5]->value = get_config_float(0, "albedo", 0);
	ved->values[6]->value = get_config_float(0, "greenhouse", 0);
	ved->values[7]->value = get_config_int(0, "weather", 0);
	ved->values[8]->value = get_config_int(0, "tectonics", 0);

	ved->edit_update();

	// and the surface type ?
	char tmp[512];
	strcpy(tmp, get_config_string(0, "surface", "default"));
	tvsurf->set_sel ( surfacetypelist->get_index(tmp, 0) );
}


void MapEditor2::init_stats()
{
	if (!selection)
		return;

	int id;
	id = objmap->sub[ selection->starnum ]->id;

	char tmp[512];
	sprintf(tmp, "gamex/gamedata/surface/%08X.ini", id);
	set_config_file(tmp);

	get_config();
}


void MapEditor2::check_radius()
{
	if (!selection)
		return;

	char *t;
	t = planettypelist->type[ objmap->sub[ selection->starnum ]->type ].type_string;

	// check if the planet's radius was changed; if so, check validity
	double Rmin, Rmax;

	if (strcmp(t, "dwarf") == 0)
	{
		Rmin = 0.01;
		Rmax = 0.1;
	}
	else
	if (strcmp(t, "small") == 0)
	{
		Rmin = 0.1;
		Rmax = 0.5;
	}
	else
	if (strcmp(t, "medium") == 0)
	{
		Rmin = 0.5;
		Rmax = 2.0;
	}
	else
	if (strcmp(t, "big") == 0)
	{
		Rmin = 2.0;
		Rmax = 10.0;
	}
	else
	if (strcmp(t, "giant") == 0)
	{
		Rmin = 10.0;
		Rmax = 20.0;
	}
	else
	{
	  Rmin = 0;
	  Rmax = 0;
	  tw_error("Undefined planet type");
	}

	double R;
	R = get_config_float(0, "radius", 0);

	if (R < Rmin || R > Rmax)
	{
		R = random(Rmin, Rmax);
	}

	ved->values[1]->value = R;
}





void GameSolarview::init_menu()
{
	// place the menu into video-memory, cause we're using this as basis for
	// drawing; the game draws onto part of the menu.
	T = new TWindow("gamex/interface/planetview", 0, 0, game_screen, true);

	maparea = new Area(T, "map_");
	makevideobmp(maparea->backgr);	// for faster drawing (I think ...)
}




void GameSolarview::init()
{
	gsolar = this;

	GameBare::init();

	H = 4000;
	
	size = Vector2(H, H*tempframe->ratio);
	prepare();

//	mapwrap = false;
	//wininfo.init( Vector2(200,200), 1024.0, tempframe );
	wininfo.zoomlimit(size.x);
	wininfo.scaletowidth(size.x);	// zoom out to this width.
	wininfo.set_game(this);


	// create star objects ?!
	
	starmap = mapeverything.sub[0];	// use the starmap of the 1st region

//	playerinfo.istar = 0;
	int istar;
	istar = playerinfo.istar;
	if (istar < 0)
	{
		istar = 0;
		playerinfo.istar = istar;
	}
	starnum = istar;

	solarmap = starmap->sub[istar];	// use the solarsystem belonging to that star


	SpaceObject *solarbody;
	//SpaceSprite *spr;

	Vector2 sunpos;
	sunpos = Vector2(0.5, 0.25) * size;

	// load the star data
	char txt[512];
	sprintf(txt, "gamex/solarview/star_%s_01.bmp",
						startypelist->type[solarmap->type].type_string);
	starspr = create_sprite( txt, SpaceSprite::MASKED );
	solarbody = new SolarBody(0, sunpos, 0.0, starspr, sunpos, istar,
									0, 0, 0, 0);
	// solarmap->o = solarbody;	don't do this, or the hyperspace map may go crazy ...
	// since the stars are already referenced (and edited?) therein...
	add(solarbody);
	solarbody->id = 0;	// cause it's not a editable map object...

	// the star is untouchable.
	solarbody->collide_flag_anyone = 0;
	solarbody->collide_flag_sameteam = 0;
	solarbody->collide_flag_sameship = 0;

	// load planet sprites
	load_planettypes(&planettypespr);
	load_surfacetypes(&surfacebmp);
	/*
	planettypespr = new SpaceSprite* [planettypelist->N];
	int i;
	for ( i = 0; i < planettypelist->N; ++i )
	{
		char tmp[512];
		sprintf(tmp, "gamex/solarview/planet_%s_01.bmp", planettypelist->type[i].type_string);
		planettypespr[i] = create_sprite( tmp, SpaceSprite::MASKED );
	}

	// load surface bitmaps
	surfacebmp = new BITMAP* [surfacetypelist->N];
	for ( i = 0; i < surfacetypelist->N; ++i )
	{
		char tmp[512];
		sprintf(tmp, "gamex/planetscan/surface_%s_01.bmp", surfacetypelist->type[i].type_string);
		load_bitmap32(&surfacebmp[i], tmp);
		scale_bitmap32(&surfacebmp[i], 0.2);
	}
	*/

	// load the planet data


	int i;
	for ( i = 0; i < solarmap->Nsub; ++i )
	{
		Vector2 Poffs;
		int col;
		double R;
		
		double b = b_default;
		ellipsparams(solarmap->sub[i]->position - sunpos, b,
			R, Poffs, col);

		int k;
		k = solarmap->sub[i]->type;

		planetspr = new SpaceSprite(planettypespr[k]->get_bitmap(0));
		// change the color according to the surface and the star...

		char tmp[512];
		sprintf(tmp, "gamex/gamedata/surface/%08X.ini", solarmap->sub[i]->id);
		set_config_file(tmp);
		strcpy(tmp, get_config_string(0, "surface", "default"));
		k = surfacetypelist->get_index(tmp, 0);
		
		double rc, gc, bc, r_ref, g_ref, b_ref;
		avcolor(surfacebmp[k], &r_ref, &g_ref, &b_ref);

		// WOOPS - should CHANGE!!
		double temperature = 5500.0;

		double rat, gat, bat;	// extra atmospheric weight.
		rat = 0.7;	// you don't have that on a starship,
		gat = 0.6;	// but well, this makes it more understandable
		bat = 0.4;	// for us earthdwellers ... otherwise the sun would look slightly blueish ...
		// the blue reduction is somewhat exaggerated

		rc = spec_r(temperature) * rat * r_ref;
		gc = spec_g(temperature) * gat * g_ref;
		bc = spec_b(temperature) * bat * b_ref;

		balance(&rc, &gc, &bc);
		colorize(planetspr, rc, gc, bc);
		brighten(planetspr);


	
		solarbody = new SolarBody(0, solarmap->sub[i]->position, 0.0,
			planetspr,
			sunpos, i, sunpos+Poffs, R, b, col	// ellips information
									);
		solarmap->sub[i]->o = solarbody;
		add(solarbody);

	}

	// the player
	//SpaceSprite *playerspr;
	playerspr = create_sprite( "gamex/solarview/player_01.bmp", SpaceSprite::MASKED, 64 );
	player = new ThePlaya(playerspr, &playerinfo);
	add(player);

	locate_onedge_aligned_to_center(player, 0.5*size, 0.45*size);


	StarBackgr *b;
	b = new StarBackgr();
	b->init(100, view->frame);
	add(b);


	// stuff for the map editor

	SpaceSprite *mousespr;
	mousespr = create_sprite( "gamex/mouse/pointer_starmap.bmp", SpaceSprite::MASKED );
	ptr = new MousePtr (mousespr);
	add(ptr);


	FONT *usefont = videosystem.get_font(3);

	// Button that displays the name of the planet.
	starname = new TextEditBox(T, "starname_", usefont, solarmap->name, sizeof(solarmap->name));
	starname->set_textcolor(makecol(255,255,0));
	strcpy(oldstarname, solarmap->name);



	mapeditor = new MapEditor2();
	mapeditor->set_game(this, ptr);

	mapeditor->init_interface(T, usefont, planettypespr, planettypelist->N,
		surfacebmp, surfacetypelist->N);

	mapeditor->set_mapinfo( solarmap, 1.0);

	mapeditor->mapcenter = sunpos;

	//mapeditor->editmap = solarmap;


	T->tree_doneinit();

	mapeditor->Tedit->show();
	mapeditor->Tedit->focus();

	// initialize the colony placer/editor


	Popup *cedit;
	cedit = new Popup("gamex/interface/planetview/editcol", 200, 500, game_screen);


	cnew = new Button(cedit, "new_");
	crem = new Button(cedit, "rem_");
	cupdate = new Button(cedit, "update_");

	char *pop_size = new char [16];
	strcpy(pop_size, "1000");
	cpop = new TextEditBox(cedit, "population_", usefont, pop_size, 16);
	cpop->set_textcolor(makecol(255,128,128));

	crace = new TextButton(cedit, "race_", usefont);
	crace->passive = false;

	cdialogname = new TextButton(cedit, "dialogname_", usefont);
	cdialogname->passive = false;

	cedit->doneinit();

	T->add(cedit);
	cedit->show();

	// initialize the racelist popup window ...
	//PopupList *rpopup;

	// construct a list of races ...
	RaceInfo *ri;
	ri = racelist.first;
	int N = 0;
	while (ri)
	{
		list[N] = ri->id;
		ri = ri->next;
		++N;
	}

	// race selection screen
	rpopup = new PopupList(crace, "gamex/interface/planetview/list", "list_",
		10, 10, usefont, 0);
	rpopup->tbl->set_optionlist(list, N, makecol(200,200,200));

	rpopup->doneinit();
	T->add(rpopup);


	// dialog selection screen

	dpopup = new PopupList(cdialogname, "gamex/interface/planetview/list", "list_",
		10, 10, usefont, 0);
	//dpopup->tbl->set_optionlist(dialoglist, N, makecol(200,200,200));

	dpopup->doneinit();
	T->add(dpopup);


//	Tedit->layer = 1;	// shown first
	cedit->layer = 1;	// shown first
	rpopup->layer = 1;
	dpopup->layer = 1;
	T->layer = 2;		// always shown later


	if (!player->isObject())
	{
		tw_error("not an object");
	}
}


void GameSolarview::quit()
{
	delete starspr;
	delete playerspr;

	int i;
	
	for ( i = 0; i < planettypelist->N; ++i )
		delete planettypespr[i];
	delete planettypespr;
	
//	for ( i = 0; i < solarmap->Nsub; ++i )
//		delete planetspr[i];
//	delete planetspr;

	for ( i = 0; i < surfacetypelist->N; ++i )
		del_bitmap(&surfacebmp[i]);
	delete surfacebmp;

	if (!hardexit)	// game is not quitted completely
	{
		// set the player position exactly equal to the star for appearing in solar orbit
		MapSpacebody *starmap;
		starmap = mapeverything.sub[0];
		playerinfo.pos = starmap->sub[playerinfo.istar]->position * starmap->scalepos;
		playerinfo.vel = 0;

		playerinfo.istar = -1;	// cause you've left the solar system
		playerinfo.angle = player->angle;
	}
	else
		playerinfo.sync(player);


	if (strcmp(solarmap->name, oldstarname))
		mapeditor->maphaschanged = true;

	if (mapeditor->maphaschanged)
	{
		// write the map to disk
		mapeverything.save("gamex/mapinfo.txt");
	}

	GameBare::quit();
}


void GameSolarview::refocus()
{
	if (!hardexit)
	{
		player->angle = playerinfo.angle;	// first, update the angle ...
		playerinfo.sync(player);
	}
}


void GameSolarview::calculate()
{
	if (next)
		return;

	double dt = frame_time * 1E-3;

	if (key[KEY_EQUALS])
		wininfo.zoom(1 + 1*dt);

	if (key[KEY_MINUS])
		wininfo.zoom(1 / (1 + 1*dt));



	wininfo.center(player->pos);
	wininfo.edgecorrect();

	wininfo.maxzoom = 0.5;


	GameBare::calculate();

	if (!(player && player->exists()))
	{
		state = 0;
		quit();
		return;
	}

	if (key[KEY_UP])
		player->accelerate(0, player->angle, 0.1*dt, 1.0);

	if (key[KEY_LEFT])
		player->angle -= 0.5*PI2 * dt;

	if (key[KEY_RIGHT])
		player->angle += 0.5*PI2 * dt;


	// editor stuff

	ptr->newpos( iround(mouse_x - maparea->pos.x), iround(mouse_y - maparea->pos.y));
	mapeditor->calculate();


	// colony editor stuff

	if (rpopup->ready())
	{
		int k;
		k = rpopup->getvalue();
		if (k >= 0)
			crace->set_text(rpopup->tbl->optionlist[k], makecol(200,255,200));


		// construct a list of available dialogs for the race ...
		
		if (crace->text)
		{
			char scanname[512];
			strcpy(scanname, "gamex/gamedata/races/");
			strcat(scanname, crace->text);
			strcat(scanname, "/*.dialog");
			
			set_filelist(dpopup->tbl, scanname);
		}
	}

	
	if (dpopup->ready())
	{
		int k;
		k = dpopup->getvalue();
		if (k >= 0)
			cdialogname->set_text(dpopup->tbl->optionlist[k], makecol(200,255,200));
	}


	if (mapeditor->selection)
	{
		// which location are we at
		int istar, iplan, imoon;

		istar = starnum;
		iplan = mapeditor->selection->starnum;
		imoon = -1;

		// which race are we dealing with
		RaceInfo *ri = 0;
		if (crace->text)
			ri = racelist.get(crace->text);

		if (ri)
		{

			// check if there's a colony at this location, for the given race
			RaceColony *rc = 0;
			rc = ri->find_colony(istar, iplan, imoon);
			
			
			if (cnew->flag.left_mouse_press && rc == 0 && cdialogname->text)
			{
				// create a new colony on the selected planet
				// if it's not yet occupied by this race

				rc = new RaceColony(ri);
				
				ri->add(rc);
				
				rc->locate(istar, iplan, imoon);
				
				rc->set_info( atoi(cpop->get_text()), cdialogname->text );
				
				rc->calculate();
			}
			
			
			
			if (cupdate->flag.left_mouse_press && rc != 0)
			{
				// other function: replace colony info of the specified race

				rc->set_info( atoi(cpop->get_text()), cdialogname->text );
				rc->calculate();

				// perhaps ... reset an invalid dialog-name in case the race is changed ?
				rc->changeowner(ri);	// only changes it, if needed.
			}
			

			if (crem->flag.left_mouse_press && rc)
			{
				// remove the colony from the race's property

				ri->rem(rc);

				// make sure the colony list is updated (note that this approach fails
				// if the list is empty, but deleting an entire race should not
				// occur too often)
				if (ri->firstcol)
					ri->firstcol->modified = true;
			}

		}
		

	}



	//save_surface();
}



void GameSolarview::checknewgame()
{
	//return;

	if (player->collisionwith)
	{

		// only activate planet view mode, if you've hit a planet.
		if (player->collisionwith->id == MAPOBJ_ID)
		{

			// playerinfo:
			// istar and iplanet should be known for this subgame.

			// which planet did you collide with ?!
			playerinfo.iplanet = ((SolarBody*) player->collisionwith)->starnum;
			playerinfo.angle = player->angle;
	
			player->vel = 0;

			// spawn a subgame
			if (!gamerequest && !next)
				gamerequest = new GamePlanetview();
		}
	}

}




void GameSolarview::animate(Frame *frame)
{
	if (next)
		return;

	GameBare::animate(frame);

	// highlight race colonies
	racelist.animate_map(frame, 2);

}



GameSolarview::ThePlaya::ThePlaya(SpaceSprite *osprite, PlayerInfo *playinf)
:
LocalPlayerInfo(osprite, playinf)
{
	pos = 0;		// locate the player somewhere on the local map
	collide_flag_sameship = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_anyone = ALL_LAYERS;
	mass = 1;
}



