// missionorder

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
#include "../ais.h"

#include "gamehierarchy.h"

#include "../twgui/twgui.h"
#include "../twgui/twmenuexamples.h"

#include "gflmelee.h"
#include "gmissions_objects.h"

#include "gplexplr.h"

#include <string.h>
#include <stdio.h>


static const int titlesize = 64;
static const int briefingsize = 2048;









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
	STACKTRACE

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
	STACKTRACE

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
	STACKTRACE

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
	STACKTRACE


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
	STACKTRACE

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
	TWindow			*missionselectmenu;
	//WindowManager	*winman;
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
	
	SpaceLocation	*gravwell[32];
	int				Ngravwells;
	void addgravwell(SpaceLocation *s);
	virtual Planet *nearest_planet(SpaceLocation *s);
	
	
	int healthtoggle;	// toggles healthbars on/off ?
	int		event_counter;
	
	MissionPointer	*missionpointer;
	Stars			*stars;
	
	int stopgametime;
	double minutes;
	
	//	virtual void debriefing();
	
	virtual void calculate();
	virtual int quit_condition();
	int quit_value;
	
	int enemycount();
	
	virtual void init(Log *_log);
	
	// stuff you need to set up the map; you're not interested in keeping track of details,
	// so a void return is enough.
	Planet *create_planet(double relx, double rely, int index);
	
	Ship *create_ship(double relx, double rely, char *shpid, Control *c, TeamCode team);
	Ship *create_ship(double relx, double rely, char *shpid, char *c, TeamCode team);
	
	Ship *create_human_ship(double relx, double rely, char *shpid);
	Ship *create_allied_ship(double relx, double rely, char *shpid);
	Ship *create_enemy_ship(double relx, double rely, char *shpid);
	Ship *create_enemy_ship(double relx, double rely, char *shpid, char *botname);

	void create_enemies(char *id, int N);
	void create_allies(char *id, int N);
	void create_enemies(char *id, int N, char *botname);
	
	void create_asteroid_belt(Vector2 center, double Rmin, double Rmax, double v, int N);
	void create_asteroids(int N);
	
	void ship_change_crew(Ship *s, double pcrew, double pcrewmax);
	void ship_change_batt(Ship *s, double pbatt, double pbattmax, double prechargeamount, double prechargerate);
	void ship_change_speed(Ship *s, double pspeed, double paccel, double pturn);
	
	void show_message(char *text, double timesec);
	
	// override AI controls, by ordering moves to the ship; by default, aim at going
	// at max speed
	void order_move(Ship *shp, Vector2 loc, double pspeed = 100.0);
	void order_guard(Ship *shp, Vector2 loc, double Rmove, double Rattack, TeamCode team);
	void order_guard(Ship *shp, Vector2 loc, double Rmove, double Rattack, Ship *enemy);

	void set_targets(TeamCode team, SpaceObject *t);

	virtual void ship_died(Ship *victim, SpaceLocation *killer);
	virtual void handle_death(Ship *victim, SpaceLocation *killer);
};


class mission
{
public:



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

	struct B : Briefing
	{
		B();
	};

	struct P : Play
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

	struct B : Briefing
	{
		B();
	};

	struct P : Play
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
	struct B : Briefing
	{
		B();
	};

	struct P : Play
	{
		Ship	*chen;
		
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

	struct B : Briefing
	{
		B();
	};
	
	struct P : Play
	{
		SpecialArea *specialarea;
		Ship *playership;
		
		virtual void init(Log *_log);
		virtual int quit_condition();
	};

	virtual Briefing *initbriefing(){return new B();};	// this returns the briefing info (pointer)
	virtual Play *initgame(){return playgame = new P();};			// this returns the game info (pointer)
};


class mission_destroy_factory : public mission
{
public:

	struct B : Briefing
	{
		B();
	};

	struct P : Play
	{
		Ship *factory;
		
		virtual void init(Log *_log);
		virtual int quit_condition();
	};

	virtual Briefing *initbriefing(){return new B();};	// this returns the briefing info (pointer)
	virtual Play *initgame(){return playgame = new P();};			// this returns the game info (pointer)
};


class mission_protect_official : public mission
{
public:
	struct B : Briefing
	{
		B();
	};

	struct P : Play
	{
		Ship	*ambassador, *terrorist, *player;
		Planet	*centralplanet;

		bool	doshipdamage, missioncomplete;
		
		virtual void init(Log *_log);
		virtual void calculate();
		virtual int quit_condition();
		virtual void handle_death(Ship *victim, SpaceLocation *killer);
	};

	virtual Briefing *initbriefing(){return new B();};	// this returns the briefing info (pointer)
	virtual Play *initgame(){return playgame = new P();};			// this returns the game info (pointer)
};

class mission_protect_official02 : public mission
{
public:
	struct B : Briefing
	{
		B();
	};

	struct P : Play
	{
		virtual void init(Log *_log);
		virtual void calculate();
		virtual int quit_condition();
	};

	virtual Briefing *initbriefing(){return new B();};	// this returns the briefing info (pointer)
	virtual Play *initgame(){return playgame = new P();};			// this returns the game info (pointer)
};


class mission_protect_official03 : public mission
{
public:
	struct B : Briefing
	{
		B();
	};

	struct P : Play
	{
		Plsurface	*plsurface;
		virtual void init(Log *_log);
		virtual void calculate();
		virtual int quit_condition();
	};

	virtual Briefing *initbriefing(){return new B();};	// this returns the briefing info (pointer)
	virtual Play *initgame(){return playgame = new P();};			// this returns the game info (pointer)
};


class mission_protect_official04 : public mission
{
public:
	struct B : Briefing
	{
		B();
	};

	struct P : Play
	{
		Planet *centralplanet;
		Ship		*phedar[128];
		Vector2		guardloc[128];
		int			Nphedar;

		virtual void init(Log *_log);
		virtual void calculate();
		virtual int quit_condition();
	};

	virtual Briefing *initbriefing(){return new B();};
	virtual Play *initgame(){return playgame = new P();};
};


class mission_protect_official05 : public mission
{
public:
	struct B : Briefing
	{
		B();
	};

	struct P : Play
	{
		bool missionloss;
		Ship *informant;
		virtual void init(Log *_log);
		virtual void calculate();
		virtual int quit_condition();
		virtual void handle_death(Ship *victim, SpaceLocation *killer);
	};

	virtual Briefing *initbriefing(){return new B();};
	virtual Play *initgame(){return playgame = new P();};
};



class mission_protect_official06 : public mission
{
public:
	struct B : Briefing
	{
		B();
	};

	struct P : Play
	{
		virtual void init(Log *_log);
		virtual void calculate();
		virtual int quit_condition();
	};

	virtual Briefing *initbriefing(){return new B();};
	virtual Play *initgame(){return playgame = new P();};
};

class mission_protect_official07 : public mission
{
public:
	struct B : Briefing
	{
		B();
	};

	struct P : Play
	{
		virtual void init(Log *_log);
		virtual void calculate();
		virtual int quit_condition();
	};

	virtual Briefing *initbriefing(){return new B();};
	virtual Play *initgame(){return playgame = new P();};
};

class mission_protect_official08 : public mission
{
public:
	struct B : Briefing
	{
		B();
	};

	struct P : Play
	{
		virtual void init(Log *_log);
		virtual void calculate();
		virtual int quit_condition();
	};

	virtual Briefing *initbriefing(){return new B();};
	virtual Play *initgame(){return playgame = new P();};
};

class mission_protect_official09 : public mission
{
public:
	struct B : Briefing
	{
		B();
	};

	struct P : Play
	{
		virtual void init(Log *_log);
		virtual void calculate();
		virtual int quit_condition();
	};

	virtual Briefing *initbriefing(){return new B();};
	virtual Play *initgame(){return playgame = new P();};
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
		{
			alertlose->show();
			//alertlose->focus();
		}
		if ( i == 2 )
		{
			alertwin->show();
			//alertwin->focus();
		}

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

	Briefing *briefing;
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

	// missionorder
	// well .. start adding missions, and manage them !!!
	add2list( new mission_protect_official() );
	add2list( new mission_protect_official02() );
	add2list( new mission_protect_official03() );
	add2list( new mission_protect_official04() );
	add2list( new mission_protect_official05() );
	add2list( new mission_protect_official06() );
	add2list( new mission_protect_official07() );
	add2list( new mission_protect_official08() );
	add2list( new mission_protect_official09() );
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

//	missionselectmenu = new AreaReserve("SELECTMISSION", 50, 400, "gmissiongui.dat", this->view->frame->surface);
	missionselectmenu = new TWindow("interfaces/selectmission/briefing", 50, 400, this->view->frame->surface);

	b_info = new TextButton(missionselectmenu, "info_", usefont);

	b_accept = new Button(missionselectmenu, "accept_", KEY_ENTER);
	b_quit = new Button(missionselectmenu, "quit_", KEY_ESC);

	b_title = new TextButton(missionselectmenu, "title_", usefont);

	b_left = new Button(missionselectmenu, "left_", KEY_LEFT);
	b_right = new Button(missionselectmenu, "right_", KEY_RIGHT);

	b_ghost = new GhostButton(missionselectmenu);

	popupinfo = new PopupTextInfo_toggle(b_ghost, "interfaces/selectmission/popupinfo", "text/", 200, 200, usefont, "", 0);
	popupinfo->option.disable_othermenu = false;
	popupinfo->option.place_relative2mouse = false; // doesn't matter in this case.
	popupinfo->show();	// by default it's inactive, but I'd prefer it's active right away.
//	delete infotext;

	alertlose = new PopupYN("interfaces/selectmission/alertlose", 400, 300, this->view->frame->surface);
	alertwin  = new PopupOk("interfaces/selectmission/alertwin",  400, 300, this->view->frame->surface);

//	winman = new WindowManager;
	missionselectmenu->add(popupinfo);
	missionselectmenu->add(alertlose);
	missionselectmenu->add(alertwin);

	missionselectmenu->tree_doneinit();

	popupinfo->exclusive = false;

	popupinfo->focus();

	set_info_buttons();
}



void gmissions::start(int k)
{
	// start the mission
	// ok .. when a mission is played, it's deleted, and can't be played again ... what to do about that ?!
	/* DISABLED FOR THE RELEASE by Geo.
	Dialog* dialog = new Dialog;
	dialog->Conversate("dialog.lua");
	delete dialog;
	*/
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
		missionselectmenu->tree_calculate();




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
			{
				missionselectmenu->show();
				//missionselectmenu->focus();
			}

			if (popupinfo->disabled)
			{
				popupinfo->show();
				//popupinfo->focus();
			}
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
		missionselectmenu->tree_setscreen(view->frame->surface);
		missionselectmenu->tree_animate();

		unscare_mouse();
		show_mouse(view->frame->surface);
	}

}









/*
mission::Briefing *mission::initbriefing()
{
	return 0;	// this must be overwritten
}

Play *mission::initgame()
{
	return 0;	// this must be overwritten
}
*/


void Briefing::define_title(char *t)
{
	strncpy(title, t, titlesize-1);
	title[titlesize-1] = 0;
}

void Briefing::define_briefing(char *t)
{
	strncpy(briefing, t, briefingsize-1);
	briefing[briefingsize-1] = 0;
}



void Play::init(Log *_log)
{
	SubGame::init(_log);

	prepare();

	stars = new Stars();
	add(stars);

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


	stopgametime = 0;
	quit_value = 0;
	minutes = 0;
	event_counter = 0;

	missionpointer = new MissionPointer(0);
	add(missionpointer);

	Ngravwells = 0;
}


void Play::calculate()
{
	SubGame::calculate();

	
	if ( quit_value != 0)
	{
		if (playercontrol && playercontrol->ship)
		{
			// make the ship unable to die, so that it will persist till the end of the game
			playercontrol->ship->collide_flag_anyone = 0;
			playercontrol->ship->collide_flag_sameship = 0;
			playercontrol->ship->collide_flag_sameteam = 0;
			playercontrol->ship->vel *= 0.1;	// slow the ship down.
		}

		if ( (get_time() - stopgametime)*1E-3 > 3.0)	// stop the game with a delay of 3 sec's
			quit("quit?");	// this sets game_done, and results in a call to removesubgame
	}
	else
	{
		quit_value = quit_condition();
		stopgametime = get_time();
	}

	minutes = game_time/(1000.0 * 60.0);
}

int Play::quit_condition()
{
	return 0;
}


Planet *Play::create_planet(double relx, double rely, int index)
{
	Planet *p;
	p = new Planet(map_size * Vector2(relx, rely), meleedata.planetSprite, index);
	//game->add( p );
	addgravwell(p);	// this adds the planet to a separate gravwell list, for easy location of a planet
	// to compensate for the large search radius, given the large force field.
	return p;
//	Planet(Vector2 location, SpaceSprite *sprite, int index);
}


Ship *Play::create_ship(double relx, double rely, char *shpid, Control *c, TeamCode team)
{
	Ship *s;
	s = Game::create_ship(shpid, c, map_size*Vector2(relx, rely), random(2*PI), team);
	add(s->get_ship_phaser());
	add(new HealthBar(s, &healthtoggle));
	return s;
}

Ship *Play::create_ship(double relx, double rely, char *shpid, char *c, TeamCode team)
{
	Ship *s;
	s = Game::create_ship(channel_none, shpid, c, map_size*Vector2(relx, rely), random(2*PI), team);
	// don't use channel_server for AI players...
	add(s->get_ship_phaser());
	add(new HealthBar(s, &healthtoggle));
	return s;
}

Ship *Play::create_human_ship(double relx, double rely, char *shpid)
{
	return Play::create_ship(relx, rely, shpid, playercontrol, team_goodguys);
}

Ship *Play::create_allied_ship(double relx, double rely, char *shpid)
{
	return Play::create_ship(relx, rely, shpid, "WussieBot", team_goodguys);
}

Ship *Play::create_enemy_ship(double relx, double rely, char *shpid)
{
	return Play::create_ship(relx, rely, shpid, "WussieBot", team_badguys);
}


Ship *Play::create_enemy_ship(double relx, double rely, char *shpid, char *botname)
{
	return Play::create_ship(relx, rely, shpid, botname, team_badguys);
}



void Play::ship_change_crew(Ship *s, double pcrew, double pcrewmax)
{
	s->crew *= pcrew / 100.0;
	s->crew_max *= pcrewmax / 100.0;
}

void Play::ship_change_batt(Ship *s, double pbatt, double pbattmax, double prechargeamount, double prechargerate)
{
	s->batt *= pbatt / 100.0;
	s->batt_max *= pbatt / 100.0;
	s->recharge_amount  *= prechargeamount / 100.0;
	s->recharge_rate /= prechargerate / 100.0;
}

void Play::ship_change_speed(Ship *s, double pspeed, double paccel, double pturn)
{
	s->speed_max *= pspeed * 0.01;
	s->accel_rate *= paccel * 0.01;
	s->turn_step *= pturn * 0.01;
}

int Play::enemycount()
{
	int i, count;

	count = 0;

	for ( i = 0; i < gametargets.N; ++i )
	{
		if (gametargets.item[i] && gametargets.item[i]->get_team() == team_badguys )
			if (gametargets.item[i]->ship && gametargets.item[i]->ship->crew > 0)
				++count;
	}

	return count;
}




void Play::show_message(char *text, double timesec)
{

	// define some bitmap with text
	BITMAP *b;

	int i = 2;
	if (screen->w == 640)
		i = 2;
	if (screen->w == 800)
		i = 3;
	if (screen->w == 1024)
		i = 4;					// more pixels available for the same "real-life" size.
	i += 2;	// choose a larger font, for easier reading.

	FONT *usefont = videosystem.get_font(i);

	int h0, H, L;

	L = text_length(usefont, text)+6*text_length(usefont, " ");
	h0 = text_height(usefont);
	H = 1.5 * h0;

	b = create_bitmap_ex(32, L, H);
	clear_to_color(b, makecol(100,80,60));
	textout_centre(b, usefont, text, L/2, (H-h0)/2, makecol(255,255,128));

	// add this to the game for a short while
	int ix, iy;
	ix = (screen->w - L) / 2;
	iy = 0.01 * screen->h;

	add( new MissionMessageBox(b, ix, iy, timesec ) );
}



void Play::order_move(Ship *shp, Vector2 loc, double pspeed)
{
	if (!(shp && shp->exists()))
		return;

	// reset the keys for this ship
	shp->nextkeys = 0;

	// re-define the keys
	double a, da, db, dc;
	a = min_delta(loc - shp->pos, map_size).atan();

	// direction
	da = a - shp->vel.atan();
	while (da < -PI)	da += PI2;
	while (da >  PI)	da -= PI2;

	db = a - shp->angle;
	while (db < -PI)	db += PI2;
	while (db >  PI)	db -= PI2;

	dc = shp->vel.atan() - shp->angle;
	while (dc < -PI)	dc += PI2;
	while (dc >  PI)	dc -= PI2;

	// don't turn so quickly that you'd come at a complete stop (180 degr)
	// but, some anti-thrust can be healthy otherwise you may keep circling?
	if (fabs(dc) > 0.6 * PI)
		da = dc;
	else
	{
		a = fabs(da/PI);
		if (a > 0.5) a = 0.5;
		da = a*da + (1-a)*db;
		// for a really big velocity deviation, attention goes to correcting that;
		// otherwise, the ship's direction should be focus
	}
		
	if (da > 0.01*PI)
	{
		shp->nextkeys |= keyflag::right;
		shp->nextkeys |= keyflag::thrust;	// you _must_ thrust, otherwise you can't steer.
	}
	if (da < -0.01*PI)
	{
		shp->nextkeys |= keyflag::left;
		shp->nextkeys |= keyflag::thrust;
	}
	

	double d = 1.0;

	if (shp->vel.length() < 0.01*pspeed * d * shp->speed_max)	// move at some percentage of max speed
		shp->nextkeys |= keyflag::thrust;
	else
		shp->vel *= (1 - 0.1*frame_time*1E-3);		// ok, very artificial, but, what the heck ;)
	/*
	// thrust if you're going to slow ?
	double d;
	d = (shp->pos - loc).length();
	d -= 250;		// 250 pixels zone of 0 velocity.
	d /= 500;		// at 500 distance from 0-vel zone slow down, otherwise go for max thrust.
	if (d > 1)
		d = 1;
	if (d < 0)
		d = 0;

	else if (d < 1)
	{
		// invert the controls - so that the ship turns away from the loc. and can slow down.
		shp->nextkeys &= ~keyflag::right;	// ?????
		shp->nextkeys &= ~keyflag::thrust;
	}
	*/
}




void Play::addgravwell(SpaceLocation *s)
{
	game->add(s);
	gravwell[Ngravwells] = s;
	++Ngravwells;
}

Planet *Play::nearest_planet(SpaceLocation *s)
{
//	return Game::nearest_planet(s);

	int i;
	double Rclosest, Rtest;
	
	Rclosest = 1E99;
	SpaceLocation *oclosest;

	oclosest = 0;
	if (Ngravwells == 0)
		oclosest = 0;

	else if (Ngravwells == 1)	// a common case
		oclosest = gravwell[0];

	else
	{
		for ( i = 0; i < Ngravwells; ++i )
		{
			Rtest = gravwell[i]->distance(s);
			if ( Rtest < Rclosest)
			{
				Rclosest = Rtest;
				oclosest = gravwell[i];
			}
		}
	}

	if (oclosest && !oclosest->isPlanet())
	{
		tw_error("Found a weird gravwell");
	}

	return (Planet*) oclosest;
}


void turnto(Ship *shp, Vector2 loc)
{
	// re-define the keys
	double a, da, db, dc;
	a = min_delta(loc - shp->pos, map_size).atan();

	// direction
	da = a - shp->vel.atan();
	while (da < -PI)	da += PI2;
	while (da >  PI)	da -= PI2;

	db = a - shp->angle;
	while (db < -PI)	db += PI2;
	while (db >  PI)	db -= PI2;

	dc = shp->vel.atan() - shp->angle;
	while (dc < -PI)	dc += PI2;
	while (dc >  PI)	dc -= PI2;

	// don't turn so quickly that you'd come at a complete stop (180 degr)
	// but, some anti-thrust can be healthy otherwise you may keep circling?
	if (fabs(dc) > 0.6 * PI)
		da = dc;
	else
	{
		a = fabs(da/PI);
		if (a > 0.5) a = 0.5;
		da = a*da + (1-a)*db;
		// for a really big velocity deviation, attention goes to correcting that;
		// otherwise, the ship's direction should be focus
	}
		
	if (da > 0.01*PI)
	{
		shp->nextkeys |= keyflag::right;
		shp->nextkeys |= keyflag::thrust;	// you _must_ thrust, otherwise you can't steer.
	}
	if (da < -0.01*PI)
	{
		shp->nextkeys |= keyflag::left;
		shp->nextkeys |= keyflag::thrust;
	}
}


// guard against any ship that's closeby (quite inefficient in case there're many ships to check)
// Move around within the guard area ; once an enemy ship gets in range of attack that you
// specify, the ship goes to attack.
void Play::order_guard(Ship *shp, Vector2 loc, double Rmove, double Rattack, TeamCode team)
{
	// check the closest ship-type target
	Ship *t;
	t = (Ship*) find_closest_team(shp, ATTRIB_SHIP, Rattack, team);

	order_guard(shp, loc, Rmove, Rattack, t);
}

// guard against a particular ship (efficient)
// attack range is relative to the ship itself - it's like, the sight range.
// move range (the guard area) is around a fixed point in space.
//   note: this routine takes very little time.
void Play::order_guard(Ship *shp, Vector2 loc, double Rmove, double Rattack, Ship *enemy)
{
	if (!(enemy && enemy->exists()))
		return;

	// check your position
	double Rs, Rt = 0;
	Rs = min_delta(shp->pos, loc, map_size).length();

	if (enemy && enemy->exists())
		Rt = min_delta(enemy->pos, shp->pos, map_size).length();

	// just move around in (or return to) the guard area, if the enemy is out of reach
	// (if the enemy stays in reach, you got a hot pursuit :)
	if (!(enemy && enemy->exists()) || Rt > Rattack)
	{
		// reset the keys for this ship
		shp->nextkeys = 0;

		if (Rs > Rmove)	// move back
			turnto(shp, loc);
		else	// something random?
		{
			switch (random(3))
			{
			case 0:
				shp->nextkeys |= keyflag::right;
				break;
			case 1:
				shp->nextkeys |= keyflag::left;
				break;
			}
		}
		shp->nextkeys |= keyflag::thrust;

		return;
	}


	// otherwise ... there's a ship within range...
	// ... do nothing then : let the normal AI handle it !!

//	shp->AI();
}


void Play::set_targets(TeamCode team, SpaceObject *t)
{
	// check all ships with that name, and set their target !
	int i;
	for ( i = 0; i < gametargets.N; ++i )
	{
		SpaceObject *s;
		s = gametargets.item[i];
		if (s->get_team() == team)
			s->target = t;			// now they should go and attack t ?!
	}
}




void Play::create_asteroid_belt(Vector2 center, double Rmin, double Rmax, double v, int N)
{
	int i;
	for (i = 0; i < N; ++i)
		game->add(new AsteroidBelt(center, Rmin, Rmax, v));
}


void Play::create_asteroids(int N)
{
	int i;
	for (i = 0; i < N; ++i)
		game->add(new Asteroid());
}


void Play::create_enemies(char *id, int N)
{
	int i;

	for ( i = 0; i < N; ++i )
	{
		create_enemy_ship(random(1.0), random(1.0), id);
	}
}


void Play::create_enemies(char *id, int N, char *botname)
{
	int i;

	for ( i = 0; i < N; ++i )
	{
		create_enemy_ship(random(1.0), random(1.0), id, botname);
	}
}



void Play::create_allies(char *id, int N)
{
	int i;

	for ( i = 0; i < N; ++i )
	{
		create_allied_ship(random(1.0), random(1.0), id);
	}
}



void Play::ship_died(Ship *victim, SpaceLocation *killer)
{
	// ... source can be 0
	if (!killer)
		return;

	// handle game events based on certain combinations of who/source :
	handle_death(victim, killer);

	Game::ship_died(victim, killer);
	return;
}

void Play::handle_death(Ship *victim, SpaceLocation *killer)
{
	// nothing: should be specified per game.
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
	//minutes = game_time/(1000.0 * 60.0);

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
		create_enemy_ship(0.4, 0.4, "shosc");
		create_enemy_ship(0.4, 0.6, "shosc");
		create_enemy_ship(0.5, 0.4, "shosc");
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
	for (i = 0; i < 16; ++i )
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
	gametargets.add(factory);	// so that UQ fighters (and homingmissiles?) also see it as a target.
	add(new HealthBar(factory, &healthtoggle));

	// place a few crew upgrades (randomly)	// quite a few, since it's a big map
	int i;
	for ( i = 0; i < 15; ++i )
	{
		add( new MSupgrade(tw_random(map_size), "gmissionobjects.dat", upgr_crew, 20.0) );
	}


	for ( i = 0; i < 3; ++i )
	{
		add( new MSupgrade(tw_random(map_size), "gmissionobjects.dat", upgr_crewmax, -1) );
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



// those were test-missions. Now something which contains more of a story-line.
// not much done yet.

// ****************************************************
//                mission 1
// ****************************************************




mission_protect_official::B::B()
{
	define_title("Protect Yehat ambassador.");

	define_briefing("Protect the ambassador on his holiday trip. ");
}


void mission_protect_official::P::init(Log *_log)
{

	Play::init(_log);

	size = Vector2(10000,10000);
	prepare();

	//create_asteroid_belt(0.5, 0.5,  0.1, 0.2, 14,  10);	// center, min/max radius, av. vel, number ?!
	// class : AsteroidBelt ??
	// other possibilities: AsteroidZone // center, radius, number (a buncha asteroids)

	centralplanet = create_planet(0.5, 0.5, 1 );

	// the human player
	player = create_human_ship(0.6, 0.5, "yehte");
	
	ambassador = create_allied_ship(0.6, 0.52, "yehte");
	ambassador->crew = 1;
	missionpointer->target(ambassador);

	terrorist = 0;

	// toggles, to let certain things take place only once in the mission
	doshipdamage = false;
	missioncomplete = false;

}


void mission_protect_official::P::handle_death(Ship *victim, SpaceLocation *killer)
{
	if (!player)
		return;

	if (ambassador && victim->ally_flag == ambassador->ally_flag && killer->ally_flag == player->ally_flag)
	{
		show_message("HOOT! You killed the ambassador! Are ye mad?", 5.0);
		missioncomplete = true;
		doshipdamage = false;
	}

	if (terrorist && victim->ally_flag == terrorist->ally_flag && killer->ally_flag == player->ally_flag)
	{
		show_message("HOOT! You killed the terrorist! The Queen is displeased...", 5.0);
		missioncomplete = true;
		doshipdamage = false;	// to force a loss.
	}
}


int mission_protect_official::P::quit_condition()
{
	
	if (!player)
	{
		show_message("Dead Yehat don't wear plaid.", 5.0);
		return 1;		// the player is dead !! loser ;)
	}

//	if (!terrorist && event_counter > 0)	// you lose; the terrorist occurs from time-point 1 on.
//	{
//		show_message("You killed the terrorist in cold blood. The Queen is displeased...", 5.0);
//		return 1;
//	}

	if (missioncomplete)
	{
		if (doshipdamage)
			return 2;		// win
		else
			return 1;		// lose
	}

	return 0;
}



void mission_protect_official::P::calculate()
{

	if (terrorist && !terrorist->exists())
		terrorist = 0;
	if (ambassador && !ambassador->exists())
		ambassador = 0;
	if (player && !player->exists())
		player = 0;

	switch (event_counter)
	{
	case 0:
		order_move(ambassador, 0.9*map_size, 75);
		if (minutes < 0.3) break;
		terrorist = create_enemy_ship(0.90, 0.90, "hydcr");
		missionpointer->target(terrorist);
		show_message("HOOT! A ship is sighted, his weapons armed - protect the ambassador!!", 6.0);
		++event_counter;
		break;

	case 1:
		if (ambassador)
		{
		//	terrorist->target = ambassador;	// does this work??
			set_targets(team_badguys, ambassador);
		}

		order_move(ambassador, 0.9*map_size, 75);

		if (terrorist->ship->crew > 16) break;
		show_message("AWK! Hold yer fire, the terrorist surrenders. Escort them to the planet.", 6.0);

		switch_team(terrorist->ally_flag, team_goodguys);
		missionpointer->target(centralplanet);

		ship_change_speed(terrorist, 90, 100, 100);		// make them slightly slower than before
		++event_counter;
		break;

	case 2:
		// order the terrorist to move to the planet (or to follow the player?!)
		if (player)
			order_move(terrorist, player->pos);
		if (terrorist)
		{
			if (terrorist->distance(centralplanet) > 500.0) break;
		}
		else
			break;
		// close to the planet, then the new enemies enter
		// and the Hydrovar moves away
		switch_team(terrorist->ally_flag, team_goodguys);
		missionpointer->target(terrorist);

		create_enemy_ship(0.45, 0.55, "jygst");
		create_enemy_ship(0.45, 0.45, "jygst");
		create_enemy_ship(0.55, 0.45, "jygst");
		show_message("YEEP! UFO's!! Hey, The terrorist is escaping, stay on his tail!", 6.0);
		++event_counter;
		break;

	case 3:

		if (player)
		{
			// if the fight takes long, create a new ship
			if (enemycount() == 1 && !doshipdamage)		// only the hydrovar is remaining, and an undamaged player
			{
				create_enemy_ship(0.45, 0.45, "jygst");
				create_enemy_ship(0.55, 0.55, "jygst");
			}

			// if you've not much health left during fighting, your engines break down.
			if (!doshipdamage && player->crew <= 6)
			{
				ship_change_speed(player, 50, 100, 100);		// crippled
				show_message("By the Queen, our engines are failing! They are getting away!", 6.0);
				doshipdamage = true;
			}
			
			// if the enemy is far away from you, and you're alive, you've "completed" the mission
			order_move(terrorist, normalize(player->pos + 0.3*map_size, map_size));
			if (terrorist)
			{
				if (player->distance(terrorist) < 0.2*map_size.x) break;

			if (doshipdamage)
				show_message("YIP! The terrorist is out of reach, but you fought valiantly.", 6.0);
			else
				show_message("BRAAK! The terrorist is out of reach. Your steering is unworthy.", 6.0);
			} else
				show_message("YIP! You let the terrorists die! The Queen awaits YOU for questioning now!.", 6.0);
			missioncomplete = true;
			// however, if this happens without you being crippled (ie unable to stay on his tail),
			// you fail the mission.

			//++event_counter;	// well, there is no other mission ?!
		}

		break;

	}


	// this should come last? So that you can intercept the controls ?
	Play::calculate();

}




// ****************************************************
//                mission 2
// ****************************************************




mission_protect_official02::B::B()
{
	define_title("Track down the enemy fleet.");

	define_briefing("BRAAK! The assassins disappeared into hyperspace, escorted by a "
		"fleet of the mysterious intruders. They've been seen travelling in the direction of Semuri Omega. "
		"Go there and find out about their identity. High command insists that ye go there alone. "
		"It looks like yer on a suicide mission; good luck.");
}



void mission_protect_official02::P::init(Log *_log)
{

	Play::init(_log);

	size = Vector2(5000,5000);
	prepare();

	create_planet(0.5, 0.5, 2 );

	create_asteroid_belt(0.5*map_size, 1500, 2000, 25.0,  50);

	// the human player
	create_human_ship(0.6, 0.5, "yehte");

	// create some enemy ships (jyglar) :
	create_enemies("jygst", 6);
}


int mission_protect_official02::P::quit_condition()
{
	
	if (playercontrol->ship->crew == 0)
	{
		return 1;		// the player is dead !! loser ;)
	}

	if (enemycount() == 0)
	{
		return 2;		// you win.
	}

	return 0;
}


void mission_protect_official02::P::calculate()
{


	switch (event_counter)
	{
	case 0:
		if (minutes < 0.2) break;
		show_message("YEEP! They are awating ye; ye are trapped, captain.", 6.0);
		++event_counter;
		break;

	}


	// this should come last, So that you can intercept the controls.
	Play::calculate();

}





// ****************************************************
//                mission 3
// ****************************************************




mission_protect_official03::B::B()
{
	define_title("Destroy the enemy base.");

	define_briefing("YIP! Commander, we've intercepted transmission originating from the "
		"planet. Ye must destroy their base on the surface.");
}



void mission_protect_official03::P::init(Log *_log)
{

	Play::init(_log);

	// remove the star bakground
	// hmm, you must also remove them from the game list ???
	stars->state = 0;	// ok, this works.

	// initialize the planet surface (and it's accompanied objects/structures)
	// and set the viewmode to something that's fixed.
	Plsurface *surf = new Plsurface("gmissionobjects.dat", "SURFACE_01_DAT");
	size = surf->size();

	prepare();

	add( surf );
	plsurface = surf;

	change_view("Frozen");
	// done, the planet surface is ready.


	// the human player
	create_human_ship(0.6, 0.5, "yehte");

	SpaceLocation *s;
	s = plsurface->findobject("buildinga", 1);	// find the first object with that name (there's only 1).
	missionpointer->target(s);
}


int mission_protect_official03::P::quit_condition()
{
	
	if (playercontrol->ship->crew == 0)
	{
		return 1;		// the player is dead !! loser ;)
	}

	if (plsurface->countobjects("buildinga") == 0)
	{
		return 2;		// you win.
	}

	return 0;
}


void mission_protect_official03::P::calculate()
{


	
	switch (event_counter)
	{
	case 0:
		if (minutes < 0.05) break;
		show_message("The enemy is located east.", 6.0);
		++event_counter;
		break;

	}


	// this should come last, So that you can intercept the controls.
	Play::calculate();

}




// ****************************************************
//                mission 4
// ****************************************************




mission_protect_official04::B::B()
{
	define_title("Siege of Hyaridad.");

	define_briefing("BWAAK! Ye captured a Jyglar commander, and he was very informative. "
		"He said our enemy is a Hydrovar citizen, who had to be returned to Hydrovar at all cost, and that thanks to yer "
		"interference, he has escaped! His life is in danger, and he has to be found at all cost.\n"
		"The commander requested we take him to the nearest Hydrovar colony - planet IV in the system "
		"of Hyaridad, where he's to meet with the Hydrovar War Council. He says it is a "
		"matter of life and death. Captain, I think ye are involved into something big!");
}



void mission_protect_official04::P::init(Log *_log)
{

	Play::init(_log);

	size = Vector2(6000,6000);
	prepare();

	centralplanet = create_planet(0.5, 0.5, 2 );

	create_asteroids(10);	// a few randomly floating asteroids.

	// the human player
	create_human_ship(0.9, 0.9, "yehte");

	// create some enemy ships (Phedar) :
	int i;
	Nphedar = 100;		// should not exceed 128...
	for ( i = 0; i < Nphedar; ++i )
	{
		double a;
		a = i * PI2 / Nphedar;
		guardloc[i] = Vector2(0.5 + 0.25 * cos(a), 0.5 + 0.25 * sin(a));
		phedar[i] = create_enemy_ship(guardloc[i].x, guardloc[i].y, "phepa", "VegetableBot");
		guardloc[i] = guardloc[i] * map_size;
		phedar[i]->hashotspots = false;			// disable their hotspot trail -- too many objects.
		phedar[i]->assigntarget(playercontrol->ship);

		/*
		// hmm? this makes little difference ??
		phedar[i]->attributes |= ATTRIB_UNDETECTABLE;
		phedar[i]->collide_flag_anyone = 0;
		phedar[i]->collide_flag_sameteam = 0;
		phedar[i]->collide_flag_sameship = 0;
		*/
	}

	missionpointer->target(centralplanet);
}


int mission_protect_official04::P::quit_condition()
{
	
	if (playercontrol->ship->crew == 0)
	{
		return 1;		// the player is dead !! loser ;)
	}

	if (playercontrol->ship->distance(centralplanet) < 150.0)
	{
		return 2;		// you win.
	}

	return 0;
}


void mission_protect_official04::P::calculate()
{

	// stable values are:
	// numitems
	// num_presences
//	message.print(1500, 15, "%i", physics->num_listed);

	switch (event_counter)
	{
	case 0:
		if (minutes < 0.1) break;
		show_message("YEEP! I hope we can reach the planet in 1 piece!", 6.0);
		++event_counter;
		break;

	}

	// always impose some AI onto the Phedar?
	// yeah - otherwise there's chaos instead of a normal "siege"
	int i;
	for ( i = 0; i < Nphedar; ++i )
	{
		if (!phedar[i])
			continue;

		// guard against the good guys
		// needed, otherwise they're all zealots who're running for the player ;)
		if (playercontrol->ship)
			order_guard(phedar[i], guardloc[i], 400.0, 400.0, playercontrol->ship);

		// also check if the phedar is still alive ?
		if (!phedar[i]->exists())
			phedar[i] = 0;
	}

	// this should come last, So that you can intercept the controls.
	Play::calculate();

}


// ****************************************************
//                mission 5
// ****************************************************




mission_protect_official05::B::B()
{
	define_title("Engaging the Phedar.");

	define_briefing("HEEP! It's the Phedar who have invaded Hydrovar space! "
		"Hydrovar command said, the one Hydrovar knowing of the terrorist and his possible whereabouts, is "
		"currently engaged in combat. Many Hydrovar have already perished in battle, and our "
		"man may not return in one piece. The commander suggests we wait and hope for the best, but ye must help him!");
}



void mission_protect_official05::P::init(Log *_log)
{

	Play::init(_log);

	size = Vector2(4000,4000);
	prepare();

	create_planet(0.5, 0.5, 2 );

	create_asteroids(10);	// a few randomly floating asteroids.

	// the human player
	create_human_ship(0.6, 0.5, "yehte");

	// create some enemy ships (Phedar) :
	create_enemies("phepa", 12);

	// and some friendly Hydro ships.
	create_allies("hydcr", 2);

	informant = create_allied_ship(0.1, 0.3, "hydcr");

	game->add( new SpecialAreaTag(informant, 150) );
	missionpointer->target(informant);

	missionloss = false;
}

void mission_protect_official05::P::handle_death(Ship *victim, SpaceLocation *killer)
{
	if (!playercontrol->ship)
		return;

	if (informant && victim->ally_flag == informant->ally_flag)
	{
		if (killer->ally_flag == playercontrol->ship->ally_flag)
			show_message("HOOT! You killed the informant! Are ye mad?", 5.0);
		else
			show_message("HOOT! You failed to save the informant! Mission failed.", 5.0);

		missionloss = true;
	}

}


int mission_protect_official05::P::quit_condition()
{
	
	if (playercontrol->ship->crew == 0 || informant->crew == 0 || missionloss)
	{
		return 1;		// the player is dead !! loser ;)
	}

	if (enemycount() == 0)
	{
		return 2;		// you win.
	}

	return 0;
}


void mission_protect_official05::P::calculate()
{


	switch (event_counter)
	{
	case 0:
		if (minutes < 0.1) break;
		show_message("YEEP! Those Phedar fight without honor!", 6.0);
		++event_counter;
		break;

	}

	// this should come last, So that you can intercept the controls.
	Play::calculate();

}





// ****************************************************
//                mission 6
// ****************************************************




mission_protect_official06::B::B()
{
	define_title("Omicron IV");

	define_briefing("YIP! The enemy has been defeated! Our informant, named Hyowo, "
		"knows the terrorist from several years back, they fought together in the war "
		"against the Phedar, which was all about possession of the Hyaridad system. He was a friendly fellow, "
		"Hyowo doesn't believe he's turned into a interstellar terrorist. The last thing he "
		"heard was that his friend was departing for Omicron; a month later "
		"the Phedar gave up, but he never saw him again. Omicron is a very "
		"inhospitable system, he thinks his friend has perished there. Let's "
		"go there to check!");
}



void mission_protect_official06::P::init(Log *_log)
{

	Play::init(_log);

	size = Vector2(4000,4000);
	prepare();

	create_planet(0.5, 0.5, 2 );

	create_asteroids(30);	// a few randomly floating asteroids.

	// the human player
	create_human_ship(0.6, 0.5, "yehte");

	// create some enemy ships (Phedar) :
	create_enemies("phepa", 10);

	// and some friendly Hydro+Jyglar ships.
	create_allies("hydcr", 2);
	create_allies("jygst", 3);
}


int mission_protect_official06::P::quit_condition()
{
	
	if (playercontrol->ship->crew == 0)
	{
		return 1;		// the player is dead !! loser ;)
	}

	if (enemycount() == 0)
	{
		return 2;		// you win.
	}

	return 0;
}


void mission_protect_official06::P::calculate()
{


	switch (event_counter)
	{
	case 0:
		if (minutes < 0.1) break;
		show_message("YEEP! Our friendly Hydrovar are in trouble! Ye must help.", 6.0);
		++event_counter;
		break;
	case 1:
		if (enemycount() > 5) break;
		// the enemies are losing - they desperately attack the player ?!
		show_message("WEEP! Ye got their attention!", 6.0);
		++event_counter;
		break;

	case 2:
		set_targets(team_badguys, playercontrol->ship);
		break;
	}

	// this should come last, So that you can intercept the controls.
	Play::calculate();

}




// ****************************************************
//                mission 7
// ****************************************************




mission_protect_official07::B::B()
{
	define_title("Protect the Hydrovar homeworld.");

	define_briefing(
		"The Phedar have recently invaded the territories of the Hydrovar and Jyglar. "
		"The Yehat diplomat turns out to be a spy who wanted to sell vital information "
		"to the Phedar, about a secret alliance that had been forged between the Yehat. "
		"and the Hydrovar. "
		"The terrorist was a special agent, who was sent to prevent this.\n"
		"Wait, incoming message: BRAAK! the Hydrovar homeworld is under attack! "
		"Yer assistence is required."
		);
}



void mission_protect_official07::P::init(Log *_log)
{

	Play::init(_log);

	size = Vector2(5000,5000);
	prepare();

	create_planet(0.5, 0.5, 2 );

	// the human player
	create_human_ship(0.6, 0.5, "yehte");

	// create some enemy ships (jyglar) :
	create_enemies("phepa", 20);

	create_allies("hydcr", 8);
	create_allies("jygst", 8);
}


int mission_protect_official07::P::quit_condition()
{
	
	if (playercontrol->ship->crew == 0)
	{
		return 1;		// the player is dead !! loser ;)
	}

	if (enemycount() <= 5 && event_counter > 1)
	{
		show_message("Cease fire! The enemy wants to negotiate.", 6.0);
		merge_teams(team_goodguys, team_badguys);	// all become good guys.
		return 2;		// you "win".
	}

	return 0;
}


void mission_protect_official07::P::calculate()
{


	switch (event_counter)
	{
	case 0:
		if (minutes < 0.1) break;
		show_message("They are overwhelming us! Make each shot count!", 6.0);
		++event_counter;
		break;

	case 1:
		// Yehat reinforcements warp in.
		if (minutes < 0.4) break;
		show_message("BWAAK! We come to help ye! Kill the Phedar!", 6.0);
		create_allies("yehte", 10);
		++event_counter;
		break;

	}


	// this should come last, So that you can intercept the controls.
	Play::calculate();

}


// ****************************************************
//                mission 8
// ****************************************************




mission_protect_official08::B::B()
{
	define_title("Party time.");

	define_briefing("Fights have stopped, but the Phedar fleet commander does not yet surrender. "
		"He has challenged the most dangerous one of our fleet to a duel to the death, "
		"and that champion is ye! Ye better win this one, otherwise they will continue to fight to the last Phedar, "
		"at the cost of many more lives."
		);
}



void mission_protect_official08::P::init(Log *_log)
{

	Play::init(_log);

	size = Vector2(5000,5000);
	prepare();

	create_planet(0.5, 0.5, 2 );

	// the human player
	create_human_ship(0.6, 0.5, "yehte");

	// create some enemy ships (jyglar) :
	create_enemies("phepa", 1);
}


int mission_protect_official08::P::quit_condition()
{
	
	if (playercontrol->ship->crew == 0)
	{
		return 1;		// the player is dead !! loser ;)
	}

	if (enemycount() == 0)
	{
		return 2;		// you win.
	}

	return 0;
}


void mission_protect_official08::P::calculate()
{

	// this should come last, So that you can intercept the controls.
	Play::calculate();

}


// ****************************************************
//                mission 9
// ****************************************************




mission_protect_official09::B::B()
{
	define_title("Home sweet home.");

	define_briefing("After a relaxed flight through hyperspace, very welcome after several days, "
		"of intense celebrations, you finally reach home, where you'll be waiting a long "
		"holiday ...");
}



void mission_protect_official09::P::init(Log *_log)
{

	Play::init(_log);

	size = Vector2(5000,5000);
	prepare();

	create_planet(0.5, 0.5, 2 );

	// the human player
	create_human_ship(0.6, 0.5, "yehte");

	// create some enemy ships :
	create_enemies("phepa", 5);
}


int mission_protect_official09::P::quit_condition()
{
	
	if (playercontrol->ship->crew == 0)
	{
		return 1;		// the player is dead !! loser ;)
	}

	if (enemycount() == 0)
	{
		return 2;		// you win.
	}

	return 0;
}


void mission_protect_official09::P::calculate()
{


	switch (event_counter)
	{
	case 0:
		if (minutes < 0.1) break;
		show_message("YIP! Don't they ever stop? Teach them!", 6.0);
		++event_counter;
		break;
	case 1:
		if (minutes < 0.5) break;
		show_message("BRAAK! Home at last!", 6.0);
		create_allies("yehte", 5);	// others from the fleet are also returning.
		++event_counter;
		break;
	}


	// this should come last, So that you can intercept the controls.
	Play::calculate();

}


SpaceObject *find_closest_team(SpaceLocation *s, int test_attr, double R, TeamCode team)
{
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
		if (  ( o->attributes & test_attr) != 0 &&
			  o->get_team() == team &&
			  o != s)
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



REGISTER_GAME(gmissions, "Geomans Campaign")

