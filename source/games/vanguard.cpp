#include <allegro.h>		//allegro library header
#include <string.h>		//Used for my DAT file searchers

#include "../melee.h"          //used by all TW source files.  well, mostly.
REGISTER_FILE                  //done immediately after #including melee.h, just because I said so
#include "../melee/mgame.h"    //Game stuff
#include "../melee/mcbodies.h" //asteroids & planets
#include "../melee/mship.h"    //ships
#include "../melee/mlog.h"     //networking / demo recording / demo playback
#include "../melee/mcontrol.h" //controllers & AIs
#include "../melee/mview.h"    //Views & messages
#include "../melee/mshppan.h"  //ship panels...
#include "../melee/mfleet.h"  //fleets...
#include "../other/orbit.h"
#include "../other/radar.h"

/*******************************Defines**************************************/

#define NUM_ENTRIES 23
#define Num_Planet_Pics 8
#define Num_Giant_Pics 2
#define Num_Moon_Pics 3
#define Num_Star_Pics 6
#define MAX_TEAMS 5

#define ZOOM_DEGREE 1000.
#define Num_Van_Keys 6

#define KEY_ZOOM_IN 0
#define KEY_ZOOM_OUT 1
#define KEY_ZOOM_ALL 2
#define KEY_SWITCH_RADAR 3
#define KEY_HYPERSPACE 4
#define KEY_HIDE_RADAR 5

/***************************Global Variables*********************************/



//The names of the key-types read from vanguard.ini
char *Van_Key_Configs[Num_Van_Keys+1] ={	"KeyZoomIn","KeyZoomOut","KeyZoomAll","KeySwitchRadar",
					"KeyHyperSpace","KeyHideRadar", NULL			};


int Van_Keys[Num_Van_Keys];	//Where the actual key-numbers are stored
DATAFILE *VanDat;

/************************HyperJammer code************************************/
class HyperJammer:public Presence
{
public:
	Vector2 oldpos;
	Ship *Target;
	int Rate, Cur;
	HyperJammer(Ship *target, int rate);
	virtual void calculate();
	bool is_in_hyperspace();
	void set_target(Ship *t);
	virtual void animate(Frame *space);
};

void HyperJammer::animate(Frame *space)
{
	if(Target!=NULL)
	{

//		Target->getSprite()->animate_character(oldx, oldy,i,makecol(255,0,0),space);
	}


//			target->normal_x() + (i + 1) *
//			cos((trajectory_angle(target) - PI)) * length,
//			target->normal_y() + (i + 1) *
//			sin((trajectory_angle(target) - PI)) * length,
//			target->getSpriteIndex(), pallete_color[beam_color[i]], space);


}


void HyperJammer::set_target(Ship *t)
{
	Target=t;
/*	if(Target!=NULL)
	{
		int i=Target->angle/(PI2/64);
		i+=16;
		i&=63;

		game->addItem(new Phaser(Target, Target->x, Target->y, oldx, oldy, Target,
			Target->getSprite(), i, hot_color, HOT_COLORS, PHASE_DELAY, PHASE_MAX,
			PHASE_DELAY));
	}*/
}

bool HyperJammer::is_in_hyperspace()
{
	return(Target!=NULL);
}

HyperJammer::HyperJammer(Ship *target, int rate)
{
	Target=target;
	Cur=0;
	Rate=rate;
}

void HyperJammer::calculate()
{
	if(Target==NULL) return;
	if(!Target->exists())
		Target=NULL;

	Cur+=frame_time;

//	oldx=Target->x;
//	oldy=Target->y;
	oldpos=Target->pos;

//	Target->x+=(Target->vx*frame_time*10.);
//	Target->y+=(Target->vy*frame_time*10.);

//	double v=pow(Target->vx*Target->vx+Target->vy*Target->vy,.5)*100.+1.;  //Get the actual velocity
	double v=magnitude(Target->vel)*100.+1.;  //Get the actual velocity

	double fv = log(v)*.32;

//	double fvx=fv*cos(Target->angle);
//	double fvy=fv*sin(Target->angle);
	Vector2 fvel = fv * unit_vector(Target->angle);

//	Target->x+=(fvx*frame_time);
//	Target->y+=(fvy*frame_time);
	Target->pos += fvel*frame_time;


//	Target->vx *= 1 - frame_time * 0.001;
//	Target->vy *= 1 - frame_time * 0.001;
	Target->vel *= 1 - frame_time * 0.001;

	if(Target->batt>0)
	{
		if(Cur>Rate)
		{
			Cur=0;
			Target->batt--;
		}
	}
	else Target=NULL;


}

/*********************************RADAR Code*********************************/

class VanRadar:public ZRadar
{
public:
	TeamCode	TCodes[MAX_TEAMS];
	int		TColor[MAX_TEAMS];
	int		Coded_Teams;
	double		InternalZoom;
	VanRadar(BITMAP *BlankSlate, SpaceLocation *target, double size);
	virtual void Paint(BITMAP *slate, double Tx, double Ty);
	void addTeam(TeamCode team, int color);
};

void VanRadar::addTeam(TeamCode team, int color)
{
	if(Coded_Teams<(MAX_TEAMS-1))
	{
		TCodes[Coded_Teams]=team;
		TColor[Coded_Teams]=color;
		Coded_Teams++;
	}
}

VanRadar::VanRadar(BITMAP *BlankSlate, SpaceLocation *target, double size):ZRadar(BlankSlate, target, size)
{
	Coded_Teams=0;
	InternalZoom = size;
}

//I have overriden ZRadar::Paint() with a more complicated radar painter.
void VanRadar::Paint(BITMAP *Slate, double Tx, double Ty)
{
	for(int num=0; num<physics->num_items; num++)
	{
		int xpos,ypos;
		SpaceLocation *o=physics->item[num];
		OrbitHandler *h;
		Ship *sp;

		double Scale = Slate->w/(2.*InternalZoom);

		InternalZoom=(size+(InternalZoom*ZOOM_DEGREE))/(ZOOM_DEGREE+1.);

		//Ignore everything outside the window except Orbits

		if((abs((int)(Tx - o->pos.x))>InternalZoom)&&(o->id!=ORBIT_ID))	continue;
		if((!o->isObject())&&(o->id!=ORBIT_ID))			continue;

		//Scale it's position

		//Position---------------------------------\
		//Zooming ---------------------------\	   |
		//Center of RADAR coordinates--\     |     |
		//Center of game coords--\/    \/    \/    \/
		xpos=(int)shiftscale(Tx, Slate->w/2.,Scale,o->pos.x);
		ypos=(int)shiftscale(Ty, Slate->w/2.,Scale,o->pos.y);

		switch(o->id)
		{
		case SUN_ID:
			circlefill(Slate,xpos,ypos,5,makecol(255,255,0));
			break;
		case MOON_ID:
			circlefill(Slate,xpos,ypos,2,makecol(150,150,150));
			break;
		case ORBIT_ID:
			h=(OrbitHandler *)o;
			circle(Slate,xpos,ypos,(int)(h->Radius*Scale),makecol(0,0,128));
			break;
		case COMET_ID:
			circle(Slate,xpos,ypos,1,makecol(0,255,255));
			break;
		default:
			if(o->isPlanet())		circlefill(Slate,xpos,ypos,3,makecol(200,200,200));
			else if(o->isAsteroid())	circlefill(Slate,xpos,ypos,1,makecol(174,131,66));
			else if(o->isShip())
			{
				sp=(Ship *)o;
				if(!sp->isInvisible())
				{
					int TeamFound=0;
					for(int num=0; num<Coded_Teams; num++)
						if(TCodes[num]==o->get_team())
						{
							circle(Slate,xpos,ypos,2,TColor[num]);
							TeamFound=1;
						}
					if(TeamFound == 0) circle(Slate,xpos,ypos,2,makecol(255,64,64));
				}
			}
			else if(o->isShot())	putpixel(Slate,xpos,ypos,makecol(255,255,0));
			break;
		}
	}
}

/********************************Game code!*****************************************************/

class Vanguard : public Game { //declaring our game type
public:
	BITMAP *blankRadar;						//Image of the blank RADAR screen
	VanRadar *vradar;							//Pointer to the VanRadar object
	double RadarZoom;						//Size of the area seen by VanRadar

	HyperJammer *jammer;

	SpaceSprite *PlanetPics[Num_Planet_Pics];			//Sprites for Solar System Melee
	SpaceSprite *GiantPics[Num_Giant_Pics];				//...
	SpaceSprite *MoonPics[Num_Moon_Pics];				//...
	SpaceSprite *StarPics[Num_Star_Pics];				//...

	BITMAP *background;						//Titlescreen picture

	ShipPanel *human_panel[2];  					//we need to keep track of the ship panels, in case
									//we have to move them around because the player
									//changes screen resolutions
	Control *human_control[2];
	TeamCode human_team, enemy_team;				 //the two teams
	Ship *HumanPlayer1, *HumanPlayer2;


	bool	GetSprites(SpaceSprite *Pics[], DATAFILE *datArray, char *cmdStr, int numSprites);	//Read a list of sprites
	SpaceSprite *GetSprite(DATAFILE *datArray, char *spriteName);				//Read a single sprite
	BITMAP *GetBitmap(DATAFILE *datArray, char *bitmapName);					//Read a single bitmap

	DATAFILE *FindDat(DATAFILE *datarray, char *name);

	virtual void init(Log *_log);					//happens when our game is first started

	virtual void set_resolution(int screen_x, int screen_y);	//we want to change our screen layout,
									//to make room for the ship panels

	virtual void calculate();				//To do the screen layouts, we need to know if those
									//panels still exist, so we need to check every
									//frame for them going away calculate is good for
									//things that need to be done every frame

	int respawn_time;						//the time left until the game is restarted

	virtual bool handle_key(int k);					//Gee, I dunno boss...

	void pick_new_ships();						//restarts the game
	int search_key(int key);					//Searches through keylist

	~Vanguard();							//Destructor:  Deletes all sprites and bitmaps
};

int Vanguard::search_key(int key)
{
	for(int num=0; num<Num_Van_Keys; num++)
			if(key==Van_Keys[num]) return num;
	return -1;
}

Vanguard::~Vanguard()
{
	int num;
	for(num=0; num<Num_Planet_Pics; num++)	delete PlanetPics[num];
	for(num=0; num<Num_Moon_Pics; num++)	delete MoonPics[num];
	for(num=0; num<Num_Giant_Pics; num++)	delete GiantPics[num];
	for(num=0; num<Num_Star_Pics; num++)	delete StarPics[num];
	delete vradar;
	destroy_bitmap(blankRadar);

}

DATAFILE *Vanguard::FindDat(DATAFILE *datarray, char *name)
{
	for(int num=0; num<NUM_ENTRIES; num++)
		if(strcmp(name,datarray[num].prop[1].dat)==0) return &datarray[num];

	return NULL;
}

bool Vanguard::handle_key(int k)
{
	char buffy[256];
	message.print(1000,12,"Key:  %s",key_to_name(k>>8, buffy));
	switch(search_key(k>>8))
	{
	case KEY_HYPERSPACE:
		if(jammer->is_in_hyperspace())
		{
			jammer->set_target(NULL);
			message.out("Exiting hyperspace.");
		}
		else if(HumanPlayer1->batt==HumanPlayer1->batt_max)
		{
			jammer->set_target(HumanPlayer1);
			message.out("Entering hyperspace.");
		}

		break;

	case KEY_HIDE_RADAR:			//Toggle on/off radar

		vradar->toggleActive();
		break;

	case KEY_SWITCH_RADAR:			//Switch between "everything" view and "hero" view
		if(vradar->t==NULL)
		{
			vradar->t=HumanPlayer1;
			break;
		}
		else	vradar->t=NULL;
		//Notice that there is no break here.  I want it to fall thru to the KEY_SLASH_PAD code.

	case KEY_ZOOM_ALL:		//Zoom out full
//		RadarZoom = width/2.;
		RadarZoom = this->size.x / 2;
		vradar->setSize(RadarZoom);
		return TRUE;
		break;


	case KEY_ZOOM_IN:
		if(RadarZoom>2000.) RadarZoom*=.8;
		vradar->setSize(RadarZoom);
		return TRUE;
		break;


	case KEY_ZOOM_OUT:
		if(RadarZoom < this->size.x /*width*/) RadarZoom*=(1./.8);
		vradar->setSize(RadarZoom);
		return TRUE;
		break;

	default:			//Everything else
		return Game::handle_key(k);
		break;
	}
	return false;
}

BITMAP *Vanguard::GetBitmap(DATAFILE *datArray, char *bitmapName)
{
	DATAFILE *tmpdata;		//The object that the data gets tossed into
	BITMAP *src,*bmp;		//Two bitmaps:  One direct from tmpdata, and the copy of it that will be returned.

	tmpdata = FindDat(datArray,bitmapName);	//Load it

	if(tmpdata==NULL)	return NULL;			//If it failed to load, return NULL

	switch(tmpdata->type)	//Check if that was really a bitmap we just loaded
	{
		case DAT_BITMAP:			//If it is a bitmap, then:
			src=(BITMAP *)tmpdata->dat;					//Have src point to it
			bmp = create_bitmap_ex(bitmap_color_depth(screen),src->w,src->h);	//Create a new bitmap
			if(!bmp) return NULL;							//If failed, return NULL

			blit(src, bmp, 0, 0, 0, 0, src->w, src->h);			//Copy bitmap from datafile
											//into src.  This copying
											//also ensures an appropriate
											//color depth.
			break;

		default:	//If it is NOT a bitmap, we'll be returning NULL.

			bmp=NULL;
			break;
	}
	return bmp;
}

SpaceSprite *Vanguard::GetSprite(DATAFILE *datArray, char *spriteName)
{
	DATAFILE *tmpdata;		//Temporary holder for data

	tmpdata = FindDat(datArray,spriteName);	//Load sprite into holder

	if(tmpdata==NULL)	//If it failed to find it:
	{
		return NULL;
	}

	return (new SpaceSprite(tmpdata, 1, SpaceSprite::MASKED, 1));	//Make a copy of it
}

bool Vanguard::GetSprites(SpaceSprite *Pics[], DATAFILE *datArray, char *cmdStr, int numSprites)
{

	//Example to load 10 sprites into an empty array, do:
	//	GetSprites(array, "whatever.dat", "SpriteName%03d",9);
	//	The %03d means that it goes SpriteName000, SpriteNAme001, etc.
	//	%02d would be ...00, ...01, ...02 etc.

	SpaceSprite *spr;
	char dataStr[100];

	for(int num=0; num<numSprites; num++)
	{
		sprintf(dataStr,cmdStr,num);
		spr=GetSprite(datArray, dataStr);

		if(!spr)	return FALSE;

		Pics[num]=spr;
	}

	return TRUE;
}

void Vanguard::calculate() {
	Game::calculate();
	if (human_panel[0] && !human_panel[0]->exists()) human_panel[0] = NULL;
	if (human_panel[1] && !human_panel[1]->exists()) human_panel[1] = NULL;
	if (respawn_time == -1) {
		int i, humans = 0, enemies = 0;
		for (i = 0; i < num_targets; i += 1) {
			if (target[i]->get_team() == human_team) humans += 1;
			if (target[i]->get_team() == enemy_team) enemies += 1;
			}
		//if either team has no targetable items remaining (generally ships), pick new ships
		if (!humans || !enemies) respawn_time = game_time + 5000; //5000 milliseconds is 10 seconds
		}
	else if (respawn_time <= game_time) pick_new_ships();

//	vradar->setTarget(HumanPlayer1);

	return;
	}

void Vanguard::set_resolution(int screen_x, int screen_y) {
/*	view->set_window(screen, //the view window is on the screen
		0, 0, //with the upper left hand corner of the view window on the upper left corner of the screen
		screen_x - 64, screen_y //the height is full-screen, but the width doesn't cover the right-hand side, to make room for the ship panel there
		);
	if (human_panel[0]) human_panel[0]->locate(view->view_x + view->view_w, 0);   //if the panel still exists, position on the right-hand side
	if (human_panel[1]) human_panel[1]->locate(view->view_x + view->view_w, 100); //same for the other panel, if there is another panel*/
	redraw();
	}

void Vanguard::pick_new_ships() {
	int i;
	acquire_screen();
	clear_to_color(screen, pallete_color[4]);
	release_screen();
	for (i = 0; i < num_items; i += 1)	if (item[i]->exists()) item[i]->die();

//	if (human_panel[0]) human_panel[0]->hide();
//	if (human_panel[1]) human_panel[1]->hide();



	//Probably a good idea to get all your variables immediately after
	//opening your .ini file.
	log_file("vanguard.ini");
	int GasGrav = get_config_int(NULL, "Gasgravity",0);
	double MoonGrav = get_config_float(NULL, "Moongravity", 0);
	int NSuns = get_config_int(NULL, "NSuns", 0);
	int MinPlanets = get_config_int(NULL, "MinPlanets", 0);
	int MaxPlanets = get_config_int(NULL, "MaxPlanets", 0);
	int NumMoons = get_config_int(NULL, "Moons", 0);
	int Radius = get_config_int(NULL, "Radius", 0);
	int MRadius = get_config_int(NULL, "MRadius", 0);
	int Comets = get_config_int(NULL, "Comets",0);
	int CoMass = get_config_int(NULL, "Comet_mass",0);
	int ComMax = get_config_int(NULL, "Comet_max",0);
	int Asteroids=get_config_int(NULL,"Asteroids",0);
	int ComAcc = get_config_int(NULL, "Comet_acc",0);
	int SizeX = get_config_int(NULL, "SizeX",0);
	int SizeY = get_config_int(NULL, "SizeY",0);
	double MinDistance = get_config_float(NULL,"MinDistance",0);
	double PlanetVel = get_config_float(NULL,"PlanetVel",0) * ANGLE_RATIO;
	double MoonVel = get_config_float(NULL,"MoonVel",0) * ANGLE_RATIO;

//	width = Size1*Size2;			//Set the size BEFORE putting in the ships
//	height = Size1*Size2;
	size = Vector2(SizeX, SizeY);


	//player 1 selects a new ship
	Fleet fleet;

#define LOAD_FLEET log_file("fleets/all.scf"); fleet.load(NULL, "Fleet"); 

	LOAD_FLEET
	i = human_control[0]->choose_ship(window, "Hey You!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
//	HumanPlayer1 = create_ship(fleet.ship[i]->id, human_control[0], tw_random(Vector2(width, height)));rand()%(int)width, rand()%(int)height, 0, human_team);
	HumanPlayer1 = create_ship(fleet.ship[i]->id, human_control[0], tw_random(size), 0, human_team);
	add(HumanPlayer1->get_ship_phaser());
	human_panel[0] = new ShipPanel(HumanPlayer1);
	human_panel[0]->window->init(window);
	human_panel[0]->window->locate(0,0.9, 0,0, 0,0.1, 0,0.25);

	add(human_panel[0]);
	add_focus(HumanPlayer1, channel_server);

	if ((log->type == Log::log_net1server) || (log->type == Log::log_net1client)) {
		LOAD_FLEET
		i = human_control[1]->choose_ship(window, "Hey You Player!\nPick a ship!", &fleet);
		log_int(channel_client, i);
		if (i == -1) i = random() % fleet.size;
//		Ship *s = create_ship(fleet.ship[i]->id, human_control[1], width/2 + 100, height/2, PI, human_team);
		Ship *s = create_ship(fleet.ship[i]->id, human_control[1], size/2 + Vector2(100,0), PI, human_team);
		add(s->get_ship_phaser());
		add_focus(s, channel_client);
		human_panel[1] = new ShipPanel(s);
		human_panel[1]->window->init(window);
		human_panel[1]->window->locate(0,0.9, 0,0.25, 0,0.1, 0,0.25);
		add(human_panel[1]);
		}
	else human_panel[1] = NULL;


	Ship *e;
	LOAD_FLEET
	i = human_control[0]->choose_ship(window, "Hey You AI1!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
//	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", width/4, height/4, random(PI2), enemy_team);
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", size/4, random(PI2), enemy_team);
	add(e->get_ship_phaser());

	LOAD_FLEET
	i = human_control[0]->choose_ship(window, "Hey You AI2!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
//	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", width*3/4, height/4, random(PI2), enemy_team);
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", size*Vector2(3.0,1.0)/4.0, random(PI2), enemy_team);
	add(e->get_ship_phaser());

	LOAD_FLEET
	i = human_control[0]->choose_ship(window, "Hey You AI3!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
//	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", width*3/4, height*3/4, random(PI2), enemy_team);
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", size*3.0/4.0, random(PI2), enemy_team);
	add(e->get_ship_phaser());

	LOAD_FLEET
	i = human_control[0]->choose_ship(window, "Hey You AI4!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
//	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", width/4, height*3/4, random(PI2), enemy_team);
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", size*Vector2(1.0,3.0)/4.0, random(PI2), enemy_team);
	add(e->get_ship_phaser());

	LOAD_FLEET
	i = human_control[0]->choose_ship(window, "Hey You AI5!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
//	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", rand()%(int)width, rand()%(int)height, random(PI2), enemy_team);
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", tw_random(size), random(PI2), enemy_team);
	add(e->get_ship_phaser());

	LOAD_FLEET
	i = human_control[0]->choose_ship(window, "Hey You AI6!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
//	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", random()%(int)width, random()%(int)height, random(PI2), enemy_team);
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", tw_random(size), random(PI2), enemy_team);
	add(e->get_ship_phaser());

	LOAD_FLEET
	i = human_control[0]->choose_ship(window, "Hey You AI7!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
//	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", random()%(int)width, random()%(int)height, random(PI2), enemy_team);
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", tw_random(size), random(PI2), enemy_team);
	add(e->get_ship_phaser());

//Solar system object code
	Sun *Centre[1000];
	Planet *Satellite, *Moon;
	OrbitHandler *handler;

//	vradar->setSize(RadarZoom=width/2.);
	vradar->setSize(RadarZoom=size.x/2.);



	int kind, moons, NumPlanets, iSuns;
	int w, h, w2, h2, ok;

	//for (int j=0; j<NSuns; j+=1)
	iSuns = 0;
	while ( iSuns < NSuns )
	{
		ok = 0;
		int n=0;
		int Dir = ((rand()%2)*2)-1;

		while (ok!=1)
		{
//			w = (int)( random((int)(width-(MinDistance*2)))+MinDistance);
//			h = (int)( random((int)(height-(MinDistance*2)))+MinDistance);
			w = (int)( random((int)(size.x-(MinDistance*2)))+MinDistance);
			h = (int)( random((int)(size.y-(MinDistance*2)))+MinDistance);
			ok=1;

			for (i=0; i<iSuns; i+=1)
			{
				w2 = (int)Centre[i]->pos.x;
				h2 = (int)Centre[i]->pos.y;

				if(pow(pow(w-w2,2)+pow(h-h2,2),.5)<MinDistance) ok=0;
			}

			n++;
			if(n>50000)
			{
				message.print(1000,12,"50,000 iterations.  Only %d stars added.",iSuns);
				goto done;
			}
		}


		add(Centre[iSuns] = new Sun(Vector2(w, h), StarPics[rand()%Num_Star_Pics],0));

		Centre[iSuns]->id=SUN_ID;

		// planets creating loop
		NumPlanets=MinPlanets+rand()%(MaxPlanets-MinPlanets);

		for(int num=0; num<NumPlanets; num++)
		{
			kind = rand()%3;
			moons = rand()%(NumMoons+1);
			if ((kind == 0) && (((num+1)*Radius)> 1600))
			{	// gas giant
//				Satellite = new Planet(width/2,height/2,GiantPics[rand()%Num_Giant_Pics],0);
				Satellite = new Planet(size/2, GiantPics[rand()%Num_Giant_Pics],0);
				Satellite->gravity_force *= GasGrav;

//				handler = new OrbitHandler(Centre[j],width/2,height/2,random(PI2), (SpaceLocation *)Centre[j],
//					(SpaceLocation *)Satellite, (num+1)*Radius, PlanetVel*Dir,0);
				handler = new OrbitHandler(Centre[iSuns],size/2,random(PI2), (SpaceLocation *)Centre[iSuns],
					(SpaceLocation *)Satellite, (num+1)*Radius, PlanetVel*Dir,0);
			}
			else
			{	// normal planet
//				Satellite = new Planet(width/2,height/2,PlanetPics[rand()%Num_Planet_Pics],0);
				Satellite = new Planet(size/2,PlanetPics[rand()%Num_Planet_Pics],0);

//				handler = new OrbitHandler(Centre[j],width/2,height/2,random(PI2), (SpaceLocation *)Centre[j],
//					(SpaceLocation *)Satellite, (num+1)*Radius, PlanetVel*Dir,0);
				handler = new OrbitHandler(Centre[iSuns],size/2,random(PI2), (SpaceLocation *)Centre[iSuns],
					(SpaceLocation *)Satellite, (num+1)*Radius, PlanetVel*Dir,0);
			}

			add(Satellite);
			add(handler);
									// moons code
			for (int i=0; i<moons; i+=1)
			{
//				Moon = new Planet(width/2,height/2,MoonPics[rand()%Num_Moon_Pics],0);
				Moon = new Planet(size/2,MoonPics[rand()%Num_Moon_Pics],0);
				Moon->gravity_force *= MoonGrav;
				Moon->gravity_range = 8;
				Moon->id=MOON_ID;

//				handler = new OrbitHandler(Satellite,width/2,height/2,random(PI2), (SpaceLocation *)Satellite,
//					(SpaceLocation *)Moon, (i*40)+MRadius, MoonVel*Dir,0);
				handler = new OrbitHandler(Satellite,size/2,random(PI2), (SpaceLocation *)Satellite,
					(SpaceLocation *)Moon, (i*40)+MRadius, MoonVel*Dir,0);

				add(Moon);
				add(handler);

			}
		}

		++iSuns;

	}
done:
	//comet code
	for (int num = 0; num < Comets; num++)
	{
		Planet *c;
//		c = new Planet (random()%int(width), random()%int(height), kaboomSprite, 1);
		c = new Planet (tw_random(size), kaboomSprite, 1);
		c->id=COMET_ID;
		c->mass = CoMass;
		c->gravity_force *= 0;
		c->gravity_whip = 0;
		c->accelerate(NULL, random(PI2), get_config_int(NULL, "Comet_acc", 2), get_config_int(NULL, "Comet_max", 2));
		add (c);
	}

//End solar system object code


//	if (human_panel[0]) human_panel[0]->locate(view->view_x + view->view_w, 0);   //if the panel still exists, position on the right-hand side
//	if (human_panel[1]) human_panel[1]->locate(view->view_x + view->view_w, 100); //same for the other panel, if there is another panel

	redraw();
	respawn_time = -1;
	return;
	}


void Vanguard::init(Log *_log) {
	Game::init(_log);

	prepare();

	add ( new Stars() );

	VanDat = load_datafile("vanguard.dat");

	background=GetBitmap(VanDat,"Title");
	if(background!=NULL) {
		window->lock();
		stretch_blit(background, window->surface, 0, 0, background->w, background->h, 0, 0, window->w, window->h);
		window->unlock();
	}

	destroy_bitmap(background);

	//width & height are the dimensions of the game map
	//they should be modified before prepare() is called
	//If for some reason you modify them later, call prepare() again
	//but you probably shouldn't be modifying them later.
	//The normal size is 3840x3840

	blankRadar=GetBitmap(VanDat,"Scope");		//Load blank RADAR image
	if(!blankRadar) error("Can't load scope image.");

	jammer = new HyperJammer(NULL,150);
	game->add(jammer);

//Solar System Melee code

	log_file("Vanguard.ini");

	FILE *fp=fopen("vankey.log","wt");

	for(int num=0; num<Num_Van_Keys; num++)
	{
		char *str=(char *)get_config_string(NULL,Van_Key_Configs[num],0);
		Van_Keys[num] = name_to_key(str);

		fprintf(fp,"%2d: %s/%d\n",num,str,Van_Keys[num]);
	}

	fclose(fp);

	if(GetSprites(PlanetPics,VanDat,"Station_Planet%03d",Num_Planet_Pics)==FALSE)
		error("File error, planet pics.  Bailing out...");

	if(GetSprites(GiantPics,VanDat,"Station_Giant%03d",Num_Giant_Pics)==FALSE)
		error("File error, giant pics.  Bailing out...");

	if(GetSprites(MoonPics,VanDat,"Station_Moon%03d",Num_Moon_Pics)==FALSE)
		error("File error, moon pics.  Bailing out...");

	if(GetSprites(StarPics,VanDat,"Star%03d",Num_Star_Pics)==FALSE)
		error("File error, star pics.  Bailing out...");

//End Solar System Melee code

	human_team = new_team();
	enemy_team = new_team();

//	vradar = new VanRadar(blankRadar,NULL,RadarZoom = width);
	vradar = new VanRadar(blankRadar,NULL,RadarZoom = size.x);

	vradar->addTeam(human_team,makecol(65,255,128));	//A pleasant green
	vradar->addTeam(enemy_team,makecol(255,65,0));		//A violent red
	add(vradar);

	human_control[0] = create_control(channel_server, "Human");

	if ((log->type == Log::log_net1server) || (log->type == Log::log_net1client))
		human_control[1] = create_control(channel_client, "Human");

	else human_control[1] = NULL;

	log_file("vanguard.ini");
//	int Size1 = get_config_int(NULL, "Size1", 0);
//	int Size2 = get_config_int(NULL, "Size2", 0);
	int Asteroids = get_config_int(NULL,"Asteroids",0);

	int i;
	for(i=0; i<Asteroids; i++) add(new Asteroid());

	human_panel[0] = human_panel[1] = NULL;

	change_view("Hero");		//sets it to Hero view mode

	view->window->locate(0,0, 0,0, 0,0.9, 0,1);

	view_locked = true;			//prevents the view mode from being changed through the menu

	respawn_time = 500;

	message.out("<Vanguard>", 3000, 15);

	unload_datafile(VanDat);

	prepare();

}

REGISTER_GAME(Vanguard, "TW: Vanguard")
