/*
Fight between a player fleet (set in the ini file) and a random adversary fleet (never
equal to the player's fleet).

  To-do:

  pointer ImIndicators

  array sizes

  ship panel



about identifying the killer (for his stats):

orzagonal: he Ship may have been destroyed already
orzagonal: but he still has a missile or something around
orzagonal: to kill you with
(...)
GeomanNL: does the weapon carry a copy of the ships id?
orzagonal: yes
orzagonal: it's member variable ally_flag
(...)
GeomanNL: which classes carry this id?
GeomanNL: spaceobjects ?
orzagonal: SpaceLocation
(...)
orzagonal: you can look at the whole ally_flag
GeomanNL: ok
orzagonal: and that's a unique identifier for the ship that spawed it

*/

#include <allegro.h>
#include <stdio.h>
#include <string.h>
#include "../melee.h"
#include "../other/radar.h"
#include "../melee/mshppan.h"

REGISTER_FILE
#include "../melee/mframe.h"
#include "../melee/mgame.h"
#include "../melee/mmain.h"
#include "../melee/mcbodies.h"
#include "../melee/mview.h"
#include "../other/orbit.h"
#include "../melee/mitems.h"
#include "../util/aastr.h"
#include "../melee/mcontrol.h"
//#define STATION_LOG

#include "gflmelee.h"

#define Num_Planet_Pics 7



void copy_sprite2bmp(SpaceSprite* src, BITMAP *bmp)
{
	src->lock();
	
	BITMAP *tmp = src->get_bitmap(0);
	aa_stretch_blit(tmp, bmp, 0, 0, tmp->w, tmp->h,
								0, 0, bmp->w, bmp->h);
	
	src->unlock();
}

// show the direction to some object, by showing a small sprite
// if it;s a location, show only a default thingy ?!
static int ImIndicatorSize = 40;
class ImIndicator : public Presence
{
public:
	BITMAP			*bmp;
	SpaceLocation	*showme;

	ImIndicator(SpaceLocation *o);
	~ImIndicator();
	void newtarget(SpaceLocation *o);
	int ImIndicator::colortype(SpaceLocation *o);
	void animate(Frame *frame);
};



extern int PANEL_WIDTH;
extern int PANEL_HEIGHT;
extern int CAPTAIN_X;
extern int CAPTAIN_Y;
extern int PANEL_DEATH_FRAMES;
extern int crew_x;
extern int crew_y;
extern int batt_x;
extern int batt_y;



// most of this is copied from ZRadar.
class YRadar : public Presence
{
	public:
	BITMAP	*ship_f_icon, *ship_e_icon, *cbody_icon, *planet_icon,
			*target_icon1, *target_icon2, *backgr_bmp, *foregr_bmp;
	TeamCode	homeplayer_team;

	Vector2	location;	// where it's drawn on the screen

	YRadar(Control *ocontroller, double Size, TeamCode hteam, DATAFILE *dat,
			bool rel, int shape);
	~YRadar();

	// the code for painting on the radar screen is in here.
	void PaintItem(BITMAP *Slate, Vector2 T, SpaceLocation *o, double Scale);
	virtual void Paint(BITMAP *Slate, Vector2 T);
	virtual void animate(Frame *space);

	BITMAP		*Painted;
	Control		*controller;
	double size;
	bool active;

	bool	relative_angle;	// 0=absolute, 1=rotated in orientation direction
	int		display_shape;	// 1=square, 2=round.

//	double shiftscale(double r_center, double v_center, double scale, double n);
	Vector2 shiftscale(Vector2 r_center, Vector2 v_center, double scale, Vector2 n);

	void setTarget(SpaceLocation *target);
	void setSize(double Size);
	void toggleActive();
};

static const int StatsMax_Nships = 128;
static const int StatsMax_Nkilled = 32;

// note, this is not part of the game, really - just accepts neutral info
class StatsManager
{
	BITMAP	*list_bmp[StatsMax_Nships];	// at most 512 interesting different objects (ships)?
	int		Nlist, list_allyflag[StatsMax_Nships];

	public:
	int Nships;
	struct stats_str
	{
		int Nkilled;
		int Npressedfire;
		int Npressedspecial;
		// ID's that are unique to a ship and the weapons it spawns (up to SpaceLocation)
		unsigned int allyflag_owner, allyflag_killed[StatsMax_Nkilled];
		//Ship *owner, *killed[32];
//		ShipData *ownerdata, *killeddata[32];
//		BITMAP *ownerbmp, *killedbmp[32]; --> is now in a centralized list
	} stats[StatsMax_Nships];	// at most 64 ships in the game?

	StatsManager();
	~StatsManager();
	void addship ( Ship *statship, int ofordisplay );	// add to the stats list
	void updatestats(SpaceLocation *killer, Ship *victim);
	void showstats(Frame *frame);
	int list_item(int flag);
};



static const int Nradarmodes = 3;	// off, all, and medium

class FlMelee : public Game
{
public:

	DATAFILE	*FlmeleeData;

	double	radar_sizes[Nradarmodes];
	int		radar_mode;
	BITMAP *blankRadar;						//Image of the blank RADAR screen
	YRadar	*radar;

	ImIndicator *target_indicator;
	int healthbartoggle;
	char alliancename[2][512];
	TeamCode alliance[2];
	Control *playercontrols[2];
	Control *oldcontrol;	// remember the original control when you take possession of a ship
	ShipPanelBmp *player_panel[2];

	int		toggle_showstats, toggle_playership, toggle_healthbars, toggle_fleetlist, toggle_radar, toggle_panel, toggle_radarpos;
	int		lastkey_playership, lastkey_healthbars, lastkey_fleetlist, lastkey_radar, lastkey_panel, lastkey_radarpos;
	StatsManager	*statsmanager;

	int radar_pos_id;

	~FlMelee();

	SpaceSprite *HugePics[Num_Planet_Pics];

	virtual void init(Log *_log);

	bool GetSprites(SpaceSprite *Pics[], char *fileName, char *cmdStr, int numSprites, int attribs);
	SpaceSprite *GetSprite(char *fileName, char *spriteName, int attribs);

	virtual void calculate();
	virtual void animate( Frame* frame );
	virtual void animate_predict(Frame *frame, int time);

	virtual void ship_died(Ship *who, SpaceLocation *source);

	int is_in_team(SpaceLocation *o, TeamCode team);
	void show_ending(int didwewin);
	void animate_onscreen_shiplist( Frame* frame );
};



HealthBar::HealthBar(Ship *creator, int *toggle)
{
	scale = 2.0;
	bartoggle = toggle;
	mother = creator;
}

void HealthBar::calculate()
{
	if ( !(mother && mother->exists()) )
	{
		state = 0;
		return;
	}

}


void HealthBar::draw_bar(Ship *s, double yoffs, int len, int H, double fraction, int col1, int col2, Frame *space)
{
	Vector2 center;
	int d;

	H = iround(H * space_zoom);
	if (H < 1)
		H = 1;			// minimum thickness.

	len = iround(len * space_zoom);	// scale

	center = corner(s->pos);	// scales and shifts onto screen coord. of the (center of the) ship

	int ix, iy;
	ix = iround(center.x - len/2);
	iy = iround(center.y - (0.6 * s->size.y + H/2 + yoffs) * space_zoom);
	
	d = iround(len * fraction);

	H -= 1;		// for plotting, pixel 0 also counts
	if (d > 0)
		rectfill(space->surface, ix, iy, ix+d-1, iy+H, col1);
	rectfill(space->surface, ix+d, iy, ix+len, iy+H, col2);

	space->add_box(ix, iy, ix+len, iy+H);
}


void HealthBar::animate(Frame *space)
{
	if (!*bartoggle)
		return;

	if (mother->isInvisible())
		return;

	int H = 2;
	int dy = 4;

	if ((dy - H/2) * space_zoom < 1)
		dy = H/2 + 1/space_zoom;


	draw_bar(mother, -dy, mother->crew_max * scale, H, mother->crew/mother->crew_max,
		makecol(0, 255, 0), makecol(150, 0, 0), space);

	draw_bar(mother,  dy, mother->batt_max * scale, H, mother->batt/mother->batt_max,
		makecol(255, 50, 50), makecol(150, 0, 0), space);

}



SpaceSprite *FlMelee::GetSprite(char *fileName, char *spriteName, int attribs)
{

	DATAFILE *tmpdata;
	tmpdata= load_datafile_object(fileName,spriteName);
	if(tmpdata==NULL)
	{
		#ifdef STATION_LOG
			sprintf(msgStr,"Unable to load %s#%s",fileName,spriteName);
			message.out(msgStr);
		#endif

		return NULL;
	}

//	#ifdef STATION_LOG
//		sprintf(msgStr, "Succesfully loaded %s#%s!  Hot damn!",fileName,spriteName);
//		message.out(msgStr);
//	#endif

	SpaceSprite *spr=new SpaceSprite(tmpdata, 1, attribs, 1);
	unload_datafile_object(tmpdata);

	return spr;
}

bool FlMelee::GetSprites(SpaceSprite *Pics[], char *fileName, char *cmdStr, 
int numSprites, int attribs)
{

	SpaceSprite *spr;
	char dataStr[100];

	for(int num=0; num<numSprites; num++)
	{
		sprintf(dataStr,cmdStr,num);
		spr=GetSprite(fileName, dataStr, attribs);
		if(!spr)
		{
			return FALSE;
		}

		Pics[num]=spr;
	}

	return TRUE;
}

void FlMelee::init(Log *_log)
{

	Game::init(_log);

	//prepare needs to be called before you add items, or do physics or graphics or anything like that.  Don't ask why.  
	prepare(); 


	// set the anti-alias to 0:
	int antia_on =  get_config_int(NULL, "Anti-AliasingOn",0);;
	set_tw_aa_mode(antia_on);


	// load the sprite, but set the anti-alias to 0 for them - they're _big_:
	if(GetSprites(HugePics,"plhuge.dat","Station_Planet%03d",Num_Planet_Pics,
		SpaceSprite::MASKED | SpaceSprite::NO_AA)==FALSE)
		error("File error, planet pics.  Bailing out...");


	//opening your .ini file.

	log_file("plhuge.ini");
	int Comets = get_config_int(NULL, "Comets",0);
	int CoMass = get_config_int(NULL, "Comet_mass",0);
	int ComMax = get_config_int(NULL, "Comet_max",0);
	int Asteroids=get_config_int(NULL,"Asteroids",0);
	int ComAcc = get_config_int(NULL, "Comet_acc",0);

//	int MapSize=get_config_int(NULL,"MapSize",0);
	int MapSize = 15000;	// somewhat bigger :)
	size.x = size.y = MapSize;
	iMessage("Size   = %d *PRESET*",(size.x));

	int PlanetType=get_config_int(NULL,"PlanetType",0);
	
	//Make sure variables being read properly	
	iMessage("Comets    = %d",Comets);
	iMessage("CoMass    = %d",CoMass);
	iMessage("ComMax    = %d",ComMax);
	iMessage("Asteroids = %d",Asteroids);
	iMessage("ComAcc    = %d",ComAcc);
	iMessage("MapSize   = %d",MapSize);
	iMessage("PlanetType= %d",PlanetType);


	add(new Stars());
//	char starName[100];

	
//Select planet, make 2 enormous planets, one on 1 side, the other on the other side.

	Planet *Planets[2];

	int rememberPlanetType = PlanetType;
	for ( int k = 0; k < 2; ++k )
	{
		Vector2 P;

		if ( k == 0 )
			P = Vector2(MapSize/2, -MapSize/4);
		else
			P = Vector2(MapSize/2,  MapSize/4);

		Planet *Centre;
		if (rememberPlanetType == -1) {
			PlanetType = random()%Num_Planet_Pics;
			iMessage("PlanetType= %d *RANDOM*",PlanetType);
		}
		else iMessage("PlanetType= %d *PRESET*",PlanetType);

		Centre = new Planet(P, HugePics[PlanetType],0);
		game->add(Centre);
		game->add(new ImIndicator(Centre));
		Planets[k] = Centre;
		
		// copied from Planet::Planet :
		// note that Planet constructor calls another log file.
		log_file("plhuge.ini");
		Centre->gravity_mindist = scale_range(get_config_float("GPlanet", "GravityMinDist", 0));
		Centre->gravity_range = scale_range(get_config_float("GPlanet", "GravityRange", 0));
		Centre->gravity_power = get_config_float("GPlanet", "GravityPower", 0);
		Centre->gravity_force = scale_acceleration(get_config_float("GPlanet", "GravityForce", 0), 0);
		Centre->gravity_whip = get_config_float("GPlanet", "GravityWhip", 0);
		Centre->gravity_whip2 = get_config_float("GPlanet", "GravityWhip2", 0);
	}


	int num;

	//comet code
	for (num = 0; num < Comets; num++)
	{
		Planet *c;
		c = new Planet (random(size), kaboomSprite, 1);
		c->id=COMET_ID;
		c->mass = CoMass;
		c->gravity_force *= 0;
		c->gravity_whip = 0;
		c->accelerate(NULL, random(PI2), get_config_int(NULL, "Comet_acc", 2), 
		get_config_int(NULL, "Comet_max", 2));
		add (c);
	}

	//asteroids code
	Asteroids = get_config_int(NULL, "Asteroids", 0);
	if (Asteroids > 0)
	{
		for (int num = 0; num < Asteroids; num += 1)
		{
			Asteroid *a = new Asteroid();
			add(a);
		}
	} else {
		 for (int i = 0; i < 4; i += 1)
			 add(new Asteroid());
	}



	// initialize the teams and the fleets :)

	log_file("gflmelee.ini");

	int Nfleets;
	int allyfleet[2], iplayer;		// there are 2 players

	Nfleets = get_config_int("FleetInit", "Nalliances", 0);

	allyfleet[0]  = get_config_int("FleetInit", "PlayerAlliance", 0);
	if ( allyfleet[0] == -1 )
		allyfleet[0] = tw_random()%Nfleets + 1;

	allyfleet[1]  = get_config_int("FleetInit", "EnemyAlliance", 0);
	if ( allyfleet[1] == -1 )
	{
		allyfleet[1] = allyfleet[0] + tw_random()%(Nfleets-1) + 1;
		if ( allyfleet[1] > Nfleets )
			allyfleet[1] -= Nfleets;
	}


	statsmanager = new StatsManager;


	alliance[0] = new_team();
	alliance[1] = new_team();

	playercontrols[0] = 0;
	playercontrols[1] = 0;

	Ship *takeovership;

	for ( iplayer = 0; iplayer < 2; ++iplayer )
	{

		char ident[512];
		int Nships;

		sprintf(&ident[0], "Alliance%02i", allyfleet[iplayer]);	// >= 1
		message.out(ident);

		Nships = get_config_int(ident, "Nships", 0);
		iMessage("Nships = %i", Nships);

		strcpy(alliancename[iplayer], get_config_string(ident, "Name", 0) );

		int iship;
		for ( iship = 0; iship < Nships; ++iship )
		{
			char identshp[512] = "";
			sprintf(identshp, "ship%02i", iship + 1);

			message.out(ident);
			char test[512];
			sprintf(test, "[%s]", identshp);
			message.out(test);

			// note, we've to store shipname in memory, cause
			// the ini operations in the ship-init overwrite value in st.
			char shipname[512];
			const char *st;
			st = get_config_string(ident, identshp, 0);
			if (!st)
			{
				message.out("Couldn't find ship entry in list, name=NULL");
				continue;
			}

			strcpy(shipname, st);

			message.out(shipname);


			// add the ships to the game, note that the first ship of the first fleet
			// belongs to the human player.

			// place ships in a circle around the planet

			
			double R, a;
			Vector2 P;

			a = iship * PI2 / Nships;
			R = 1500.0;

			if ( iplayer == 0 )
				P = Planets[iplayer]->pos + R * unit_vector(a-PI/2);
			else
				P = Planets[iplayer]->pos + R * unit_vector(a+PI/2);

			Ship *s;

			if ( iplayer == 0 && iship == 0 )
			{
				Control *c  = create_control(channel_server, "Human");
				playercontrols[iplayer] = c;
				// see gsample - this tracks the player position, whatever ship
				// he controls ;)
				add_focus(c, c->channel);
			}


			//s = create_ship(shipname, "Wussiebot", P, a, alliance[iplayer]);
			s = create_ship(channel_none, shipname, "WussieBot", P, a, alliance[iplayer]);
			add(s->get_ship_phaser());

			add(new HealthBar(s, &healthbartoggle));


			// do not add this to the game:
			statsmanager->addship(s, 1);

			if ( iplayer == 0 && iship == 0 )
				takeovership = s;
			

			// have to do this, since the ship-init loads different ini files.
			// and it's best placed here, right after the last call to a different ini.
			log_file("gflmelee.ini");


		}
	}

	playercontrols[0]->set_target(-1);
				

	oldcontrol = takeovership->control;	// remember :)
	playercontrols[0]->select_ship(takeovership, "none");

	healthbartoggle = 1;	// show health bars for the ships.

	//if (num_targets != 14 ) {tw_error("Oh man ...");} OK this works fine !

//	if (playercontrols[0]->target)
//	{
//		target_ImIndicator = new WedgeImIndicator(playercontrols[0]->target, 100, makecol(0,250,0));
//		game->add(target_ImIndicator);
//	} else
		target_indicator = 0;



	toggle_showstats = 0;
	toggle_playership = 0;
	toggle_healthbars = 1;
	toggle_fleetlist = 1;
	toggle_radar = 0;
	toggle_panel = 1;
	toggle_radarpos = 0;
	lastkey_playership = 0;
	lastkey_healthbars = 0;
	lastkey_fleetlist = 0;
	lastkey_radar = 0;
	lastkey_panel = 0;
	lastkey_radarpos = 0;

	//BITMAP *BlankSlate, Presence *target, double Size
//	blankRadar = GetBitmap(VanDat,"Scope");		//Load blank RADAR image
//	blankRadar = create_bitmap(200, 200);
//	clear_to_color(blankRadar, makecol(100,100,100));
//	if (!blankRadar) error("Can't load scope image.");

	radar_sizes[0] = 100;		// is toggled off, then
	radar_sizes[1] = size.x;
	radar_sizes[2] = 4000;
	// this is a decreasing order - is best for fighting I think.

	radar_mode = 1;		// area of the map you see (is fitted onto the clean image)
//	TeamCode tc = playercontrols[0]->ship->get_team();
	FlmeleeData = load_datafile("gflmelee.dat");
	radar = new YRadar(playercontrols[0], radar_sizes[radar_mode], alliance[0], FlmeleeData, 0, 2);
	game->add(radar);
	radar_pos_id = 0;
	radar->location = Vector2(0, 0.5*window->surface->h - 100);

//	vradar->addTeam(human_team,makecol(65,255,128));	//A pleasant green
//	vradar->addTeam(enemy_team,makecol(255,65,0));		//A violent red
//	add(vradar);

	player_panel[0] = 0;
	player_panel[1] = 0;

}


int FlMelee::is_in_team(SpaceLocation *o, TeamCode team)
{
	return ((o->ally_flag & SpaceLocation::team_mask) == (team << SpaceLocation::team_shift));
}



void FlMelee::animate_onscreen_shiplist( Frame* frame )
{
	// info about the fleets

	
	if ( toggle_fleetlist )
	{
		Vector2 spos, ssize;
		
		spos = Vector2( frame->surface->w/2, 1);
		ssize = Vector2( text_length(font,alliancename[0]), text_height(font) );
		
		textprintf( frame->surface, font, spos.x, spos.y, pallete_color[15], alliancename[0]);
		frame->add_box(spos.x, spos.y, ssize.x, ssize.y);
		
		spos = Vector2( frame->surface->w/2, frame->surface->h-15);
		ssize = Vector2( text_length(font,alliancename[1]), text_height(font) );
		
		textprintf( frame->surface, font, spos.x, spos.y, pallete_color[15], alliancename[1]);
		frame->add_box(spos.x, spos.y, ssize.x, ssize.y);
		
		//textprintf( frame->surface, font, frame->surface->w/2, frame->surface->h-15, pallete_color[15], alliancename[1]);
		///*
		// also, show the fleet(s) and targets:
		int cnt[2], yoffs[2];
		cnt[0] = 0;
		cnt[1] = 0;
		yoffs[0] = 10;
		yoffs[1] = frame->surface->h-40;
		
		ssize = Vector2(30,30);
		
		for ( int itarget = 0; itarget < num_targets; ++itarget )
		{
			SpaceObject *o = game->target[itarget];
			
			if ( o->isShip() )
			{
				Ship *shp = (Ship*) o;
				int iall;
				for ( iall = 0; iall < 2; ++iall )
				{
					if ( is_in_team(o, alliance[iall]) )
					{
						// show the ship-sprite, somewhere :)
						//textprintf( frame->surface, font, frame->surface->w/2, frame->surface->h-20-i*10, pallete_color[15], alliancename[1]);
						
						SpaceSprite *spr = o->get_sprite();
						// set transparency to indicate how alive it is ?
						double scale = 0.5;
						
						cnt[iall] += 1;
						
						Vector2 spos;
						
						spos = Vector2(cnt[iall]*50,yoffs[iall]);
						
						Control *c = playercontrols[0];	// you check from perspective of the human player = 0
						if (!( c && c->target && shp == c->target ))
						{
							int a;
							a = aa_get_trans();
							aa_set_trans(150 + 50*(1-shp->crew/shp->crew_max));		// 0 = solid
							spr->draw(spos, ssize, 0, frame);
							aa_set_trans(a);
						} else {
							int col = makecol(150, 30, 30);
							spr->draw_character(spos.x, spos.y, ssize.x, ssize.y, 0, col, frame);
							//spr->draw(spos, ssize, 0, frame);
						}
						
						
						int col1, col2;
						col1 = makecol(0, 255, 0);	// green = crew
						col2 = makecol(255, 0, 0);	// red = dead crew
						
						int ix, iy, h1, h2;
						iy = spos.y + 30;
						ix = spos.x - 5;
						h1 = shp->crew_max;
						h2 = shp->crew;
						line(frame->surface, ix, iy-h1, ix, iy-h2, col2);
						line(frame->surface, ix, iy-h2, ix, iy, col1);
						
						frame->add_line(ix, iy-h1, ix, iy);
						
					}
				}
				
			}
		}

	}	// end of toggle_fleetlist;
//*/
	if (toggle_showstats)
		statsmanager->showstats(frame);
//*/

	if (toggle_panel)
	{
		int ix, iy, iw, ih;
		BITMAP *src;
		BITMAP *dest = frame->surface;

		// show the ship panel(s)
		Ship		*s;
		SpaceObject	*t;
		int		i;

		i = 0;
		s = playercontrols[i]->ship;
		
		if (s)
		{
			if (!player_panel[i])
				player_panel[i] = new ShipPanelBmp(s);
			
			if (player_panel[i]->ship != s)
				player_panel[i]->ship = s;
			
			
			player_panel[i]->animate_panel();
		}

		src = player_panel[i]->panel;

		ix = dest->w - src->w;
		iy = 0;
		iw = src->w;
		ih = src->h;

		blit(src, dest, 0, 0 , ix, iy, iw, ih);
		frame->add_box(ix, iy, iw, ih);

		i = 0;
		t = playercontrols[i]->target;
		
		i = 1;
		if (t && t->exists() && t->isShip() )
		{
			if (!player_panel[i])
				player_panel[i] = new ShipPanelBmp((Ship*) t);
			
			if (player_panel[i]->ship != t)
				player_panel[i]->ship = (Ship*) t;
			
			
			
			player_panel[i]->animate_panel();
			
			src = player_panel[i]->panel;

			ix = dest->w - src->w;
			iy = dest->h - src->h;
			iw = src->w;
			ih = src->h;
			blit(src, dest, 0, 0 , ix, iy, iw, ih);
			frame->add_box(ix, iy, iw, ih);

		}

	}

}

void FlMelee::animate_predict(Frame *frame, int time)
{
	Game::animate_predict(frame, time);

	animate_onscreen_shiplist( frame );
}



void FlMelee::animate( Frame* frame )
{
	
	Game::animate( frame );

	animate_onscreen_shiplist( frame );
}


void FlMelee::calculate()
{
	Game::calculate();

	Control *c = playercontrols[0];

	// check death ...

	// not if the player is captain of one fixed vessel: a command ship
	int PlayerIsCaptain = 0;
	if ( !(c->exists() && c->ship->exists()) && !PlayerIsCaptain )
	{
		c->state = 1;

		for ( int itarget = 0; itarget < num_targets; ++itarget )
		{
			SpaceObject *o = game->target[itarget];
			
			if ( o->isShip() && o != c->ship && is_in_team(o, alliance[0]) )
			{
				Ship *shp = (Ship*) o;

				// the name doesn't really matter
				oldcontrol = shp->control;
				c->select_ship(shp, "none");	// also takes care of the ship's pointer :)

				break;
			}
		}
	}
	// if you're still dead after this check, the game ends
	if ( !(c->exists() && c->ship->exists()) )
	{
		show_ending(0);
	}

	// check if the target has changed:

	if ( (c->target && c->target->exists()) )
	{

		if ( !target_indicator )
		{
			target_indicator = new ImIndicator(c->target);
			game->add(target_indicator);
		}

		if ( c->target != target_indicator->showme )
		{
			target_indicator->newtarget(c->target);
		}

	} else {
		// choose a new target !
		int i;
		for ( i = 0; i < num_targets; ++i )
		{
			if (c->valid_target(game->target[i]))
			{
				c->set_target(i);
				break;
			}
		}

		SpaceObject *s = c->target;
		if ( s )
		{
			if ( !target_indicator )
			{
				target_indicator = new ImIndicator(s);
				game->add(target_indicator);
			}
			target_indicator->newtarget(c->target);
		}
	}
	if ( !c->target)
	{
//		{tw_error("what the ...?! We won! Hurray !");}
		// I guess you win ;)
		show_ending(1);
	}


	// jump to the next available ship if your ship dies

//	Control *c = playercontrols[0];


	if (key[KEY_S])
		toggle_showstats = 1;
	else
		toggle_showstats = 0;
	//toggle_showstats = 1;

	if (!lastkey_playership && key[KEY_V])
		toggle_playership = 1;
	else if (!lastkey_playership && key[KEY_C])
		toggle_playership = -1;
	else
		toggle_playership = 0;
	lastkey_playership = key[KEY_V] || key[KEY_C];

	// (...)
	// choosing targets is done elsewhere already

	if (!lastkey_healthbars && key[KEY_A])
		toggle_healthbars = 1 & ~toggle_healthbars;
	lastkey_healthbars = key[KEY_A];
	healthbartoggle = toggle_healthbars;

	if (!lastkey_fleetlist && key[KEY_D])
		toggle_fleetlist = 1 & ~toggle_fleetlist;
	lastkey_fleetlist = key[KEY_D];

	if (!lastkey_radar && key[KEY_R]  && !key[KEY_LSHIFT])
		toggle_radar = 1 & ~toggle_radar;
	lastkey_radar = key[KEY_R];

	if (toggle_radar)
	{
		toggle_radar = 0;

		++radar_mode;
		if ( radar_mode >= Nradarmodes )
			radar_mode = 0;

		radar->setSize(radar_sizes[radar_mode]);
	}
	if ( radar_mode != 0 )
		radar->active = 1;
	else
		radar->active = 0;


	if (!lastkey_radarpos && key[KEY_R] && key[KEY_LSHIFT])
		toggle_radarpos = 1 & ~toggle_radarpos;
	lastkey_radarpos = key[KEY_R] && key[KEY_LSHIFT];

	if ( toggle_radarpos )
	{
		toggle_radarpos = 0;

		++radar_pos_id;
		if ( radar_pos_id > 2 )
			radar_pos_id = 0;

		switch (radar_pos_id )
		{
		case 0:
			radar->location = Vector2(0, 0.5*window->surface->h - 100);
			break;
		case 1:
			radar->location = Vector2(0.5*window->surface->w-100, 0.5*window->surface->h - 100);
			break;
		case 2:
			radar->location = Vector2(window->surface->w-200, 0.5*window->surface->h - 100);
			break;
		}
	}


	if (!lastkey_panel && key[KEY_F])
		toggle_panel = 1 & ~toggle_panel;
	lastkey_panel = key[KEY_F];



	if ( playercontrols[0]->ship && toggle_playership )
	{
		// jump to the ship in this list:
		int itarget, lastitarget;

		itarget = 0;
		while ( game->target[itarget] != c->ship && itarget < num_targets-1 )
			++itarget;

		lastitarget = itarget;

		// then start searching for the next entry;
		itarget = lastitarget + 1;
		while ( itarget != lastitarget )
		{

			if ( itarget > num_targets-1 )
				itarget -= num_targets;
		
			SpaceObject *o = game->target[itarget];
			
//			Control *c = playercontrols[0];	// is already done earlier
			
			if ( o->isShip() && is_in_team(o, alliance[0]) )
			{
				Ship *shp = (Ship*) o;
				
				Control *c1, *c2;
				Ship *s1, *s2;

				c1 = shp->control;
				s1 = shp;

				c2 = playercontrols[0];
				s2 = c2->ship;

				oldcontrol->select_ship(s2, "none");	// re-assign original control ai.
				oldcontrol = s1->control;		// remember control ai of the new ship
				playercontrols[0]->select_ship(s1, "none");	// take over the new ship


				break;
			}

			++itarget;
		}

	}
	

	// check if the player panel(s) still exist (shouldn't be needed, since
	// the panel's calculate function isn't called):
	int i;
	for ( i = 0; i < 2; ++i )
	{
		if ( !(player_panel[i] && player_panel[i]->exists()) )
			player_panel[i] = 0;
	}

}


void FlMelee::ship_died(Ship *who, SpaceLocation *source)
{
	Game::ship_died(who, source);

	//updatestats(Ship *killer, Ship *victim)
	Ship *s;
//	if (source->isShip())
//		s = source->ship;
//	else if (source->ship && source->ship->exists() )

	// ... source can be 0 (who=RogSq, src=0) ??? dunno why
	if (!source)
		return;

	statsmanager->updatestats(source, who);

	s = source->ship;
	if ( s && s->isShip() )
	{

		BITMAP *dest = view->frame->surface;
		char sometext[512];
		sprintf(sometext, "Cptn [%s] killed Cptn [%s]\0", s->captain_name, who->captain_name);
		message.out(sometext, 8000, 14);
	}

	return;
	}


void FlMelee::show_ending(int didwewin)
{
	// show some bmp
	
	SpaceSprite *s = playercontrols[0]->ship->data->spriteShip;
	BITMAP *dest = view->frame->surface;


//	view->prepare(view->frame);
//	not really needed here: handles zoom

	// clear the screen
	rectfill(dest, 0, 0, dest->w, dest->h, makecol(50,40,30));

	// show the player ship
	s->draw(Vector2(0,0), Vector2(dest->w, dest->h), 0, view->frame);

	// show some text
	if ( didwewin )
	{
		textprintf( dest, font, 50, 10, pallete_color[15], alliancename[0]);
		textprintf( dest, font, 50, 25, pallete_color[14], "CRUSHED");
		textprintf( dest, font, 50, 40, pallete_color[15], alliancename[1]);
	} else {
		textprintf( dest, font, 50, 10, pallete_color[15], alliancename[1]);
		textprintf( dest, font, 50, 25, pallete_color[14], "CRUSHED");
		textprintf( dest, font, 50, 40, pallete_color[15], alliancename[0]);
	}

	// display stats of the ships
	statsmanager->showstats(view->frame);

	textprintf( dest, font, 50, dest->h -30, pallete_color[15], "PRESS ENTER TO CONTINUE");

	// put all the drawing routines before drawing the mem frame ;)
	scare_mouse();
	view->frame->draw();
	unscare_mouse();

	// wait till the player presses ENTER
	while (! (readkey() >> 8 == KEY_ENTER) ) {};

	game->quit("THE END");
}







StatsManager::StatsManager()
{
	Nships = 0;
	// no stats yet :)

	Nlist = 0;
	// no ships have been added (or killed) yet
}


void StatsManager::addship ( Ship *statship, int ofordisplay )
{
	int H = 40;

	SpaceSprite *spr = statship->data->spriteShip;
	BITMAP *tmp = create_bitmap(40, 40);

	spr->lock();
	BITMAP *src = spr->get_bitmap(0);
	stretch_blit(src, tmp, 0, 0, src->w, src->h, 0, 0, tmp->w, tmp->h);
	spr->unlock();

	// add the bmp to the list now that we're still sure the ship exists:
	list_bmp[Nlist] = tmp;
	list_allyflag[Nlist] = statship->ally_flag;
	++Nlist;
	if ( Nlist >= StatsMax_Nships )
	{
		Nlist = StatsMax_Nships-1;
		//tw_error("Too many ships in the gamelist for the stats manager");
	}

	// only add a stats thingy if the ships needs monitoring (eg. has weapons, belongs to a team)
	if (ofordisplay)
	{
		stats[Nships].Nkilled = 0;
		stats[Nships].allyflag_owner = statship->ally_flag;

		++Nships;
		if ( Nships >= StatsMax_Nships )
		{
			Nships = StatsMax_Nships-1;
			//tw_error("Too many ships in the game for the stats manager");
		}
	}
}

void StatsManager::updatestats(SpaceLocation *killer, Ship *victim)
{

	if ( !(killer && victim) )
		return;

	int i;
	// check the list_allyflag to see if it's an important enough target to record

	int k = 0;
	for ( i = 0; i < Nlist; ++i )
	{
		if ( list_allyflag[i] == (int)killer->ally_flag )
			++k;
		if ( list_allyflag[i] == (int)victim->ally_flag )
			++k;
	}

	if ( k != 2 )	// either killer or killee isn't important.
		return;


	// search the owner:
	for ( i = 0; i < Nships; ++i )
	{
		if ( stats[i].allyflag_owner == killer->ally_flag )
		{
			int k = stats[i].Nkilled;
//			stats[i].killed[k] = victim;
			stats[i].allyflag_killed[k] = victim->ally_flag;
//			stats[i].killeddata[k] = victim->data;

			/*
			SpaceSprite *spr = victim->data->spriteShip;
			BITMAP *tmp = create_bitmap(40, 40);

			spr->lock();
			BITMAP *src = spr->get_bitmap(0);
			stretch_blit(src, tmp, 0, 0, src->w, src->h, 0, 0, tmp->w, tmp->h);
			spr->unlock();

			stats[i].killedbmp[k] = tmp;

			stats[i].killedbmp[k] = tmp;
			*/

			++stats[i].Nkilled;
			if ( stats[i].Nkilled >= StatsMax_Nkilled )
			{
				stats[i].Nkilled = StatsMax_Nkilled - 1;
				//tw_error("Too many ships killed in the game for the stats manager");
			}

		}
	}
}

void StatsManager::showstats(Frame *frame)
{
	// well ... show the graphics of all the victims ?
	int i;

	Vector2 spos, ssize;
	double H1 = 28;
	double H2 = 30;
	ssize = Vector2 ( H1, H1 );

	BITMAP *dest;
	dest = frame->surface;

	for ( i = 0; i < Nships; ++i )
	{
		BITMAP *src;


		int ilist;
		ilist = list_item(stats[i].allyflag_owner);
		src = list_bmp[ilist];
		
		//src = stats[i].ownerbmp;

		spos = Vector2( 50, 60+i*H2 );

		aa_stretch_blit(src, dest, 
			0, 0, src->w, src->h, 
			spos.x, spos.y, ssize.x, ssize.y
			);
		frame->add_box(spos.x, spos.y, ssize.x, ssize.y);

		int k;
		for ( k = 0; k < stats[i].Nkilled; ++k )
		{
			ilist = list_item(stats[i].allyflag_killed[k]);
			src = list_bmp[ilist];
			//src = stats[i].killedbmp[k];

			spos = Vector2( 110+k*H2, 60+i*H2 );

			aa_stretch_blit(src, dest, 
				0, 0, src->w, src->h, 
				spos.x, spos.y, ssize.x, ssize.y
				);
			frame->add_box(spos.x, spos.y, ssize.x, ssize.y);


		}

	}
}


int StatsManager::list_item(int flag)
{
	int i;

	for ( i = 0; i < Nlist; ++i )
		if ( list_allyflag[i] == flag )
			return i;

	
	return 0;		// default value in case nothing was found.
}


FlMelee::~FlMelee()
{
	// this is handled outside normal game scope.
	delete statsmanager;

	unload_datafile(FlmeleeData);
}

StatsManager::~StatsManager()
{
	int i;

	for ( i = 0; i < Nlist; ++i )
	{
		BITMAP *b = list_bmp[i];
		if (b)
			destroy_bitmap(b);
	}
}



BITMAP* copybmp( BITMAP* src )
{
	BITMAP *dest;

	// copied from vanguard
	dest = create_bitmap_ex(bitmap_color_depth(screen), src->w, src->h);	//Create a new bitmap
	if(!dest) return NULL;							//If failed, return NULL

	blit(src, dest, 0, 0, 0, 0, src->w, src->h);	//Copy bitmap from datafile

	return dest;
}



Vector2 YRadar::shiftscale(Vector2 r_center, Vector2 v_center, double scale, Vector2 n)
{
	//Used to scale game coordinates onto RADAR screen coordinates
	return  scale * min_delta(n - r_center, map_size) + v_center;
}

void YRadar::PaintItem(BITMAP *Slate, Vector2 T, SpaceLocation *o, double Scale)
{
	Vector2 pos;

	pos = shiftscale(T, Vector2(Slate->w/2,Slate->h/2), Scale, o->pos);

	if ( display_shape == 2 )
		if ( (pos - Vector2(Slate->w/2,Slate->w/2)).magnitude() > Slate->w/2 - 6)
			return;

	if ( relative_angle )
	{
		Ship *s = controller->ship;
		if (s)
			rotate(pos, -s->angle );
	}
	
	
	BITMAP *bmp = 0;

	if(o->isShip())
	{
		if ( o->get_team() == homeplayer_team )
			bmp = ship_f_icon;
		else
			bmp = ship_e_icon;
	}
	
	if (o->isAsteroid())
		bmp = cbody_icon;
	
	if (o->isPlanet())
		bmp = planet_icon;

	if (o == controller->target)
	{
		if ( game->game_time & 512 )
			bmp = target_icon1;
		else
			bmp = target_icon2;
	}

	if ( bmp )
	{
		pos -= Vector2(bmp->w/2, bmp->h/2);

		masked_blit(bmp, Slate, 
			0, 0, pos.x, pos.y, bmp->w, bmp->h);
	} else {
		putpixel(Slate, pos.x, pos.y, makecol(200,200,200));
	}

}


void YRadar::Paint(BITMAP *Slate, Vector2 T)
{
	double Scale = Slate->w/(2.*size);

	for(int num=0; num<physics->num_items; num++)
	{
		SpaceLocation *o=physics->item[num];

		if ( o != controller->ship )
			PaintItem(Slate, T, o, Scale);
	}
}


YRadar::YRadar(Control *ocontroller, double Size, TeamCode hteam, DATAFILE *dat, bool rel, int shape)
{
	relative_angle = rel;
	display_shape = shape;

	controller = ocontroller;
	size = Size;
	active = TRUE;
	set_depth(DEPTH_STARS + 0.1);

	ship_f_icon = copybmp( (BITMAP*) find_datafile_object(dat, "radar_ship_f")->dat );
	ship_e_icon = copybmp( (BITMAP*) find_datafile_object(dat, "radar_ship_e")->dat );
	cbody_icon  = copybmp( (BITMAP*) find_datafile_object(dat, "radar_cbody")->dat );
	planet_icon = copybmp( (BITMAP*) find_datafile_object(dat, "radar_planet")->dat );
	target_icon1 = copybmp( (BITMAP*) find_datafile_object(dat, "radar_target1")->dat );
	target_icon2 = copybmp( (BITMAP*) find_datafile_object(dat, "radar_target2")->dat );

	backgr_bmp = copybmp( (BITMAP*) find_datafile_object(dat, "radar_backgr")->dat );
	foregr_bmp = copybmp( (BITMAP*) find_datafile_object(dat, "radar_foregr")->dat );

	Painted = create_bitmap_ex(bitmap_color_depth(screen),backgr_bmp->w,backgr_bmp->h);

	if ( !(ship_f_icon && ship_e_icon && cbody_icon && planet_icon &&
		target_icon1 && target_icon2 && backgr_bmp && foregr_bmp ) )
	{
		tw_error("Failed to load one of the radar icons");
	}

	homeplayer_team = hteam;

	location = 0;
}



void YRadar::animate(Frame *space)
{
	//If the radar is disabled, don't do anything.
	if(active==FALSE) return;

	clear_to_color(Painted, 0x0FF00FF);

	//Copy the blank slate onto the temporary bitmap Painted
	masked_blit(backgr_bmp, Painted, 0,0,0,0,backgr_bmp->w, backgr_bmp->h);

	SpaceLocation *l = controller->get_focus();
	if (l)
		Paint(Painted,l->pos);

	// also, blit the radar foreground:
	masked_blit(foregr_bmp, Painted, 0,0,0,0,backgr_bmp->w, backgr_bmp->h);

	int ix, iy;
	ix = location.x;
	iy = location.y;


	//Copy Painted onto space->frame, which will then paint it on the screen.
	masked_blit(Painted,space->surface,0,0,ix,iy,Painted->w,Painted->h);

	//Tell the frame to redraw this space
	space->add_box(ix,iy,Painted->w,Painted->h);

}


void YRadar::toggleActive()
{
	active^=1;
}

void YRadar::setSize(double Size)
{
	size=Size;
}

YRadar::~YRadar()
{
	destroy_bitmap(Painted);
	destroy_bitmap(ship_f_icon);
	destroy_bitmap(ship_e_icon);
	destroy_bitmap(cbody_icon);
	destroy_bitmap(planet_icon);
	destroy_bitmap(target_icon1);
	destroy_bitmap(target_icon2);
	destroy_bitmap(backgr_bmp);
	destroy_bitmap(foregr_bmp);
}






/*
Same as mshppan.cpp, except the panel is NOT drawn onto a new window
(gives me more control), but only the (panel) bitmap is being updated.
*/

ShipPanelBmp::ShipPanelBmp(Ship *_ship) {STACKTRACE
	id |= ID_SHIP_PANEL;

	panel   = create_bitmap_ex(bitmap_color_depth(screen), 64, 100);
	captain = create_bitmap_ex(bitmap_color_depth(screen), CAPTAIN_WIDTH, CAPTAIN_HEIGHT);

	crew_old   = -1;
	batt_old   = -1;
	batt_light = FALSE;

	old_thrust       = FALSE;
	old_turn_left    = FALSE;
	old_turn_right   = FALSE;
	old_fire_weapon  = FALSE;
	old_fire_special = FALSE;

	deathframe = 0;
	ship = _ship;

	ship->spritePanel->draw(0, 0, 0, panel);
	ship->spritePanel->draw(0, 0, 1, captain);
	draw_stuff (crew_x, crew_y, 2, 1, -3, -2, 2, iround_up(ship->crew), iround_up(ship->crew_max), tw_color(ship->crewPanelColor()), 0);
	draw_stuff (batt_x, batt_y, 2, 1, -3, -2, 2, iround_up(ship->batt), iround_up(ship->batt_max), tw_color(ship->battPanelColor()), 0);

	}

ShipPanelBmp::~ShipPanelBmp()
{STACKTRACE
  destroy_bitmap(captain);
  destroy_bitmap(panel);
}


void ShipPanelBmp::animate_panel() {STACKTRACE
	if (!ship) {
		double w, h;

		if (deathframe < 0) return;
		rectfill(captain, 0, 0, CAPTAIN_WIDTH, CAPTAIN_HEIGHT, 0);
		w = CAPTAIN_WIDTH * deathframe / (double)PANEL_DEATH_FRAMES;
		h = CAPTAIN_HEIGHT * deathframe / (double)PANEL_DEATH_FRAMES;
		if(w < 1.0) w = 1.0;
		if(h < 1.0) h = 1.0;
		rectfill(captain,
				(int)(((double)(CAPTAIN_WIDTH) - w) / 2.0),
				(int)(((double)(CAPTAIN_HEIGHT) - h) / 2.0),
				(int)((double)(CAPTAIN_WIDTH) - (((double)(CAPTAIN_WIDTH) - w) / 2.0)),
				(int)((double)(CAPTAIN_HEIGHT) - (((double)(CAPTAIN_HEIGHT) - h) / 2.0)),
				pallete_color[128 - (7 * deathframe / PANEL_DEATH_FRAMES)]);
		blit (captain, panel, 0, 0, CAPTAIN_X, CAPTAIN_Y, captain->w, captain->h);
		return;
		}

	// ship->spritePanel = 0 !! ... this->ship = RogueFigther.
	// can happen, if ships are spawned in-game. You should set spritepanel yourself
	// in that ship, then?
	if (!ship->spritePanel)
		return;

//	if (panel_needs_update) {
		ship->spritePanel->draw(0, 0, 0, panel);
		crew_old = -1;
		batt_old = -1;
//		}

//	if (captain_needs_update) {
//		captain_needs_update = false;
		old_thrust       = ship->thrust;
		old_turn_left    = ship->turn_left;
		old_turn_right   = ship->turn_right;
		old_fire_weapon  = ship->fire_weapon;
		old_fire_special = ship->fire_special;

		ship->spritePanel->draw(0, 0, 1, captain);
		if(ship->thrust)       ship->spritePanel->overlay(1, 2, captain);
		if(ship->turn_right)   ship->spritePanel->overlay(1, 3, captain);
		if(ship->turn_left)    ship->spritePanel->overlay(1, 4, captain);
		if(ship->fire_weapon)  ship->spritePanel->overlay(1, 5, captain);
		if(ship->fire_special) ship->spritePanel->overlay(1, 6, captain);
		blit (captain, panel, 0, 0, CAPTAIN_X, CAPTAIN_Y, captain->w, captain->h);
//		panel_needs_update = true;
//		}

		// BUG: pkunk can have inf. crew ?!
	if (iround_up(ship->crew) != crew_old) {		
		crew_old = iround_up(ship->crew);
		draw_stuff (crew_x, crew_y, 2, 1, -3, -2, 2, crew_old, iround_up(ship->crew_max), tw_color(ship->crewPanelColor()), 0);
//		panel_needs_update = true;
		}

	if (iround_up(ship->batt) != batt_old) {
		batt_old = iround_up(ship->batt);
		draw_stuff (batt_x, batt_y, 2, 1, -3, -2, 2, batt_old, iround_up(ship->batt_max), tw_color(ship->battPanelColor()), 0);
//		panel_needs_update = true;
		}

	/* cannot access those protected variables of the ship
	if((!batt_light) && (ship->weapon_low || ship->special_low)) {
		game->panelSprite->draw(40, 58, 2, panel);
		batt_light = TRUE;
		panel_needs_update = true;
		}
	else if(batt_light && (!ship->weapon_low) && (!ship->special_low)) {
		game->panelSprite->draw(40, 58, 1, panel);
		batt_light = FALSE;
		panel_needs_update = true;
		}
	*/

	return;
	}

void ShipPanelBmp::draw_stuff (int x, int y, int w, int h, int dx, int dy, int m, int value, int max, int color, int bcolor) {STACKTRACE
	int i;
	w -= 1;
	h -= 1;
	if (value > max) value = max;
	for (i = 0; i < value; i += 1) {
		int _x = x + dx * (i % m);
		int _y = y + dy * (i / m);
		rectfill(panel, _x, _y, _x+w, _y+h, color);
		}
	for (i = value; i < max; i += 1) {
		int _x = x + dx * (i % m);
		int _y = y + dy * (i / m);
		rectfill(panel, _x, _y, _x+w, _y+h, bcolor);
		}
	return;
	}



ImIndicator::ImIndicator(SpaceLocation *o)
{
	showme = o;

	bmp = create_bitmap_ex( bitmap_color_depth(screen), ImIndicatorSize, ImIndicatorSize);
//void SpaceSprite::draw_character(int x, int y, int index, int color, BITMAP *bmp) 

	clear_to_color(bmp, makecol(255,0,255));
	// also, turn this into some "character" of the sprite ?!

	if (o->isObject())
	{
		SpaceSprite *src;
		src = ((SpaceObject*)o)->get_sprite();
		src->draw_character(0, 0, bmp->w, bmp->h, 0, colortype(o), bmp);
	} else {
		clear_to_color(bmp, 0x0408090);
	}
}


ImIndicator::~ImIndicator()
{
	destroy_bitmap(bmp);
}

int ImIndicator::colortype(SpaceLocation *o)
{
	if (o->isPlanet())
		return makecol(150,100,25);

	else if (o->isShip())
		return makecol(100,150,25);

	else
		return makecol(150,50,50);
}

void ImIndicator::newtarget(SpaceLocation *o)
{
	showme = o;

	if (o->isObject())
	{
		SpaceSprite *src;
		src = ((SpaceObject*)o)->get_sprite();
		src->draw_character(0, 0, bmp->w, bmp->h, 0, colortype(o), bmp);
	} else {
		clear_to_color(bmp, 0x0408090);
	}
}


void ImIndicator::animate(Frame *frame)
{

	// do not draw always -- uhm, is either irritating if too fast, and useless if too slow
	//if ( (game->game_time/1024) & 1 )
	//	return;


	// where does the thing intersect the screen ?
	// the screen has 4 boundaries ...

	Vector2 C, P, S;

	C = space_view_size / 2;		// center of the map
	P = corner ( showme->pos );

	Vector2 ssize = ((SpaceObject*)showme)->get_sprite()->size() * space_zoom / 2;

	if (   P.y + ssize.y > 0
		&& P.y - ssize.y < space_view_size.y
		&& P.x + ssize.x > 0
		&& P.x - ssize.x < space_view_size.x )
		return;

	S = P;

	if ( S.y < 0 )
	{
		S.y = 0;
		S.x = C.x + (S.y - C.y) * (P.x - C.x) / (P.y - C.y);
	}

	if ( S.y > space_view_size.y )
	{
		S.y = space_view_size.y;
		S.x = C.x + (S.y - C.y) * (P.x - C.x) / (P.y - C.y);
	}

	if ( S.x < 0 )
	{
		S.x = 0;
		S.y = C.y + (S.x - C.x) * (P.y - C.y) / (P.x - C.x);
	}

	if ( S.x > space_view_size.x )
	{
		S.x = space_view_size.x;
		S.y = C.y + (S.x - C.x) * (P.y - C.y) / (P.x - C.x);
	}

	// now, S is the position of the indicator. We need to know the distance, for
	// scaling purposes

	double	scale;
	scale = sqrt( frame->surface->w / min_delta(showme->pos - space_center, map_size).magnitude() );
	if ( scale > 1.0 )	scale = 1.0;
	if ( scale < 0.2 )	scale = 0.2;

	// now, plot the indicator ?!


	int iw, ih;
	iw = bmp->w*scale;
	ih = bmp->h*scale;

	S -= Vector2(iw/2, ih/2);

	masked_stretch_blit(bmp, frame->surface, 0, 0, bmp->w, bmp->h,
						S.x, S.y, iw, ih);
	frame->add_box(S.x, S.y, iw, ih);
}


REGISTER_GAME (FlMelee, "Melee with Fleets");

