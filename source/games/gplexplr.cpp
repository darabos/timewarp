/*

  PLANET EXPLORER

based on gsample.cpp

- a tilemap of surface bitmaps forms the background - similar to stars

- mountains, buildings, other things you can collide with form objects and
   are initialized separately.



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



/* ------------------ STATIC OBJECTS -----------------

  mountains, buildings; things that don't need calculations of themselves, since
  they are completely passive.

*/

/*
class TerrainObject : public SpaceObject
{
};
*/

class Mountain : public SpaceObject
{
public:
	Mountain(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite);
	virtual void calculate();
	virtual void animate(Frame *space);
};


Mountain::Mountain(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite)
:
SpaceObject(creator, opos, oangle, osprite)
{

	this->set_depth(DEPTH_ASTEROIDS);
	this->layer = LAYER_CBODIES;
	
	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	
	attributes |= ATTRIB_COLLIDE_STATIC;

	mass = 1E99;

	id = ID_ASTEROID;

}

void Mountain::calculate()
{
	return;
}

void Mountain::animate(Frame *space)
{
	//return;
	SpaceObject::animate(space);
}


// ------------------ NEW VIEW CLASS -----------------------
// this one is the most basic one possible
// it always has space_zoom = 1.


class View_Frozen : public View
{
	double f;
	double max, min;
	public:
	virtual void calculate(Game *game);
	virtual void init(View *old);
	virtual void prepare ( Frame *frame, int time );
	//virtual void set_window (BITMAP *dest, int x, int y, int w, int h);
	};

void View_Frozen::init(View *old)
{
	View::init(old);
	f = 0;
	min = 120;//480;
	max = 4800;
	return;
	}

void View_Frozen::prepare ( Frame *frame, int time )
{

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






class gplexplr : public Game
{ //declaring our game type
	virtual void init(Log *_log) ; //happens when our game is first started
	virtual void calculate();
	
	public:
		double ZoomDefault;	// is camera.z, see struct CameraPosition {
};


const int BitmapSize = 128;		// slightly bigger than 1 ship.

// this manages the drawing of the (planet) background ; it also initializes the map ?
class Plsurface : public Presence
{
public:
	BITMAP	**terrain_tile;
	SpaceSprite **object_spr;
	int		*terrain_index, *object_index, Nx, Ny, Ntiles, Nobjects;

	Plsurface();
	~Plsurface();
	void calculate();
	void animate(Frame *frame);
};


BITMAP* get_data_bmp(DATAFILE *data, char *objname)
{
	DATAFILE *dataobj;
	BITMAP *r, *old;

	dataobj = find_datafile_object(data, objname);
	if (!dataobj)
		tw_error( "couldn't find data object [%s]!", objname);
	
	// copied from mshipdata.cpp:
	old = (BITMAP*)dataobj->dat;
	r = create_bitmap(old->w, old->h);
	blit (old, r, 0, 0, 0, 0, r->w, r->h);
	
	return r;
}



Plsurface::Plsurface()
:
Presence()
{
	set_depth(DEPTH_STARS);

	// initialize the bitmaps, and store them in video-memory if possible

	DATAFILE *data = load_datafile("gterrain.dat");

	Ntiles = 1;
	terrain_tile = new BITMAP* [Ntiles];

	terrain_tile[0] = get_data_bmp(data, "GRASSa_00");
	if (!terrain_tile[0])
		tw_error("Failed to load terrain bitmap !!");


	// initialize the objects (mountains, buildings, etc.)
	// needs to be a space-sprite, since it is collidable !
	Nobjects = 1;
	int Nterrain_objects = 1;
	object_spr = new SpaceSprite* [Nobjects];
	object_index = new int [Nterrain_objects];	// not correct ...
	object_spr[0] = new SpaceSprite(find_datafile_object(data, "MNTNa_00"), 1);	// mountain
	object_index[0] = 0;

	unload_datafile(data);


	// initialize the map properties

	Nx = 32;	// pretty big !
	Ny = 32;
	terrain_index = new int [Ny*Nx];
	int ix, iy;
	for ( iy = 0; iy < Ny; ++iy )
	{
		// initialize tiles
		for ( ix = 0; ix < Nx; ++ix )
		{
			terrain_index[iy*Nx+ix] = 0;	// use only 1 terrain-type for now.


		// initialize environment objects
		game->add( new Mountain(NULL, Vector2(ix*BitmapSize, iy*BitmapSize), 0.0, object_spr[0]) );
		}
	}

	// (re-)set the map size :
	map_size = Vector2(Nx*BitmapSize, Ny*BitmapSize);

}


Plsurface::~Plsurface()
{
	delete[] terrain_index;
	delete[] object_index;

	int i;
	for ( i = 0; i < Ny*Nx; ++i )
		destroy_bitmap(terrain_tile[i]);

	for ( i = 0; i < Nobjects; ++i )
		delete object_spr[i];		// delete each object
		

	delete[] terrain_tile;
	delete[] object_spr;		// delete the array of pointer
}

void Plsurface::calculate()
{
	// deal with interaction of map features with "space" objects
}





void gplexplr::init(Log *_log)
{
	//you need to call Game::init very early on, to set stuff up... rarely do you want to do anything before that
	Game::init(_log);

	//prepare needs to be called before you add items, or do physics or graphics or anything like that.  Don't ask why.  
	prepare();

	//first, we create the teams.  This battle has just 2 sides; the human player, and the AI enemies
	TeamCode human, enemies;
	human = new_team();
	enemies = new_team();

	//this creates a keyboard controller, using configuration 0 by default, since we don't specify another configuration
	Control *c = create_control(channel_server, "Human");
	//the first parameter is channel_server, which means that this player is on the server computer
	//the second parameter is "Human", the name of a controller type

	//this creates a ship
	Ship *s = create_ship("plopl", c, Vector2(size.x/2,size.y/2), 0, human);
	//the 1st parameter is a 5-letter ship code.  e.g. plopl = Ploxis Plunderer, andgu = Androsynth Guardian, chmav = Chmmr Avatar, estgu = Estion Gunner, tauhu = Tau Hunter, thrto = Thraddash Torch
	//the 2nd parameter is the controller for the ship.  
	//the 3th parameter is the positition where it appears.  
	//the 4th parameter is the angle that the ship starts out facing (in radians), 0 in this case, meaning that it points to the right
	//Note that angles in radians range from 0 to 2 * PI (6.283...), unlike angles in degrees which range from 0 to 360
	//You can convert degrees to radians by multiplying by ANGLE_RATIO
	//the 5th parameter is the team it's on... this parameter is optional... if we leave it out, then it's on it's own team of 1, and everyone is its enemy

	//I should talk more about the 3rd parametere there, the position
	//The idea of the position is simple: an X coordinate and a Y coordinate
	//The X & Y coordinates indicate a point in space.  
	//The ship is placed with its center at that point.  
	//However, even though it's 2 coordinates, it's only one parameter
	//That's because create_ship takes a 2-dimensional vector (the type for which is called Vector2 in TimeWarp)
	//the vector contains the X & Y coordinates.  
	//There are many other functions in TimeWarp that take vectors. 
	//When you want to pass one of those functions a constant like x=13.5,y=7.9, you give them Vector2(13.5, 7.9)
	//When you recieve a Vector2 and want to know what the x or y component of it are , you append .x or .y to the end of it to get those components
	//In the above example I said "Vector2(size.x/2,size.y/2)"
	//but it would have been equivalent to just say "size/2"

	//this causes the ship to warp into the game.  note that we call add() on the phaser, not the ship, because the phaser will add the ship once it finishes warping in
	add(s->get_ship_phaser());
	//if we wanted the ship to just appear without warping in, then we would say "add(s);" instead

	//this causes the human ship to be a focus for the camera
	add_focus(s);

	//now, we create some enemies to fight

	Ship *e;
	e = create_ship(channel_none, "kzedr", "WussieBot", Vector2(size.x/4,size.y/4), random(PI2), enemies);
	//This is a different syntax for create_ship
	//It creates a ship and an AI for it at the same time.  
	//AIs created in this way are automatically destroyed when their ship dies
	//Notice how it takes the parameters normally taken by both a create_control and a create_ship
	//Anyway, this creates a Kzer-Za Dreadnought, and WussieBot AI to control it
	//This ship starts facing a random direction from 0 to 2 * PI radians, because of the "random(PI2)"
	//PI2 is shorthand for 2 * PI in timewarp.  In timewarp, random(x) will produce a random number from 0 to x
	//Notice that it's on channel channel_none.  This mean that the AI is considered part of physics and uses no bandwidth in network games
	//When AIs use channel_none they are not effected by network lag.  
	//You can also use a channel_server or channel_client, to locate the AI on the server or client computer.  
	//If you do so the AI will experience lag and use network bandwidth just like a human player
	add(e->get_ship_phaser());
	/*
	e = create_ship(channel_none, "kohma", "WussieBot", Vector2(size.x*3/4,size.y/4), random(PI2), enemies);
	//here we add a Kohr-Ah Marauder
	add(e->get_ship_phaser());
	e = create_ship(channel_none, "druma", "WussieBot", Vector2(size.x*3/4,size.y*3/4), random(PI2), enemies);
	//here we add a Druuge Mauler
	add(e->get_ship_phaser());
	e = create_ship(channel_none, "yehte", "WussieBot", Vector2(size.x/4,size.y*3/4), random(PI2), enemies);
	//here we add a Yehat Terminator
	add(e->get_ship_phaser());
	*/


	//BTW, this is a vicious combination of enemies.  
	//Yehat for direct assault, Druuge for long range support, 
	//and the Urquans to make life miserable


	change_view("Frozen"); 
	//sets it to Hero view mode


	view->window->locate(0,0,0,0,
		-PANEL_WIDTH,1,0,1);
	//we tell the main window to occupy the screen region to the left of the ship panels

	friendly_fire = 0;
	//here we override the normal settings for friendly fire to make sure that human players can't hurt each other


	message.out("Welcome to Sample Game 1: Arena!", 3500, 15);
	//display a welcoming message to the player
	//the message is displayed for 3500 milliseconds (3.5 seconds)
	//in color 15 of the standard TW palette, which is white


	// initialize the environment bitmaps, and place these (if possible) in video
	// memory, since they'll be used a lot ! Actually, this initiailizes the whole map :)
	// (all except the interactive parts - for now).

	game->add(new Plsurface());
	}


void gplexplr::calculate()
{
	Game::calculate();

	// this is required, because we only use a terrain bitmap of default pixel-size.
	// (thus it becomes resolution-dependent but ... what the heck ! )

	// override the redraw routine; let's redraw everything from scratch each frame.
	FULL_REDRAW = TRUE;
}



void Plsurface::animate(Frame *frame)
{


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

	ixmin = int((xgame - dx) / BitmapSize);
	ixmax = int((xgame + dx) / BitmapSize);

	iymin = int((ygame - dy) / BitmapSize);
	iymax = int((ygame + dy) / BitmapSize);


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
			ixpos = ix2 * BitmapSize - (xgame - dx);	// relative to top-left of the screen
			iypos = iy2 * BitmapSize - (ygame - dy);

			BITMAP *bmp = terrain_tile[terrain_index[k]];
			blit(bmp, frame->surface, 0, 0, ixpos, iypos, bmp->w, bmp->h);
			
		}
	}
}






REGISTER_GAME(gplexplr, "Planet Explorer") //registering our game type, so that it will appear in the menus

