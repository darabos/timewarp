/*

  PLANET EXPLORER

  You can toggle the "editmode" variable to change this into an edit-mode program (place objects).
  In editmode, you "remove" objects by shooting them.
  The objects are saved to file according to the positions they are at the end of edit mode (escape/quit)

*/

#include <allegro.h> //allegro library header

#include "../melee.h"          //used by all TW source files.  well, mostly.  
REGISTER_FILE                  //done immediately after #including melee.h, just because I said so
#include "../melee/mgame.h"    //Game stuff
#include "../melee/mcbodies.h" //asteroids & planets
#include "../melee/mship.h"    //ships
#include "../melee/mlog.h"     //networking / demo recording / demo playback
#include "../melee/mcontrol.h" //controllers & AIs
#include "../melee/mview.h"    //Views & text messages
#include "../melee/mshppan.h"  //ship panels...
#include "../melee/mitems.h"   //indicators...
#include "../melee/mfleet.h"   //fleets...
#include "../frame.h"
#include "../id.h"

#include "../melee/mshot.h"

#include "gplexplr.h"

#include <stdio.h>
#include <string.h>



/* ------------------ STATIC OBJECTS -----------------

  mountains, buildings; things that don't need calculations of themselves, since
  they are completely passive.

*/



MeasurePeriod::MeasurePeriod()
{
	period = 1;
	time = 0;
}

void MeasurePeriod::setperiod(double operiod)
{
	period = operiod;
	time = 0;
}

void MeasurePeriod::reset()
{
	STACKTRACE

	time = game->game_time * 1E-3;
}

bool MeasurePeriod::ready()
{
	STACKTRACE

	return (game->game_time*1E-3 - time > period);
}


GunBody::GunBody(Plsurface *surf, Vector2 opos, double oangle, SpaceSprite *obodysprite,
				 SpaceSprite *oweaponsprite, char *txt, bool otangible, int ohealth,
				   int oturnvel, int oweaponvel, int oweapondamage)
:
Mountain(surf, opos, oangle, obodysprite, txt, otangible, ohealth)
{
	turnvel = oturnvel;				// number of seconds required to turn 360 degr.
	weaponsprite = oweaponsprite;

	weaponvel = scale_velocity(oweaponvel);
	weapondamage = oweapondamage;

	weaponperiod.setperiod(1.0);
	turnperiod.setperiod(turnvel / 64);		// time needed to turn 1 seg.

	weaponrange = 300.0;	// short range?
	viewrange = 500.0;		// detecting ships at this range?
}


SpaceObject *find_closest(SpaceLocation *s, int test_attr, double R)
{
	STACKTRACE

	Vector2 Dmin;
	SpaceObject *closest;
	Query q;

	closest = 0;

	Dmin = Vector2(1E99, 1E99);

	// check for neighbouring (small) objects
	for (q.begin(s, ALL_LAYERS, R); q.current; q.next())
	{
		SpaceObject *o;
		o = q.currento;
		if (( o->attributes & test_attr) != 0)
		{
			//message.out("there is a ship!!");
			Vector2 D;
			D = min_delta(s->pos, o->pos, map_size);
			if (D.x < Dmin.x && D.y < Dmin.y)
			{
				if (D.length() < Dmin.length())
				{
					Dmin = D;
					closest = o;
				}
			}
		}
		
	}
	q.end();

	return closest;
}


void GunBody::calculate()
{
	STACKTRACE


	// find the closest target?
	SpaceObject *o;
	o = find_closest(this, ATTRIB_SHIP, viewrange);
	
	if (o)
	{
		if (turnperiod.ready())
		{
			double a;
			a = (o->pos - pos).atan();

			a -= angle;
			while (a <= -PI) a += PI2;
			while (a >   PI) a -= PI2;

			double da = PI2/64;

			if (a < -da/2)
				angle -= da;
			else if (a > da/2)
				angle += da;


			turnperiod.reset();
		}

		if (min_delta(pos, o->pos).length() < weaponrange  &&  weaponperiod.ready())
		{
			game->add( new Missile(this, Vector2(0,50), angle, weaponvel, weapondamage, weaponrange, 1,
				this, weaponsprite, 0) );

			weaponperiod.reset();
		}
	}
	
	sprite_index = get_index(angle);
}



int GunBody::handle_damage (SpaceLocation *source, double normal, double direct)
{
	STACKTRACE

	int i;
	i = Mountain::handle_damage(source, normal, direct);
	if (!health)
		state = 0;	// the object dies ;)
	return i;
}







SpaceObjectFrozen::SpaceObjectFrozen(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite, char *txt, bool otangible, int ohealth)
:
SpaceObject(creator, opos, oangle, osprite)
{
	strncpy(name, txt, 128);
	name[127] = 0;

	tangible = otangible;

	if (!tangible)
	{
		collide_flag_sameship = 0;
		collide_flag_sameteam = 0;
		collide_flag_anyone = 0;
	} else {
		collide_flag_anyone = ALL_LAYERS;
		collide_flag_sameship = ALL_LAYERS;
		collide_flag_sameteam = ALL_LAYERS;
	}

	health = ohealth;
	starthealth = health;
}

void SpaceObjectFrozen::drawpos(Frame *frame, int index, BITMAP **bmp, int *x, int *y)
{
	STACKTRACE

	(*bmp) = sprite->get_bitmap(index);

	double xgame, ygame;
	double	dxpos, dypos;

	xgame = space_center.x;
	ygame = space_center.y;

	int dx, dy;
	dx = frame->surface->w / 2;
	dy = frame->surface->h / 2;

	// and now in the top-left of the screen.
	xgame -= dx;
	ygame -= dy;

	dxpos = pos.x - iround(0.5*(*bmp)->w) - xgame;
	dypos = pos.y - iround(0.5*(*bmp)->h) - ygame;
	// the "iround" here is necessary, to synch the pixel position with that of the map
	// cause if you got half a pixel, and add a value, it'll be rounded off at other times 
	// than the integer-placed map - thus, it'll look like the sprite is moving 1 pixel
	// up and down all the time, which is weird. Note that for an even number of pixels,
	// the mid-value is always at half a pixel.
	
	while (dxpos >= 0.5*map_size.x)		dxpos -= map_size.x;
	while (dypos >= 0.5*map_size.y)		dypos -= map_size.y;
	while (dxpos < -0.5*map_size.x)		dxpos += map_size.x;
	while (dypos < -0.5*map_size.y)		dypos += map_size.y;
	
	(*x) = (int)dxpos;
	(*y) = (int)dypos;
}


void SpaceObjectFrozen::animate(Frame *frame)
{

	STACKTRACE

	int ixpos, iypos;
	BITMAP *bmp;

	drawpos(frame, sprite_index, &bmp, &ixpos, &iypos);

	if (ixpos + bmp->w >= 0 && ixpos < frame->surface->w &&
		iypos + bmp->h >= 0 && iypos < frame->surface->h)
	{
		masked_blit(bmp, frame->surface, 0, 0, ixpos, iypos, bmp->w, bmp->h);
	}

}


int SpaceObjectFrozen::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE

	double d = normal + direct;
	health -= d;
	if (health <= 0)
	{
		health = 0;
		//state = 0;		// it doesn't "die", cause by default, there's some debris left.
		tangible = false;	// no more interaction ...
		d = -health;	// that's left of the damage
	}
	return iround(d);
}


Creature::Creature(Plsurface *surf, Vector2 opos, double oangle, SpaceSprite *osprite, char *txt, bool otangible, int ohealth,
				   int ohabitat, int ovel, int odamage)
:
SpaceObjectFrozen(0, opos, oangle, osprite, txt, otangible, ohealth)
{
	plsurface = surf;

	habitat = ohabitat;

	set_depth(DEPTH_ASTEROIDS);
	layer = LAYER_CBODIES;
	
	attributes |= ATTRIB_COLLIDE_STATIC;

	mass = 10;

	time = 0;
	turntime = 1.0;

	damage = odamage;
	v = scale_velocity(ovel);

	sprite_index = 0;
	dindex = 0;
	toggledir = true;
	dist = 0;
}


void Creature::calculate()
{
	STACKTRACE

	SpaceObject::calculate();

	// run around aimlessly...

	time += frame_time * 1E-3;
	if (time > turntime)
	{
		time -= turntime;
		angle = random(PI2);

		vel = v * unit_vector(angle);

		angle = vel.atan();

	}

	// depending on distance travelled, switch frames:
	dist += v * frame_time;
	if (dist > 25)
	{
		dist -= 25;

		if (dindex >= sprite->frames()-64)
			toggledir = false;	// down
		else if (dindex <= 0)
			toggledir = true;	// up

		if (toggledir)
			dindex += 64;
		else
			dindex -= 64;

	}

	sprite_index = get_index(angle) + dindex;



	int col;
	col = plsurface->surf_color((int)pos.x, (int)pos.y);


	// check the background:
	bool inwater = (col == makecol(20,5,129));
	if (  (habitat == 1 && inwater)  ||		// 1 = land creature
		  (habitat == 2 && !inwater)  )		// 2 = water creature
	{
		pos -= vel * frame_time;	// return to previous position
		vel = -vel;					// turn around.
	}

	return;
}


void Creature::inflict_damage(SpaceObject *other)
{
	STACKTRACE

	// only damage shiptype objects, not objects that are on the ground
	if (other->isShip())
		other->handle_damage(this, damage);
}


int Creature::handle_damage (SpaceLocation *source, double normal, double direct)
{
	STACKTRACE

	int i;
	i = SpaceObjectFrozen::handle_damage(source, normal, direct);
	if (!health)
		state = 0;	// the object dies ;)

	return i;
}





Mountain::Mountain(Plsurface *surf, Vector2 opos, double oangle, SpaceSprite *osprite, char *txt, bool otangible, int ohealth)
:
SpaceObjectFrozen(0, opos, oangle, osprite, txt, otangible, ohealth)
{
	plsurface = surf;

	this->set_depth(DEPTH_ASTEROIDS);
	this->layer = LAYER_CBODIES;
	
	attributes |= ATTRIB_COLLIDE_STATIC;

	mass = 1E99;

	id = ID_ASTEROID;
}

void Mountain::calculate()
{
	STACKTRACE

	// make a decision about which sprite to draw... based on health

	int n;
	n = sprite->frames();
	if (n == 1)
		sprite_index = 0;
	else
	{
		// the last one is the "intangible" sprite - of the remains of the dead object...
		// is nice if this is still animated ?

		sprite_index = int((n-1) * (1 - health/starthealth));
		// only if health==0, does this become exactly n-1.
	}

	return;
}

// ------------------ NEW VIEW CLASS -----------------------
// this one is the most basic one possible
// it always has space_zoom = 1.



void View_Frozen::init(View *old)
{
	STACKTRACE

	View::init(old);
	f = 0;
	min = 120;//480;
	max = 4800;
	return;
	}

void View_Frozen::prepare ( Frame *frame, int time )
{
	STACKTRACE


	Vector2 oc = camera.pos;
	camera.pos += camera.vel * time;

	frame->prepare();

	::space_view = this;

	view_size.x = window->w;
	view_size.y = window->h;

	::space_view_size = view_size;
	::space_zoom = 1.0;
	::space_mip = -log(space_zoom) / log(2);
	::space_mip_i = iround_down(::space_mip);
	::space_size  = view_size * space_zoom;

	::space_corner = normalize2(camera.pos - space_size /2, map_size);
	::space_center = normalize2(camera.pos, map_size);
	::space_vel = camera.vel;
	::space_center_nowrap = camera.pos;
	
	camera.pos = oc;

	return;
}

void View_Frozen::calculate (Game *game)
{
	STACKTRACE

	CameraPosition n = camera;
	/*
	if (key_pressed(key_zoom_in))  n.z /= 1 + 0.002 * frame_time;
	if (key_pressed(key_zoom_out)) n.z *= 1 + 0.002 * frame_time;
	if (n.z < min) n.z = min;
	if (n.z > max) n.z = max;
	if (key_pressed(key_alter1)) f += 0.008 * frame_time;
	else f -= 0.008 * frame_time;
	if (f < 0) f = 0;
	if (f > 1.2) f = 1.2;
	*/
	SpaceLocation *c = NULL;
	if (game->num_focuses) c = game->focus[game->focus_index]->get_focus();
	focus ( &n, c );
//	if (c) {
//		n.pos += (f) * n.z / 4 * unit_vector(c->get_angle_ex());
//	}
	
	track ( n, frame_time );
	return;
}


REGISTER_VIEW ( View_Frozen, "Frozen" )

// ------------------ END OF THE VIEW CLASS ------------------






void gplexplr::init(Log *_log)
{
	STACKTRACE

	//you need to call Game::init very early on, to set stuff up... rarely do you want to do anything before that
	Game::init(_log);

	Plsurface *surf = new Plsurface("gmissionobjects.dat", "SURFACE_01_DAT");
	size = surf->size();

	prepare();

	add( surf );
	plsurface = surf;	// for global use ... well, within this particula gametype ... for edit mode, mainly

	//first, we create the teams.  This battle has just 2 sides; the human player, and the AI enemies
	TeamCode human, enemies;
	human = new_team();
	enemies = new_team();

	Control *c = create_control(channel_server, "Human");

	Ship *s = create_ship("plopl", c, 0.5*map_size, 0, human);
	//add(s->get_ship_phaser());
	add_focus(s);
	add(s);

//	Ship *e;
//	e = create_ship(channel_none, "kzedr", "WussieBot", Vector2(size.x/4,size.y/4), random(PI2), enemies);
//	add(e->get_ship_phaser());

	change_view("Frozen"); 

	view->window->locate(0,0,0,0,
		-PANEL_WIDTH,1,0,1);

	// initialize the environment bitmaps, and place these (if possible) in video
	// memory, since they'll be used a lot ! Actually, this initiailizes the whole map :)
	// (all except the interactive parts - for now).


	/*
	// add some creatures to the planet:
	int i;
	for ( i = 0; i < 20; ++i )
	{
		game->add(new Creature(NULL, random(map_size), 0.0, surf->object_spr[1]));
	}
	*/

}


void gplexplr::calculate()
{
	STACKTRACE

	Game::calculate();

	// this is required, because we only use a terrain bitmap of default pixel-size.
	// (thus it becomes resolution-dependent but ... what the heck ! )

	// override the redraw routine; let's redraw everything from scratch each frame.
	FULL_REDRAW = TRUE;

}



bool gplexplr::handle_key(int k)
{
	STACKTRACE

	Game::handle_key(k);

	switch ( k >> 8 )
	{
	case KEY_EQUALS:
		++plsurface->edittype;
		if (plsurface->edittype >= plsurface->Nobjecttypes)
			plsurface->edittype = 0;
		break;

	case KEY_MINUS:
		--plsurface->edittype;
		if (plsurface->edittype < 0)
			plsurface->edittype = plsurface->Nobjecttypes-1;
		break;

	case KEY_SPACE:
		plsurface->edit_add();
		break;
	}
	return false;
}



BITMAP* get_data_bmp(DATAFILE *data, char *objname)
{
	STACKTRACE

	DATAFILE *dataobj;
	BITMAP *r, *old;

	dataobj = find_datafile_object(data, objname);
	if (!dataobj)
		tw_error( "couldn't find data object [%s]!", objname);
	
	// copied from mshipdata.cpp:
	old = (BITMAP*)dataobj->dat;

//	// first, try to store this bitmap in video memory
//	r = create_video_bitmap(old->w, old->h);
//	// if this fails, store it in the normal memory
//	if (!r)
	// well, not useful in case you've a complete redraw - cause that complete
	// redraw is done in conventional memory ?!

	r = create_bitmap(old->w, old->h);
	blit (old, r, 0, 0, 0, 0, r->w, r->h);
	
	return r;
}



int istr;
char *strtxt;
void strset(char *t)
{
	istr = 0;
	strtxt = t;
}

bool strnumber(char c)
{
	// example:
	// +1.2E-3
	if (c >= '0' && c <= '9')
		return true;
	if (c == '.' || c == '+' || c == '-' || c == 'E' || c == 'e')
		return true;
	return false;
}

bool stralfa(char c)
{

	if (c >= '0' && c <= 'z')
		return true;
	return false;
}

void strskipgap()
{
	while (!strnumber(strtxt[istr]))
		++istr;
}

void strskipnonalfa()
{
	while (!stralfa(strtxt[istr]))
		++istr;
}
void strskipnum()
{
	while (strnumber(strtxt[istr]))
		++istr;
}

void strgetint(int *val)
{
	strskipgap();
	sscanf(&strtxt[istr], "%i", val);
	strskipnum();
}

void strline(char *t)
{
	STACKTRACE

	strskipnonalfa();

	int k;
	k = 0;

	while (strtxt[istr] != '\n' && strtxt[istr] != 0 )
	{
		t[k] = strtxt[istr];
		++k;
		++istr;
		if (k >= 512 - 1) break;
	}
	t[k] = 0;
}


Plsurface::Plsurface(char *datafilename, char *subdatafilename)
:
Presence()
{
	set_depth(DEPTH_STARS);


	// true, if you want to place or destroy (using the ship guns) objects and units (don't move);
	// false if you want to play.
	editmode = true;
	editmode = false;	// disable this line, if you want to use the edit mode.

	// initialize the bitmaps

	DATAFILE *dataobject = load_datafile_object(datafilename, subdatafilename);

	if (!(dataobject && dataobject->type == DAT_FILE))
	{
		tw_error("unable to load the plsurface subdata");
	}
	DATAFILE *data = (DATAFILE*) dataobject->dat;


	// first, read the info data:
	DATAFILE *d = find_datafile_object(data, "INFO_TXT");
	if ( !d )
	{
		tw_error("unable to load info_txt in plsurface subdata");
	}
	char *txt = (char*) d->dat;

	strset(txt);
	strgetint(&Ntiles);
	strgetint(&BitmapSize);
	strgetint(&Nx);
	strgetint(&Ny);

	if (Nx <= 0 || Ny <= 0)
	{
		tw_error("error in the map size");
	}

	// initialize the tile references
	terrain_index = new int [Ny*Nx];
	int i;
	for ( i = 0; i < Ny*Nx; ++i )
	{
		strgetint(&(terrain_index[i]));
		terrain_index[i] -= 1;				// cause in the txt file, 1 is the first map01, map02, ... map##.
	}

	// this concludes the info file.


	// initialize the map tiles
	terrain_tile = new BITMAP* [Ntiles];

	for ( i = 0; i < Ntiles; ++i )
	{
		char tilename[128];
		sprintf(tilename, "MAP%02i_BMP", i+1);
		terrain_tile[i] = get_data_bmp(data, tilename);
		if (!terrain_tile[i])
		{
			tw_error("Failed to load terrain bitmap !!");
		}
	}


	// initialize the objects (mountains, buildings, etc.)
	// needs to be a space-sprite, since it is collidable !
	strgetint(&Nobjecttypes);
	object_type = new ObjectProperty [Nobjecttypes];

	for ( i = 0; i < Nobjecttypes; ++i )
	{
		char t[512], bmpname[512], crot;
		int Nbmps, Nrot;
		strline(t);
		int tangible;
		sscanf(t, "%s %i %c %i  %i %i %i %i ", bmpname, &Nbmps, &crot,
					&object_type[i].habitat, &tangible,
					&object_type[i].vel, &object_type[i].damage,
					&object_type[i].health );
		object_type[i].tangible = (bool)tangible;

		strcpy(object_type[i].name, bmpname);

		strcat(bmpname, "_01_BMP");

		if (crot == 'r')
			Nrot = 64;
		else
			Nrot = 1;

		object_type[i].spr = new SpaceSprite(find_datafile_object(data, bmpname), Nbmps, 0 & SpaceSprite::MASKED, Nrot);	// mountain
		// the bmps should be ordered such, that they're immediately following the bmp labeled 01
		object_type[i].spr->change_color_depth(32);									// 0 &, otherwise the color correction doesn't work correctly ...

	}

	unload_datafile_object(dataobject);


	// place the objects on the map.
	Nobjects = 0;
	Nobjectsmax = 1024;		// some arbitrary number
	obj = new SpaceObjectFrozen* [Nobjectsmax];

	// keep track of which units are placed on the map by default (useful for edit mode)
	// note that units are added to the game (not to Plsurface), so those are treated
	// differently here.

	Nunits = 0;
	Nunitsmax = 1024;
	unit = new UnitProperty [Nunitsmax];

	editfilename = "gplexplr_mapobjects.txt";
	edit_read(editfilename);

	edittype = 0;
}


Vector2 Plsurface::size()
{
	return Vector2(Nx*BitmapSize, Ny*BitmapSize);
}


Plsurface::~Plsurface()
{
	if (editmode)
		edit_save(editfilename);

	delete[] terrain_index;

	int i;

	for ( i = 0; i < Ntiles; ++i )
		if (terrain_tile[i])
			destroy_bitmap(terrain_tile[i]);

	for ( i = 0; i < Nobjecttypes; ++i )
		if (object_type[i].spr)
			delete object_type[i].spr;		// delete each object

	for ( i = 0; i < Nobjects; ++i )
	{
		if (obj[i])
			delete obj[i];				// remove the mountains, and stuff ?
	}
		

	delete[] terrain_tile;
	delete[] object_type;		// delete the array of pointers
	delete[] obj;

}





int low_int(double d)
{
	if (d > 0)
		return int(d);
	else
		return int(d)-1;
}


void Plsurface::animate(Frame *frame)
{

	STACKTRACE

	// draw the surface !!
	int ix, iy, ix2, iy2, k;

	// which part should you draw ?

	// well ... you've to go from game coordinates (=center of the screen), to screen coordinates !!

	if ( fabs(space_zoom - 1.0) > 0.01 )
		tw_error("zoom isn't one !");
	
	// game coordinate which is at the center of the screen :
	double xgame, ygame;

	xgame = space_center.x;
	ygame = space_center.y;

	int ixmin, ixmax, iymin, iymax;

	int dx, dy;
	dx = frame->surface->w / 2;
	dy = frame->surface->h / 2;

	// and now in the top-left of the screen.
	xgame -= dx;
	ygame -= dy;

	ixmin = low_int((xgame) / BitmapSize);
	ixmax = low_int((xgame + frame->surface->w) / BitmapSize);

	iymin = low_int((ygame) / BitmapSize);
	iymax = low_int((ygame + frame->surface->h) / BitmapSize);

//	char m[128];
//	sprintf(m, "%2i %2i  %2i %2i  %i", ixmin, ixmax, iymin, iymax, int(xgame+frame->surface->w/2));
//	message.out(m);


	k = 0;
	for ( iy2 = iymin; iy2 <= iymax; ++iy2 )
	{
		for ( ix2 = ixmin; ix2 <= ixmax; ++ix2 )
		{
			ix = ix2;
			iy = iy2;

			
			while ( ix < 0 )	ix += Nx;
			while ( ix >= Nx )	ix -= Nx;
			while ( iy < 0 )	iy += Ny;
			while ( iy >= Ny )	iy -= Ny;
			
			k = iy*Nx + ix;	// in map

			int ixpos, iypos;	// in game
			ixpos = ix2 * BitmapSize - (int)xgame;	// relative to top-left of the screen
			iypos = iy2 * BitmapSize - (int)ygame;

			BITMAP *bmp = terrain_tile[terrain_index[k]];
			blit(bmp, frame->surface, 0, 0, ixpos, iypos, bmp->w, bmp->h);
			
		}
	}

	int i;
	for ( i = 0; i < Nobjects; ++i )
	{
		if (!obj[i]->state)
			continue;

		obj[i]->animate(frame);

	}

	if (editmode)
	{
		// show the edittype thingy ?
		// using the mouse position...
		if (edittype >= 0)
		{
			int ixpos, iypos;
			ixpos = mouse_x;
			iypos = mouse_y;
			
			BITMAP *bmp = object_type[edittype].spr->get_bitmap(0);
			masked_blit(bmp, frame->surface, 0, 0, ixpos-bmp->w/2, iypos-bmp->w/2, bmp->w, bmp->h);
		}
	}
	
}



void Plsurface::calculate()
{
	STACKTRACE

	
	// the ground objects aren't added to the game by themselves, for reason that they
	// can't collide with each other (they don't move), so... that takes away a lot of
	// possible collisions.

	// some stuff copied from:
	//void Physics::collide() {STACKTRACE
	// (but not exactly, I use the less advanced thing which was commented out).

	// shouldn't be in the calculate() function btw, but well ...
	int iobj;
	
	
	SpaceLocation **item;
	item = game->item;
	
	Query q;
	for ( iobj = 0; iobj < Nobjects; ++iobj )
	{
		if (!(obj[iobj]->tangible && obj[iobj]->health))
			continue;

		SpaceObject *o = (SpaceObject*) obj[iobj];

		// check for neighbouring (small) objects
		for (q.begin(o, OBJECT_LAYERS, (192 + o->get_size().x + o->get_size().y)/2); q.current; q.next())
		{
			o->collide(q.currento);
		}
		q.end();
	}
	

	// also, check if objects or units get deleted (sometimes happens)
	int i;
	for ( i = 0; i < Nobjects; ++i )
		if (!obj[i]->exists())
			delobj(i);

	// the following is only interesting in edit mode:
	for ( i = 0; i < Nunits; ++i )
		if (!unit[i].s->exists())
			delunit(i);


	// also, allow calculations for each object:
	// (this must be done here, since they're not added to the game manager)
	for ( i = 0; i < Nobjects; ++i )
		obj[i]->calculate();


}


void Plsurface::delobj(int i)
{
	STACKTRACE

	if (Nobjects <= 0)
		return;

	obj[i] = obj[Nobjects-1];
	--Nobjects;
}

void Plsurface::delunit(int i)
{
	STACKTRACE

	if (Nunits <= 0)
		return;

	unit[i] = unit[Nunits-1];
	--Nunits;
}

int Plsurface::surf_color(int x, int y)
{
	STACKTRACE

	int ix, iy;

	ix = x / BitmapSize;	// the ix-th tile to the right.
	x -= ix * BitmapSize;	// relative position within the tile.

	iy = y / BitmapSize;
	y -= iy * BitmapSize;

	int k;
	k = iy*Nx + ix;			// 1D map tile number

	BITMAP *bmp = terrain_tile[terrain_index[k]];	// the actual bitmap of this tile

	return getpixel(bmp, x, y);
}



bool Plsurface::edit_test(int i, char *txt)
{
	STACKTRACE

	return !strncmp(object_type[i].name, txt, strlen(txt));
}

// find the number belonging to the object of a certain name
int Plsurface::finditem(char *txt)
{
	STACKTRACE

	int i;
	for ( i = 0; i < Nobjecttypes; ++i )
	{
		if (!strcmp(object_type[i].name, txt))
			return i;
	}
	return -1;
}


void Plsurface::additem(Vector2 P, char *descr)
{
	STACKTRACE

	additem(P, finditem(descr));
}

void Plsurface::addobj(SpaceObjectFrozen *s)
{
	STACKTRACE

	if (Nobjects >= Nobjectsmax)
		return;

	obj[Nobjects] = s;
	++Nobjects;
}

void Plsurface::addunit(SpaceObjectFrozen *u)
{
	STACKTRACE

	if (Nunits >= Nunitsmax)
		return;

	unit[Nunits].s = u;
	++Nunits;
}

// add items based on some baseclasses (basenames starting with plant#, tree#, etc);
void Plsurface::additem(Vector2 P, int i)
{
	STACKTRACE

	// planta, plantb, plant...
	// treea, treeb, tree..
	if (edit_test(i, "plant") ||
		edit_test(i, "tree") ||
		edit_test(i, "building"))
		addobj(new Mountain(this, P, 0.0, object_type[i].spr, object_type[i].name,
							object_type[i].tangible, object_type[i].health));

	/*
	if (edit_test(i, "tree"))
		addobj(new Mountain(this, P, 0.0, object_type[i].spr, object_type[i].name,
							object_type[i].tangible, object_type[i].health));
	
	if (edit_test(i, "building"))
		addobj(new Mountain(this, P, 0.0, object_type[i].spr, object_type[i].name,
							object_type[i].tangible, object_type[i].health));
	*/
	
	if (edit_test(i, "critter"))
	{
		Creature *s;
		s = new Creature(this, P, 0.0, object_type[i].spr, object_type[i].name,
							object_type[i].tangible, object_type[i].health,
						object_type[i].habitat, object_type[i].vel, object_type[i].damage);
		if (editmode)
			s->v = 0;	// make it immobile...
		game->add(s);
		addunit(s);	// also add it to the list of placed items ... for storage on disk.
	}

	if (edit_test(i, "gunbody"))
	{
		GunBody *s;
		int k;
		
		k = finditem("gunshella");		// hmm, always a ?!
		s = new GunBody(this, P, 0.0,
						object_type[i].spr, object_type[k].spr, object_type[i].name,
							object_type[i].tangible, object_type[i].health,
						object_type[i].vel, object_type[k].vel, object_type[k].damage);
		game->add(s);
		addunit(s);	// also add it to the list of placed items ... for storage on disk.
	}
}


void Plsurface::edit_add()
{
	STACKTRACE

	Vector2 P;
	int i;

	if (Nobjects >= Nobjectsmax)
		return;

	i = edittype;

	P = space_center + Vector2(mouse_x - game->view->frame->surface->w/2, mouse_y - game->view->frame->surface->h/2);
	P = Vector2(int(P.x), int(P.y));


	additem(P, i);
}


// add all objs/units on the map that are specified on a file on disk
void Plsurface::edit_read(char *fname)
{
	STACKTRACE


	FILE *f;

	f = fopen(fname, "rt");
	if (!f)
		return;

	// read all the object and unit types :
	while (!feof(f))
	{
		int ixpos, iypos, i;
		char descr[128];
		i = fscanf(f, "%i %i %s", &ixpos, &iypos, descr);
		
		if (i == EOF)
			break;

		additem(Vector2(ixpos, iypos), descr);
	}

	fclose(f);

}

// save all objs/units that are present on the map to a file on disk, overwriting all
// that's in the file with the information that's in memory.
void Plsurface::edit_save(char *fname)
{
	STACKTRACE

	FILE *f;
	int i;

	f = fopen(fname, "wt");
	if (!f)
		return;

	// the objects:
	for ( i = 0; i < Nobjects; ++i )
	{
		SpaceObjectFrozen *s;
		s = obj[i];
		fprintf(f, "%i %i %s\n", int(s->pos.x), int(s->pos.y), s->name);
	}

	// the units:
	for ( i = 0; i < Nunits; ++i )
	{
		SpaceObjectFrozen *s;
		s = unit[i].s;
		fprintf(f, "%i %i %s\n", int(s->pos.x), int(s->pos.y), s->name);
	}


	fclose(f);
}


int Plsurface::countobjects(char *idname)
{
	STACKTRACE

	int i, count;

	count = 0;
	for ( i = 0; i < Nobjects; ++i )
	{
		if (obj[i]->health)
			if (strcmp(obj[i]->name, idname) == 0)
				++count;
	}

	return count;
}





SpaceObject *Plsurface::findobject(char *idname, int nskip)
{
	STACKTRACE

	int i, count;

	count = 0;

	for ( i = 0; i < Nobjects; ++i )
	{
		if (obj[i]->health)
			if (strcmp(obj[i]->name, idname) == 0)
			{
				++count;
				if (count == nskip)
					return obj[i];
			}
	}

	return 0;
}


// ok, this is for testing only.
// the game thingy should be removed, eventually.
REGISTER_GAME(gplexplr, "Planet Explorer")


