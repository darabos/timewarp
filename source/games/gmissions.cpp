
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

#include "../melee/manim.h"
#include "../frame.h"

#include "gamehierarchy.h"

#include "../twgui/twgui.h"

#include "gflmelee.h"
#include "gmissions_objects.h"

#include <string.h>
#include <stdio.h>


//static const titlesize = 64;
//static const briefingsize = 2048;
static const int titlesize = 64;      //added int 7/1/2003 Culture20
static const int briefingsize = 2048; //added int 7/1/2003 Culture20


enum UpgradeCode
{
	upgr_crew=1,
	upgr_crewmax
};

class MSupgrade : public MissionObject
{
	UpgradeCode	upgradecode;
	double		waittimer, respawntime;
public:
	MSupgrade(Vector2 opos, char *datafilename, UpgradeCode oupgradecode, double orespawntime);

	virtual void calculate();
	virtual void animate(Frame *space);
	virtual int handle_damage(SpaceLocation *source, double normal, double direct=0);
};


MSupgrade::MSupgrade(Vector2 opos, char *datafilename, UpgradeCode oupgradecode, double orespawntime)
:
MissionObject(opos, datafilename, "OBJUPGRADE_DAT")
{
	// actually ... in this case, the upgrade can have >1 sprites, each indicating
	// a different type ... but for now, only the default type (extra crew) ?

	waittimer = 0;

	respawntime = orespawntime;
	upgradecode = oupgradecode;
	sprite_index = upgradecode-1;

}

int MSupgrade::handle_damage(SpaceLocation *source, double normal, double direct)
{
	if (source->isShip())
	{
		Ship *s = (Ship*) source;

		switch(upgradecode)
		{
		case upgr_crew:
			s->crew += 10;
			if (s->crew > s->crew_max)
				s->crew = s->crew_max;
			break;
		case upgr_crewmax:
			s->crew_max += 10;
		}
	}

	if (respawntime >= 0)
		waittimer = respawntime;	// this thing re-appears after a while
	else
		state = 0;					// it is deleted

	collide_flag_anyone = 0;	// disable physical interaction
	return 0;
}

void MSupgrade::calculate()
{
	if (waittimer > 0)
	{
		waittimer -= frame_time * 1E-3;
		if (waittimer <= 0)		// make it tangible again
			collide_flag_anyone = ALL_LAYERS;
	}
	else
		SpaceObject::calculate();

}

void MSupgrade::animate(Frame *space)
{
	if (waittimer <= 0)			// disable drawing
		SpaceObject::animate(space);
}



class MSFactory : public MissionShip
{
public:
	MSFactory(Vector2 opos, char *datafilename, char *ininame, TeamCode team);
	virtual void missioncontrol();

	virtual int activate_weapon();
};




MSFactory::MSFactory(Vector2 opos, char *datafilename,
		char *ininame, TeamCode team)
:
MissionShip(opos, datafilename, "SHPFACTORY_DAT", ininame, "shpfactory", team)
{
}




void MSFactory::missioncontrol()
{

	// simple, always turn around
	turn_left = TRUE;
	thrust = TRUE;		// thus it'll slow down itself when it's hit by something, I think that's useful...

	// unless the enemy is close ; then always face the enemy ?
	// what is the closest enemy? never mind ....

	// also, always try to activate the weapon:
	if (batt >= weapon_drain)
		fire_weapon = TRUE;
	else
		fire_weapon = FALSE;
}



int MSFactory::activate_weapon()
{
	double A;
	Vector2 P;

	// what's the thing that's created: a ship !!

	P = pos;
	A = angle;

	Ship *s;
	s = game->create_ship(game->channel_none, "zfpst", "WussieBot", P, A, get_team());
	game->add(s);

	return TRUE;
}




static const int maxmissions = 128;

class mission;

class gmissions : public MainGame
{
public:
	PopupTextInfo_toggle	*popupinfo;
	AreaReserve		*missionselectmenu;
	WindowManager	*winman;
	Button			*b_accept, *b_quit, *b_left, *b_right;
	TextButton		*b_title, *b_info;
	GhostButton		*b_ghost;
	PopupOk			*alertwin;		// only accept option
	PopupYN			*alertlose;		// accept and retry option

	~gmissions();

	int Nmissions, chosenmission;

	virtual void init(Log *_log) ; //happens when our game is first started

	virtual void calculate() ; 
	virtual void animate(Frame *frame);

	mission		*missionlist[maxmissions];
	void add2list(mission *m);

	void set_info_buttons();

	// I'd like to add some extra functionality (simple: increase the mission number automatically)
	virtual void removesubgame(int k);

	// this starts mission number k
	void start(int k);
};




// Note that the mission has 2 different parts: the Briefing part, whose resources are only
// allocated at the time of the briefing, or when you want to scroll quickly through different
// mission descriptions without actually playing the missions.
// the Play part is allocated when the Briefing has been examined, and the choice is made
// to actually play the thing.

class mission
{
public:
	
	struct Briefing
	{
		char title[titlesize];
		void define_title(char *t);
		
		char briefing[briefingsize];
		void define_briefing(char *t);
	};
	
	
	struct Play : public SubGame
	{
		Control		*playercontrol;
		TeamCode	team_goodguys, team_badguys;
		
		int healthtoggle;	// toggles healthbars on/off ?

		int stopgametime;

		//	virtual void debriefing();
		
		virtual void calculate();
		virtual int quit_condition();
		int quit_value;
		
		int enemycount();
		
		virtual void init(Log *_log);
		
		// stuff you need to set up the map; you're not interested in keeping track of details,
		// so a void return is enough.
		void create_planet(double relx, double rely, int index);
		
		Ship *create_ship(double relx, double rely, char *shpid, Control *c, TeamCode team);
		Ship *create_ship(double relx, double rely, char *shpid, char *c, TeamCode team);
		
		Ship *create_human_ship(double relx, double rely, char *shpid);
		Ship *create_allied_ship(double relx, double rely, char *shpid);
		Ship *create_enemy_ship(double relx, double rely, char *shpid);

		void ship_change_crew(Ship *s, double pcrew, double pcrewmax);
		void ship_change_batt(Ship *s, double pbatt, double pbattmax, double prechargeamount, double prechargerate);
		void ship_change_speed(Ship *s, double pspeed, double paccel, double pturn);
	};

	// give the following no body, so it must be defined by the inheritors ?!
	virtual Briefing *initbriefing(){return 0;};	// this returns the briefing info (pointer)
	virtual Play *initgame(){return 0;};			// this returns the game info (pointer)
	// this is necessary, cause you want to initialize a high-level (overloaded) function
	// from a call at a lower level - that must be done through a high-level (overloaded)
	// initialization routine [that's a trick to create virtual constructors - got that
	// from a book].

	Play *playgame;		// can be used to acces the quit_value later on.
};



class mission_defeat_vux : public mission
{
public:

	struct B : mission::Briefing
	{
		B();
	};

	struct P : mission::Play
	{
		virtual void init(Log *_log);
		virtual int quit_condition();
	};

	virtual Briefing *initbriefing(){return new B();};	// this returns the briefing info (pointer)
	virtual Play *initgame(){return playgame = new P();};			// this returns the game info (pointer)
};




class mission_defeat_alari : public mission
{
public:

	struct B : mission::Briefing
	{
		B();
	};

	struct P : mission::Play
	{
		virtual void init(Log *_log);
		virtual int quit_condition();
	};

	virtual Briefing *initbriefing(){return new B();};	// this returns the briefing info (pointer)
	virtual Play *initgame(){return playgame = new P();};			// this returns the game info (pointer)
};



class mission_protect_chenjesu : public mission
{
public:
	struct B : mission::Briefing
	{
		B();
	};

	struct P : mission::Play
	{
		Ship	*chen;
		int		event_counter;
		double	minutes;
		
	  //mission_protect_chenjesu();
	  void mission_protect_chenjesu(); //added void 7/1/2003 Culture20
		
		virtual void init(Log *_log);
		virtual void calculate();
		virtual int quit_condition();
	};

	virtual Briefing *initbriefing(){return new B();};	// this returns the briefing info (pointer)
	virtual Play *initgame(){return playgame = new P();};			// this returns the game info (pointer)
};


class mission_escape_from_alcatraz : public mission
{
public:

	struct B : mission::Briefing
	{
		B();
	};
	
	struct P : mission::Play
	{
		SpecialArea *specialarea;
		Ship *playership;
		
	  //mission_escape_from_alcatraz(); 
		void mission_escape_from_alcatraz(); //added void 7/1/2003 Culture20
		
		virtual void init(Log *_log);
		virtual int quit_condition();
	};

	virtual Briefing *initbriefing(){return new B();};	// this returns the briefing info (pointer)
	virtual Play *initgame(){return playgame = new P();};			// this returns the game info (pointer)
};


class mission_destroy_factory : public mission
{
public:

	struct B : mission::Briefing
	{
		B();
	};

	struct P : mission::Play
	{
		Ship *factory;
		
		virtual void init(Log *_log);
		virtual int quit_condition();
	};

	virtual Briefing *initbriefing(){return new B();};	// this returns the briefing info (pointer)
	virtual Play *initgame(){return playgame = new P();};			// this returns the game info (pointer)
};






gmissions::~gmissions()
{
	// delete the listed games
	
	int i;
	for ( i = 0; i < Nmissions; ++i )
		delete missionlist[i];
}


void gmissions::add2list(mission *m)
{
	if (Nmissions < maxmissions)
	{
		missionlist[Nmissions] = m;
		++Nmissions;
	}
}


void gmissions::removesubgame(int k)
{
	if (k == 1)		// otherwise, you've some error?
	{
		// let the player know whether you win or lose
		// 1 = lose
		// 2 = win
		int i;
		i = missionlist[chosenmission]->playgame->quit_value;

		if ( i == 1 )
			alertlose->show();
		if ( i == 2 )
			alertwin->show();

		if ( chosenmission < Nmissions-1 && i == 2 )		// only advance if you win.
		{
			++chosenmission;
		}

		set_info_buttons();

	} else {
		tw_error("unexpected number of games are active (only 2 expected)");
	}

	MainGame::removesubgame(k);
}


void gmissions::set_info_buttons()
{

	mission::Briefing *briefing;
	briefing = missionlist[chosenmission]->initbriefing();		// initialize information.

	// something short to remember the mission by.
	char *t;
	t = briefing->title;
	b_title->set_text(t, 1);

	// text of the briefing - a short background story, and the intention of the mission
	t = briefing->briefing;
	popupinfo->tia->set_textinfo(t, strlen(t));

	// the mission number
	char num[32];
	sprintf(num, "%i", chosenmission+1);
	b_info->set_text(num, makecol(255,255,128));

	delete briefing;	// the info has been copied - no need to keep it.
}


void gmissions::init(Log *_log)
{
	MainGame::init(_log);

	Nmissions = 0;

	// well .. start adding missions, and manage them !!!

	add2list( new mission_defeat_vux() );
	add2list( new mission_defeat_alari() );
	add2list( new mission_protect_chenjesu() );
	add2list( new mission_escape_from_alcatraz() );
	add2list( new mission_destroy_factory() );

	chosenmission = 0;



	// initialize the interface ?


	unscare_mouse();
	show_mouse(window->surface);


	// which font to use ... that depends on the screen resolution:
	int i = 2;
	if (screen->w == 640)
		i = 2;
	if (screen->w == 800)
		i = 3;
	if (screen->w == 1024)
		i = 4;					// more pixels available for the same "real-life" size.

	i += 2;	// choose a larger font, for easier reading.

	FONT *usefont = videosystem.get_font(i);

//	char *infotext;
//	int L;

	this->view->frame->prepare();

	missionselectmenu = new AreaReserve("SELECTMISSION", 50, 400, "gmissiongui.dat", this->view->frame->surface);

	b_info = new TextButton(missionselectmenu, "INFO", -1, -1, usefont);

	b_accept = new Button(missionselectmenu, "ACCEPT", -1, -1, KEY_ENTER);
	b_quit = new Button(missionselectmenu, "QUIT", -1, -1, KEY_ESC);

	b_title = new TextButton(missionselectmenu, "TITLE", -1, -1, usefont);

	b_left = new Button(missionselectmenu, "LEFT", -1, -1, KEY_LEFT);
	b_right = new Button(missionselectmenu, "RIGHT", -1, -1, KEY_RIGHT);

	b_ghost = new GhostButton(missionselectmenu);

	popupinfo = new PopupTextInfo_toggle(b_ghost, "POPUPINFO", -50, -200, "gmissiongui.dat", usefont, "", 0);
	popupinfo->option.disable_othermenu = false;
	popupinfo->option.place_relative2mouse = false;	// hmm, a bit late, after it's already been placed... oh well, never mind ...
	popupinfo->show();	// by default it's inactive, but I'd prefer it's active right away.
//	delete infotext;

	alertlose = new PopupYN("ALERTLOSE", 400, 300, "gmissiongui.dat", this->view->frame->surface);
	alertwin  = new PopupOk("ALERTWIN",  400, 300, "gmissiongui.dat", this->view->frame->surface);

	winman = new WindowManager;
	winman->add(missionselectmenu);
	winman->add(popupinfo);
	winman->add(alertlose);
	winman->add(alertwin);


	set_info_buttons();
}



void gmissions::start(int k)
{
	// start the mission
	// ok .. when a mission is played, it's deleted, and can't be played again ... what to do about that ?!
	SubGame *g;
	g = missionlist[k]->initgame();
	addsubgame(g);
	// note, the game (g) is automatically deleted in removesubgame when the game is played
}

void gmissions::calculate()
{
	MainGame::calculate();

	if (Nsubgames == 1)		// if only the manager is present
	{
		
		FULL_REDRAW = true;
		winman->calculate();




		// only "lose" has a retry shortcut option.
		if (alertlose->returnvalueready)
		{
			if (alertlose->getvalue() == 1)	// yes
				start(chosenmission);
		}
		

		
		// depending on the alert window, determine the state of the other items
		//
		// this should be done on a lower level than this but for now, it's ok I think
		//

		// if not both alert windows disabled
		if (!(alertwin->disabled && alertlose->disabled))
		{
			// then deactivate the other menu
			if (!missionselectmenu->disabled)
				missionselectmenu->disable();

			if (!popupinfo->disabled)
				popupinfo->disable();
		} else {
			// otherwise, make sure the other i/o possibility is activated
			if (missionselectmenu->disabled)
				missionselectmenu->show();

			if (popupinfo->disabled)
				popupinfo->show();
		}
		

		int ilast = chosenmission;
		
		if (b_right->flag.left_mouse_press)
		{
			++chosenmission;
			if (chosenmission > Nmissions-1)
				chosenmission = Nmissions-1;
		}
		
		
		if (b_left->flag.left_mouse_press)
		{
			--chosenmission;
			if (chosenmission < 0)
				chosenmission = 0;
		}
		
		if (chosenmission != ilast)
		{
			set_info_buttons();
		}
		
		if (b_quit->flag.left_mouse_press)
			quit("Thanks for playing");
		
		if (b_accept->flag.left_mouse_press)
		{
			start(chosenmission);
			/*
			// start the mission
			// ok .. when a mission is played, it's deleted, and can't be played again ... what to do about that ?!
			SubGame *g;
			g = missionlist[chosenmission]->initgame();
			addsubgame(g);
			// note, the game (g) is automatically deleted in removesubgame when the game is played
			*/
		}

	}

}


void gmissions::animate(Frame *frame)
{

	if (Nsubgames == 1)		// if only the manager is present
	{
		
		MainGame::animate(frame);
		
		//show_mouse(frame->surface);
		scare_mouse();
		winman->setscreen(view->frame->surface);
		winman->animate();

		unscare_mouse();
		show_mouse(view->frame->surface);
	}

}









/*
mission::Briefing *mission::initbriefing()
{
	return 0;	// this must be overwritten
}

mission::Play *mission::initgame()
{
	return 0;	// this must be overwritten
}
*/


void mission::Briefing::define_title(char *t)
{
	strncpy(title, t, titlesize-1);
	title[titlesize-1] = 0;
}

void mission::Briefing::define_briefing(char *t)
{
	strncpy(briefing, t, briefingsize-1);
	briefing[briefingsize-1] = 0;
}



void mission::Play::init(Log *_log)
{
	SubGame::init(_log);

	prepare();

	add(new Stars());

	team_goodguys = new_team();
	team_badguys = new_team();

	playercontrol = create_control(channel_server, "Human");
	playercontrol->target_sign_color = 4;
	//This makes that controller draw a target sign in red
	//the number is a pallete index
	//to hide the target sign again, set target_sign_color to -1
	add_focus(playercontrol, channel_server);
	//Here we make the keyboard controller a screen focus
	//this means that the camera will track whatever ship is controlled by this controller
	//so that we don't have to call add_focus again every time the controller gets a new ship
	//BTW, if you have multiple focuses, the camera will track one of them, and the player can switch which one by pressing F3
	
	/*
	if ((log->type == Log::log_net1server) || (log->type == Log::log_net1client)) { 
		human_control[1] = create_control(channel_client, "Human");
		human_control[1]->target_sign_color = 2;
		add_focus(human_control[1], channel_client);
		}
	else human_control[1] = NULL;

	respawn_time = 500;

	int i;
	for (i = 0; i < 7; i += 1) add(new Asteroid());

	if (log->type == Log::log_net1server) fleet.load("fleets/all.scf", "Fleet");
	log_fleet(channel_server, &fleet);
	//this time, instead of transmitting the fleet file over the network and then loading 
	//it on both sides
	//we instead load it on the server, and then transmit the fleet data (rather than the 
	//raw file)
	*/


	stopgametime = 0.0;
	quit_value = 0;
}


void mission::Play::calculate()
{
	SubGame::calculate();

	
	if ( quit_value != 0)
	{
		if ( (get_time() - stopgametime)*1E-3 > 3.0)	// stop the game with a delay of 3 sec's
			quit("quit?");	// this sets game_done, and results in a call to removesubgame
	}
	else
	{
		quit_value = quit_condition();
		stopgametime = get_time();
	}
}

int mission::Play::quit_condition()
{
	return 0;
}


void mission::Play::create_planet(double relx, double rely, int index)
{
	game->add( new Planet(map_size * Vector2(relx, rely), game->planetSprite, index) );
//	Planet(Vector2 location, SpaceSprite *sprite, int index);
}


Ship *mission::Play::create_ship(double relx, double rely, char *shpid, Control *c, TeamCode team)
{
	Ship *s;
	s = Game::create_ship(shpid, c, map_size*Vector2(relx, rely), random(2*PI), team);
	add(s->get_ship_phaser());
	add(new HealthBar(s, &healthtoggle));
	return s;
}

Ship *mission::Play::create_ship(double relx, double rely, char *shpid, char *c, TeamCode team)
{
	Ship *s;
	s = Game::create_ship(channel_server, shpid, c, map_size*Vector2(relx, rely), random(2*PI), team);
	add(s->get_ship_phaser());
	add(new HealthBar(s, &healthtoggle));
	return s;
}

Ship *mission::Play::create_human_ship(double relx, double rely, char *shpid)
{
	return Play::create_ship(relx, rely, shpid, playercontrol, team_goodguys);
}

Ship *mission::Play::create_allied_ship(double relx, double rely, char *shpid)
{
	return Play::create_ship(relx, rely, shpid, "WussieBot", team_goodguys);
}

Ship *mission::Play::create_enemy_ship(double relx, double rely, char *shpid)
{

	return Play::create_ship(relx, rely, shpid, "WussieBot", team_badguys);
}


void mission::Play::ship_change_crew(Ship *s, double pcrew, double pcrewmax)
{
	s->crew *= pcrew / 100.0;
	s->crew_max *= pcrewmax / 100.0;
}

void mission::Play::ship_change_batt(Ship *s, double pbatt, double pbattmax, double prechargeamount, double prechargerate)
{
	s->batt *= pbatt / 100.0;
	s->batt_max *= pbatt / 100.0;
	s->recharge_amount  *= prechargeamount / 100.0;
	s->recharge_rate /= prechargerate / 100.0;
}

void mission::Play::ship_change_speed(Ship *s, double pspeed, double paccel, double pturn)
{
	s->speed_max *= pspeed * 0.01;
	s->accel_rate *= paccel * 0.01;
	s->turn_step *= pturn * 0.01;
}

int mission::Play::enemycount()
{
	int i, count;

	count = 0;

	for ( i = 0; i < num_targets; ++i )
	{
		if (target[i] && target[i]->get_team() == team_badguys )
			if (target[i]->ship && target[i]->ship->crew > 0)
				++count;
	}

	return count;
}








// ****************************************************
//                a mission
// ****************************************************


mission_defeat_vux::B::B()
{
	define_title("defeat the poor lonely VUX");

	define_briefing("The title gives it all away - except that you're flying in "
		"an overpowered dreadnought and that the VUX doesn't have a chance.");
}


void mission_defeat_vux::P::init(Log *_log)
{
	Ship *s;

	Play::init(_log);

	//create_asteroid_belt(0.5, 0.5,  0.1, 0.2, 14,  10);	// center, min/max radius, av. vel, number ?!
	// class : AsteroidBelt ??
	// other possibilities: AsteroidZone // center, radius, number (a buncha asteroids)
	int i;
	for (i = 0; i < 30; ++i)
		game->add(new AsteroidBelt(0.5*map_size, 800, 1200, 30.0));


	create_planet(0.5, 0.5, 1 );

	// the human player
	s = create_human_ship(0.3, 0.3, "kzedr");
	ship_change_batt(s, 200, 200, 200, 200);

	// enemy ships
	create_enemy_ship(0.7, 0.7, "vuxin");
}


int mission_defeat_vux::P::quit_condition()
{
	
	if (playercontrol->ship->crew == 0)
		return 1;		// the player is dead !! loser ;)

	if (enemycount() == 0)
		return 2;		// no enemies left.


	return 0;
}



// ****************************************************
//                another mission
// ****************************************************


mission_defeat_alari::B::B()
{
	define_title("destroy the Alari battlecruiser");

	define_briefing("In a remote system, far far away, an amazing battle is about to ensue. "
		"A scouting fleet of the UrQuan meets an Alari patrol in the remote United Territories. "
		"The Alari, a proud and highly advanced race, with a written history that dates back "
		"over a million years, is about to meet their doom. Ha ha ha haaaa.");
}


void mission_defeat_alari::P::init(Log *_log)
{
	Ship *s;

	Play::init(_log);

	//create_asteroid_belt(0.5, 0.5,  0.1, 0.2, 14,  10);	// center, min/max radius, av. vel, number ?!
	// class : AsteroidBelt ??
	// other possibilities: AsteroidZone // center, radius, number (a buncha asteroids)

	create_planet(0.5, 0.5, 1 );

	// the human player
	s = create_human_ship(0.1, 0.1, "kzedr");
	ship_change_batt(s, 200, 200, 200, 200);

	// allied ships
	// (10 is too many, maybe 7 is better?)
//	int i;

	double a, da;

	da = PI2 / 7;
	a = 0;
	create_allied_ship(0.1 + 0.05 * cos(a), 0.1 + 0.05 * sin(a), "umgdr");
	a += da;
	create_allied_ship(0.1 + 0.05 * cos(a), 0.1 + 0.05 * sin(a), "vuxin");
	a += da;
	create_allied_ship(0.1 + 0.05 * cos(a), 0.1 + 0.05 * sin(a), "spael");
	a += da;
	create_allied_ship(0.1 + 0.05 * cos(a), 0.1 + 0.05 * sin(a), "mycpo");
	a += da;
	create_allied_ship(0.1 + 0.05 * cos(a), 0.1 + 0.05 * sin(a), "andgu");
	a += da;
	create_allied_ship(0.1 + 0.05 * cos(a), 0.1 + 0.05 * sin(a), "ilwav");
	a += da;
	create_allied_ship(0.1 + 0.05 * cos(a), 0.1 + 0.05 * sin(a), "kzedr");

	// enemy ships
	s = create_enemy_ship(0.7, 0.7, "alabc");
	create_enemy_ship(0.7, 0.1, "alabc");
}


int mission_defeat_alari::P::quit_condition()
{
	
	if (playercontrol->ship->crew == 0)
		return 1;		// the player is dead !! loser ;)

	if (enemycount() == 0)
		return 2;		// no enemies left.


	return 0;
}




// ****************************************************
//                another mission
// ****************************************************



mission_protect_chenjesu::B::B()
{
	define_title("protect the Chenjesu");

	define_briefing("A Chenjesu captain came under attack in enemy space. He fought "
		"valliantly and was victorious, but in the fight his engines were "
		"damaged and he's unable to warp to safety. Fortunately you were closeby and "
		"were able to pick up his weak distress call. You immediately radioed for "
		"additional forces, but until they arrive the safety and well-being of the "
		"Chenjesu crew is in your hands.");
}


void mission_protect_chenjesu::P::init(Log *_log)
{
	Ship *s;

	Play::init(_log);

	//create some asteroids
	int i;
	for ( i = 0; i < 6; ++i )
		add(new Asteroid());
	
	// and an asteroid zone
	for ( i = 0; i < 5; ++i )
		add( new AsteroidZone( Vector2(0.2,0.3) * map_size, 500.0, 10.0) );

	create_planet(0.5, 0.5, 1 );

	// the human player
	s = create_human_ship(0.1, 0.1, "kzedr");
	ship_change_batt(s, 200, 200, 200, 200);

	// allied ship
	chen = create_allied_ship(0.15, 0.1, "chebr");
	ship_change_crew(chen, 80, 100);		// fewer crew
	ship_change_batt(chen, 10, 10, 10, 10);	// little offensive power
	ship_change_speed(chen, 30, 10, 50);	// very slow

	// enemy ships
	create_enemy_ship(0.7, 0.7, "umgdr");


	// place a few crew upgrades (randomly)
	
	for ( i = 0; i < 2; ++i )
	{
		add( new MSupgrade(map_size*tw_random(Vector2(1.0,1.0)), "gmissionobjects.dat", upgr_crew, 20.0) );
	}

	minutes = 0.0;
	event_counter = 0;
}



void mission_protect_chenjesu::P::calculate()
{
	Play::calculate();

	// timed arrival of ships ?!
	minutes = game_time/(1000.0 * 60.0);

	switch (event_counter)
	{
	case 0:
		if (minutes < 0.5) break;
		create_enemy_ship(0.25, 0.25, "mycpo");
		create_enemy_ship(0.90, 0.90, "zfpst");
		++event_counter;
	break;
	case 1:
		if (minutes < 1.0) break;	// a group of shofixties appear near the planet
		create_enemy_ship(0.45, 0.45, "shosc");
		create_enemy_ship(0.45, 0.55, "shosc");
		create_enemy_ship(0.50, 0.45, "shosc");
		++event_counter;
	break;
	case 2:
		if (minutes < 1.5) break;
		create_enemy_ship(0.25, 0.25, "druma");
		++event_counter;
	break;
	case 3:
		if (minutes < 2.0) break;
		int i;
		for ( i = 0; i < 3; ++i )
			create_enemy_ship(random(1.0), random(1.0), "umgdr");
		++event_counter;
	break;
	case 4:
		if (minutes < 3.0) break;
		for ( i = 0; i < 3; ++i )
			create_enemy_ship(random(1.0), random(1.0), "mycpo");
		++event_counter;
	break;
	case 5:
		if (minutes < 3.1) break;
		for ( i = 0; i < 6; ++i )
			create_allied_ship(random(1.0), random(1.0), "syrpe");
		++event_counter;
	break;
	}


}

int mission_protect_chenjesu::P::quit_condition()
{
	
	if ( (playercontrol->ship->crew == 0) || (chen->crew == 0) )
		return 1;		// the player or his protege is dead !! loser ;)

	if (minutes > 4.0 && enemycount() == 0)
		return 2;		// no enemies left, and reinforcements have arrived.


	return 0;
}


// ****************************************************
//                another mission
// ****************************************************



mission_escape_from_alcatraz::B::B()
{
	define_title("Escape from Alcatraz");

	define_briefing("The Druuge planet Alcatraz is renowned for it's inhospitability. "
		"You want to leave A.S.A.P. but, the Druuge forces won't allow you to "
		"just like that. You hadn't spend enough of your precious RU's there yet, "
		"especially on that old Shofixty Scout of them that you took for a test flight. "
		"They want something in return and if necessary, your dead body will do. "
		"Your only chance of escaping this infernal system as a free human lies in the escape area, "
		"where a cloaked ship is waiting to take you to the safety of hyperspace!");
}


void mission_escape_from_alcatraz::P::init(Log *_log)
{
//	Ship *s;

	Play::init(_log);

	int i;
	for (i = 0; i < 30; ++i)
		game->add(new AsteroidBelt(0.5*map_size, 600, 1000, 30.0));

	create_planet(0.5, 0.5, 1 );

	// the human player
	playership = create_human_ship(0.55, 0.55, "shosc");
	ship_change_batt(playership, 10, 10, 10, 10);
	ship_change_crew(playership, 20, 100);

	specialarea = new SpecialArea(Vector2(0,0), 50);
	game->add( specialarea );
	game->add( new WedgeIndicator(specialarea, 50, 2 ) );	// this uses a palette indicator to choose a color

	// enemy ships
	for (i = 0; i < 12; ++i )
	{
		create_enemy_ship(random(1.0), random(1.0), "druma");
	}

}


int mission_escape_from_alcatraz::P::quit_condition()
{
	
	if (playercontrol->ship->crew == 0)
		return 1;		// the player is dead !! loser ;)

	if ( specialarea->inreach(playership) )
		return 2;		// you reached your destination !! congrat.


	return 0;
}


// ****************************************************
//                another mission
// ****************************************************


mission_destroy_factory::B::B()
{
	define_title("Destroy the enemy factory");

	define_briefing("The enemy has warped in a factory compound in Beta Draconis IV. "
		"They are quickly producing a fleet, fortunately you were around to spot them in time. "
		"Destroy the factory before the enemy takes a firm foothold. Track down the attackers "
		"to locate the factory. If you lose too many crew in the process, make use of the "
		"crew upgrades that are floating around (+10 each).");
}


void mission_destroy_factory::P::init(Log *_log)
{
	Ship *s;


	Play::init(_log);

	size = Vector2(15000,15000);
	prepare();

	//create_asteroid_belt(0.5, 0.5,  0.1, 0.2, 14,  10);	// center, min/max radius, av. vel, number ?!
	// class : AsteroidBelt ??
	// other possibilities: AsteroidZone // center, radius, number (a buncha asteroids)

	create_planet(0.5, 0.5, 1 );

	// the human player
	s = create_human_ship(0.6, 0.5, "kzedr");
	ship_change_batt(s, 200, 200, 200, 200);

	factory = new MSFactory(Vector2(0,0), "gmissionobjects.dat", "gmissionobjects.ini",
								team_badguys);
	add( factory );
	add_target(factory);	// so that UQ fighters (and homingmissiles?) also see it as a target.
	add(new HealthBar(factory, &healthtoggle));

	// place a few crew upgrades (randomly)	// quite a few, since it's a big map
	int i;
	for ( i = 0; i < 15; ++i )
	{
		add( new MSupgrade(map_size*tw_random(Vector2(1.0,1.0)), "gmissionobjects.dat", upgr_crew, 20.0) );
	}


	for ( i = 0; i < 3; ++i )
	{
		add( new MSupgrade(map_size*tw_random(Vector2(1.0,1.0)), "gmissionobjects.dat", upgr_crewmax, -1) );
	}
}


int mission_destroy_factory::P::quit_condition()
{
	
	if (playercontrol->ship->crew == 0)
		return 1;		// the player is dead !! loser ;)

	if (factory->crew == 0)
		return 2;		// the spawn point is destroyed.

	return 0;
}




REGISTER_GAME(gmissions, "Geomans Campaign")

