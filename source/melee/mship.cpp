/*
 * Star Control - TimeWarp
 *
 * melee/mship.cpp - Melee ship module
 *
 * 19-Jun-2002
 *
 * - init_ships() modified to load ship ini files according to shp*.* file naming
 *   convention.
 * - Cosmetic code changes.
 */

#include <allegro.h>
#include <string.h>
#include <stdio.h>

#include "../melee.h"
REGISTER_FILE
#include "../id.h"
#include "../frame.h"

#include "mcbodies.h"
#include "mgame.h"
#include "mcontrol.h"
#include "mship.h"
#include "mshot.h"
#include "manim.h"
#include "mview.h"

#include "ais/ext_ai.h"


/*------------------------------*
 *		Ship Class Registration *
 *------------------------------*/

int num_shipclasses = 0;
ShipClass *shipclasses = NULL;

void register_shipclass (
	const char *name, 
	const char *source_name, 
	Ship *(*func)(Vector2 pos, double a, ShipData *data, unsigned int code)
) 
{
	num_shipclasses += 1;
	shipclasses = (ShipClass*)realloc(shipclasses, num_shipclasses * sizeof(ShipClass));
	int i = num_shipclasses-1;
	shipclasses[i].link_order = i;
	shipclasses[i].name = name;
	shipclasses[i].source = source_name;
	shipclasses[i]._get_ship = func;
	return;
}

Ship *ShipClass::get_ship(Vector2 pos, double angle, ShipData *dat, unsigned int team) {
	dat->lock();
	Ship *s = _get_ship(pos, angle, dat, team);
	dat->unlock();
	s->code = this;
	return s;
}

ShipClass *shipclass ( const char *name ) {
	int i;
	if (!name) return NULL;
	for (i = 0; i < num_shipclasses; i += 1) {
		if (!strcmp(shipclasses[i].name, name)) return &shipclasses[i];
	}
	return NULL;
}


/*------------------------------*
 *		Ship Type Registration  *
 *------------------------------*/


int num_shiptypes = 0;
ShipType *shiptypes = NULL;


Ship *ShipType::get_ship(Vector2 pos, double angle, unsigned int team) {
	game->log_file(file);
	Ship *s = code->get_ship(pos, angle, data, team);
	s->type = this;
	return s;
}

ShipType *shiptype(const char *shiptype_id) {
	int i;
	for (i = 0; i < num_shiptypes; i += 1) {
		if (!strcmp(shiptype_id, shiptypes[i].id)) {
			return &shiptypes[i];
		}
	}
	return NULL;
}

const char *old_code_name( const char *file ) {
	char buffy[2048];
	if (strlen(file) != 18) return NULL;
	strncpy(buffy, file+9, 5);
	buffy[5] = 0;
	int i;
	for (i = 0; i < num_shipclasses; i += 1) {
		if (strstr(shipclasses[i].source, buffy)) return shipclasses[i].name;
	}
	return NULL;
}

static void register_shiptype ( const char *file ) {
	num_shiptypes += 1;
	shiptypes = (ShipType*)realloc(shiptypes, num_shiptypes * sizeof(ShipType));
	int i = num_shiptypes-1;
/*	char id[2048];
	strncpy(id, file, 255);
	char *tmp = id, *tmp2 = NULL;
	while (tmp2 = strchr(tmp, 
	char *tmp = strchr(id, '/');
	if (tmp) {tmp[0] = 0;*/

	set_config_file(file);

	shiptypes[i].file = strdup(file);
	char buffy[1024];
	strncpy(buffy, file, 1000);
	char *tmp = strchr(buffy, '.');
	if (!tmp || (tmp - buffy < 5)) tw_error("bad ship file name (%s)", file);
	*tmp = 0;
	shiptypes[i].id = strdup(tmp - 5);

	int &ori = shiptypes[i].origin;
	ori = SHIP_ORIGIN_NONE;
	char *origname = (char*)get_config_string("Info", "Origin", NULL);
    strupr(origname);
	if (strcmp(origname, "SC1") == 0)
		ori = SHIP_ORIGIN_SC1;
	if (strcmp(origname, "SC2") == 0)
		ori = SHIP_ORIGIN_SC2;
	if (strcmp(origname, "SC3") == 0)
		ori = SHIP_ORIGIN_SC3;
	if ( (strcmp(origname, "TWA") == 0) ||		// alpha ships
         (strcmp(origname, "TW") == 0) )
		ori = SHIP_ORIGIN_TW_ALPHA;
	if (strcmp(origname, "TWB") == 0)		// beta ships
		ori = SHIP_ORIGIN_TW_BETA;
	if (strcmp(origname, "TWS") == 0)		// special ships
		ori = SHIP_ORIGIN_TW_SPECIAL;


	const char *name = get_config_string("Info", "Name", NULL);
	if (!name) {
		const char *tmp;
		int l = 0;
		tmp = get_config_string("Info", "Name0", NULL);
		if (tmp) {
			l += sprintf(buffy + l, "%s ", tmp);
		}
		tmp = get_config_string("Info", "Name1", NULL);
		if (!tmp)
			tw_error("init_ships - error initializing name (%s)", file);
		l += sprintf(buffy + l, "%s", tmp);
		int n = 1;
		while (true) {
			char buf[25];
			n += 1;
			sprintf(buf, "Name%d", n);
			tmp = get_config_string("Info", buf, NULL);
			if (!tmp) break;
			l += sprintf(buffy + l, " %s", tmp);
		}
		name = buffy;
	}
	shiptypes[i].name = strdup(name);
	shiptypes[i].cost = get_config_int("Info", "TWCost", 0);
	const char *data = get_config_string("Info", "Data", NULL);
	if (!data) {
		replace_extension(buffy, file, "dat", 2040);
		shiptypes[i].data = shipdata(buffy);
	}
	else shiptypes[i].data = shipdata(data);
	const char *text = get_config_string("Info", "Description", NULL);
	if (!text) {
		char duck[2048];
		replace_extension(duck, file, "txt", 2040);
		//if (exists(duck)) 
			shiptypes[i].text = strdup(duck);
		//else shiptypes[i].text = NULL;
	}
	else shiptypes[i].text = strdup(text);
	const char *code = get_config_string("Info", "Code", NULL);
	int old = 0;
	if (!code) {
		code = old_code_name(file);
		old = 1;
	}
	shiptypes[i].code = shipclass(code);
	if (!shiptypes[i].data || !shiptypes[i].code) {
		if (!data) data = "none";
		if (!code) code = "none";
		char buffy[2048];
		char *tmp = buffy;
		tmp += sprintf(tmp, "Ship registration failed\n%s (%s)\n",
			shiptypes[i].name, shiptypes[i].file);
		if (!shiptypes[i].data) tmp += sprintf(tmp, "Data not found (%s)\n", data);
		if (!shiptypes[i].code) tmp += sprintf(tmp, "Code not found (%s)\n", code);
		error("%s", buffy);
		num_shiptypes -= 1;
		return;
	}
	if (old) set_config_string("Info", "Code", code);

	return;
	
}

void _register_shiptype ( const char *fn, int attrib, int param ) {
	register_shiptype(fn);
}

void _register_shiptype_dir ( const char *fn, int attrib, int param ) {
	char buffy[2048];
	if (!(attrib & FA_DIREC)) return;
	if (strstr(fn, ".")) return;
	if (param > 0) {
		sprintf(buffy, "%s/*", fn);
		for_each_file ( buffy, FA_DIREC|FA_RDONLY, _register_shiptype_dir, param-1 );
	}
	sprintf(buffy, "%s/shp*.ini", fn);
	for_each_file ( buffy, FA_ARCH|FA_RDONLY, _register_shiptype, 0 );
}

void init_ships() {STACKTRACE
	_register_shiptype_dir ( "ships", FA_DIREC|FA_RDONLY, 3 );

/*	shiptype_type tmp;
	int i = _num_shiptypes;
	int num_bad = 0;

	num_shiptypes = i;
	while (i > 0) {
		int bad = false;
		i -= 1;

		//check ini file exists
		char ini_name[80];
		char dat_name[80];
		sprintf(fname, "ships/shp%s.ini", shiptypes[i].id);
		if (!exists(ini_name)) bad = true;

		sprintf(dat_name, "ships/shp%s.dat", shiptypes[i].id);
		if (!exists(dat_name)) bad = true;
		
		set_config_file(ini_name);
		const char *tmp = get_config_string("Info", "Data", NULL);
		if (!tmp) tmp = ini_name;
		if (!exists(tmp)) bad = true;
		shiptypes[i].cost = get_config_int("Info", "TWCost", 999);
		shiptypes[i].name = get_config_string("Info", "Name", NULL);
		if (!shiptypes[i].name) {
			char sname[1024];
			const char *tmp;
			int l = 0;
			tmp = get_config_string("Info", "Name0", NULL);
			if (tmp) {
				l += sprintf(sname + l, "%s ", tmp);
			}
			tmp = get_config_string("Info", "Name1", NULL);
			if (!tmp)
				tw_error("init_ships - error initializing name (%s)", shiptypes[i].id);
			l += sprintf(sname + l, "%s", tmp);
			int n = 1;
			while (true) {
				char buf[25];
				n += 1;
				sprintf(buf, "Name%d", n);
				tmp = get_config_string("Info", buf, NULL);
				if (!tmp) break;
				l += sprintf(sname + l, " %s", tmp);
			}
			shiptypes[i].name = strdup(sname);
		}
		else shiptypes[i].name = strdup(shiptypes[i].name);
	}
			tmp = shiptypes[i];
			shiptypes[i] = shiptypes[num_shiptypes-1];
			memmove(&shiptypes[i], &shiptypes[num_shiptypes-1], sizeof(shiptype_type));
			num_shiptypes -= 1;

	if (num_shiptypes == 0)
		tw_error("No ship data available.\nProbably caused to an install problem.\nConsult readme.txt");
*/
	return;
}



int hot_color[HOT_COLORS] =
{
  122, 123, 124, 125, 126, 127,
  42,  43,  44,  45,  46,  47
};


Ship::Ship(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite) :
	SpaceObject(creator, opos, oangle, osprite),
	death_counter(-1),
	update_panel(false),
	target_pressed(false),
	control(NULL)
{STACKTRACE
	attributes |= ATTRIB_SHIP;
	layer = LAYER_SHIPS;
	set_depth(DEPTH_SHIPS);
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;
	nextkeys = 0;
	id = SPACE_SHIP;

	type = NULL;
	code = NULL;

	// modified otherwise the kat poly crashes
	if (creator->isShip())
		type = ((Ship*) creator)->type;

	captain_name[0] = '\0';
	thrust           = FALSE;
	turn_left        = FALSE;
	turn_right       = FALSE;
	thrust_backwards = FALSE;
	fire_weapon      = FALSE;
	fire_special     = FALSE;
	fire_altweapon   = FALSE;

	spritePanel = NULL;

	turn_step = 0;
	angle = floor(oangle / (PI2/64)) * (PI2/64);
	sprite_index = get_index(angle);

	hashotspots = true;
	ext_ai = NULL;
}

Ship::Ship(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int ally_flag) :
	SpaceObject(NULL, opos, shipAngle, shipData->spriteShip),
	death_counter(-1),
	update_panel(false),
	target_pressed(false),
	control(NULL)
{STACKTRACE
	shipData->lock();
	attributes |= ATTRIB_SHIP;
	layer = LAYER_SHIPS;
	set_depth(DEPTH_SHIPS);

	type = NULL;
	code = NULL;

	captain_name[0] = '\0';
	ship = this;
	data = shipData;
	this->ally_flag = ally_flag;

	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;
	nextkeys = 0;

	id = SPACE_SHIP;

	int i;
	i = get_config_int("Names", "NumNames", 0);
	int L = sizeof(captain_name);
	if (i) {
		char buffy[16];
		sprintf(buffy, "CaptName%d", 1+(rand() % i));
		const char *tmp = get_config_string("Names", buffy, "");
		i = strlen(tmp);
		strncpy(captain_name, tmp, L);
		if (i >= L) i = L-1;
		captain_name[i] = '\0';
	}
	else captain_name[0] = '\0';

	//  set_config_data((char *)(data->data[0].dat), data->data[0].size);
	crew     = get_config_int("Ship", "Crew", 0);
	crew_max = get_config_int("Ship", "CrewMax", 0);
	batt     = get_config_int("Ship", "Batt", 0);
	batt_max = get_config_int("Ship", "BattMax", 0);

	recharge_amount  = get_config_int("Ship", "RechargeAmount", 0);
	recharge_rate    = scale_frames(get_config_float("Ship", "RechargeRate", 0));
	recharge_step    = recharge_rate;
	weapon_drain     = get_config_int("Ship", "WeaponDrain", 0);
	weapon_rate      = scale_frames(get_config_float("Ship", "WeaponRate", 0));
	weapon_sample    = 0;
	weapon_recharge  = 0;
	weapon_low       = FALSE;
	special_drain    = get_config_int("Ship", "SpecialDrain", 0);
	special_rate     = scale_frames(get_config_float("Ship", "SpecialRate", 0));
	special_sample   = 0;
	special_recharge = 0;
	special_low      = FALSE;

	double raw_hotspot_rate = get_config_float("Ship", "HotspotRate", 0);
	hotspot_rate  = scale_frames(raw_hotspot_rate);
	hotspot_frame = 0;
	turn_rate     = scale_turning(get_config_float("Ship", "TurnRate", 0));
	turn_step     = 0.0;
	speed_max     = scale_velocity(get_config_float("Ship", "SpeedMax", 0));
	accel_rate    = scale_acceleration(get_config_float("Ship", "AccelRate", 0), raw_hotspot_rate);
	mass          = (get_config_float("Ship", "Mass", 0));

	thrust           = FALSE;
	turn_left        = FALSE;
	turn_right       = FALSE;
	thrust_backwards = FALSE;
	fire_weapon      = FALSE;
	fire_special     = FALSE;
	fire_altweapon   = FALSE;

	spritePanel  = new SpaceSprite(*(data->spritePanel));
	if (captain_name[0]) {
		spritePanel->lock();
		text_mode(-1);
		textprintf_centre(
			spritePanel->get_bitmap(0), 
			videosystem.get_font(1), 
			30, 51, 
			pallete_color[0], 
			captain_name
		);
		spritePanel->unlock();
	}

	angle = floor(shipAngle / (PI2/64)) * (PI2/64);
	sprite_index = get_index(angle);

	hashotspots = true;
	ext_ai = NULL;
}

void Ship::death() {STACKTRACE
	if (attributes & ATTRIB_NOTIFY_ON_DEATH) {
		game->ship_died(this, NULL);
		attributes &= ~ATTRIB_NOTIFY_ON_DEATH;
	}
	return;
}

Ship::~Ship() {STACKTRACE
	delete spritePanel;
	destroy_external_ai();
}

double Ship::getCrew()
{
  return(crew);
}

double Ship::getBatt()
{
  return(batt);
}

RGB Ship::crewPanelColor(int k)
{
	RGB c = {0,225,0};
	return c;
}

RGB Ship::battPanelColor(int k)
{
	RGB c = {225,0,0};
	return c;
}

void Ship::locate() {STACKTRACE
	int tries = 0;
	double mindist = 1000;
	while (tries < 15) {
		pos = random(map_size);
		SpaceLocation *spacePlanet = nearest_planet();
		if (!spacePlanet || (distance(spacePlanet) > mindist))
			break;
		if (tries < 10)
			mindist *= 0.9;
		else
			mindist *= 0.5;
	}
	return;
}

void Ship::calculate()
{STACKTRACE

//added by Tau - start
	if (exists() && death_counter >= 0) {
		while(fabs(turn_step) > (PI2/64) / 2) {
			if(turn_step < 0.0) {
				angle -= (PI2/64);
				turn_step += (PI2/64); }
			else
				if(turn_step > 0.0) {
					angle += (PI2/64);
					turn_step -= (PI2/64); }
			if(angle < 0.0)
				angle += PI2;
			if(angle >= PI2)
				angle -= PI2;
		}

		// changed GEO - just to be sure you don't exceed #frames if #frames<64 (the assumed value here...).
		// sprite_index = get_index(angle);
		//sprite_index = get_index(angle, PI/2, sprite->frames());
		// (geo) actually, this introduces a big bug if the ship has >64 frames in
		// the ship sprite -- eg the tau mercury.
		if (sprite->frames() > 64)
			sprite_index = get_index(angle);
		else
			sprite_index = get_index(angle, PI/2, sprite->frames());


		SpaceObject::calculate();

		Animation *a;
		int i, ff;
		double vv = magnitude(vel);

		death_explosion_counter -= frame_time;
		while (death_explosion_counter <= 0) {
			death_explosion_counter += 25;
			for (i=0; i<2; i++) {
				ff = random(25);
				a = new Animation(this, pos, meleedata.xpl1Sprite,
					ff, 40-ff, 25, DEPTH_EXPLOSIONS);
				a->transparency = 1.0 / 4;
				game->add(a);
				a->accelerate(this, vel.angle(), vv, MAX_SPEED);
				a->accelerate(this, random(PI2),
					random(1.0)*scale_velocity(25)*sqrt((15+mass)/35),
					MAX_SPEED);
				a->collide_flag_anyone = a->collide_flag_sameship = a->collide_flag_sameteam = 0;
			}
		}

		death_counter += frame_time;
		if (death_counter > 700 * (15+mass)/35 || death_counter > 3000) //smaller ships will make smaller explosions ; GEO: but a real upper limit is also good to have
			state = 0; //die already

		return;
	}
//added by Tau - end

	if (control) {
		this->thrust           = 1&&(nextkeys & keyflag::thrust);
		this->thrust_backwards = 1&&(nextkeys & keyflag::back);
		this->turn_left        = 1&&(nextkeys & keyflag::left);
		this->turn_right       = 1&&(nextkeys & keyflag::right);
		this->fire_weapon      = 1&&(nextkeys & keyflag::fire);
		this->fire_special     = 1&&(nextkeys & keyflag::special);
		this->fire_altweapon   = 1&&(nextkeys & keyflag::altfire);
		this->target_next      = 1&&(nextkeys & keyflag::next);
		this->target_prev      = 1&&(nextkeys & keyflag::prev);
		this->target_closest   = 1&&(nextkeys & keyflag::closest);

        if (nextkeys & keyflag::suicide) {
            crew  = 0;
            play_sound((SAMPLE *)(melee[MELEE_BOOMSHIP].dat));

            if (meleedata.xpl1Sprite) {
                death_counter = 0;
                death_explosion_counter = 0;
                collide_flag_anyone = collide_flag_sameship = collide_flag_sameteam = 0;
            }
            else {
                state = 0;
                game->add(new Animation(this, pos, meleedata.kaboomSprite, 0, KABOOM_FRAMES, time_ratio, DEPTH_EXPLOSIONS));
            }
            if (attributes & ATTRIB_NOTIFY_ON_DEATH) {
                game->ship_died(this, NULL);
                attributes &= ~ATTRIB_NOTIFY_ON_DEATH;
            }
        }
		this->nextkeys = control->keys;


        

		if (!control->exists()) control = NULL;
	}

	if(batt < batt_max) {
		recharge_step -= frame_time;
		while(recharge_step < 0 && recharge_rate > 0)	// this loop never ends if there's no recharge
		{
			batt += recharge_amount;
			if (batt > batt_max) batt = batt_max;
			recharge_step += recharge_rate;
		}
	}

	if(weapon_recharge > 0)
		weapon_recharge -= frame_time;
	if(special_recharge > 0)
		special_recharge -=  frame_time;

	int target_pressed_prev = target_pressed;
	target_pressed = target_next || target_prev || target_closest;

	int i;
	if (target_pressed && (!target_pressed_prev) && control) {
		if (target_next) {
			if (control && targets->N) {
				i = control->index;
				if (i < 0) i = 0;
				while (1) {
					i = (i + 1) % targets->N;
					if (control->valid_target(targets->item[i])) {
						control->set_target(i);
						break;
					}
					if (control->index != -1) {
						if (i == control->index) break;
					}
					else {
						if (i == 0) break;
					}
				}
			}				
		}
		else if (target_prev) {
			if (control && targets->N) {
				i = control->index;
				if (i < 0) i = 0;
				while (1) {
					i = (i + targets->N - 1) % targets->N;
					if (control->valid_target(targets->item[i])) {
						control->set_target(i);
						break;
					}
					if (control->index != -1) {
						if (i == control->index)
							break;
					}
					else {
						if (i == 0) break;
					}
				}
			}				
		}
		else if (target_closest) {
			if (control && targets->N) {
				int i, j = -1;
				double r = 99999;
				double d;
				for (i = 0; i < targets->N; i += 1) {
					if (control->valid_target(targets->item[i])) {
						d = distance(targets->item[i]);
						if (d < r) {
							r = d;
							j = i;
						}
					}
				}
				control->set_target(j);
			}
		}
	}

	target_pressed = target_next | target_prev | target_closest;
	if (control)
		target = control->target;

	// communication code
	if (  (nextkeys & keyflag::communicate) && (target->exists ()) && (target->isShip()))
	{
		Ship * s = (Ship*)target;
		if( s->ext_ai != NULL )
			s->ext_ai->Dialog(this);
	}
	

	calculate_turn_left();
	calculate_turn_right();
	calculate_thrust();
	calculate_fire_weapon();
	calculate_fire_special();


	while(fabs(turn_step) > (PI2/64) / 2) {
		if(turn_step < 0.0) {
			angle -= (PI2/64);
			turn_step += (PI2/64);
		} else
			if(turn_step > 0.0) {
				angle += (PI2/64);
				turn_step -= (PI2/64);
			}

		if(angle < 0.0) angle += PI2;
		if(angle >= PI2) angle -= PI2;
	}

	sprite_index = get_index(angle);

	// hotspots are too much a luxury to include in massive games (lots of objects)
	if (hashotspots)
		calculate_hotspots();

	SpaceObject::calculate();
}

int Ship::handle_fuel_sap(SpaceLocation *source, double normal) {STACKTRACE

	if (death_counter >= 0) return 0; //added by Tau

	batt -= normal;

	if(batt < 0) {
		normal += batt;
		batt = 0;
	}

	if(batt > batt_max) {
		normal += batt_max - batt;
		batt = batt_max;
	}

	return 1;
}

double Ship::handle_speed_loss(SpaceLocation *source, double normal) {STACKTRACE
	double speed_loss = normal;
	if(speed_loss > 0.0) {

/*		double oh = hotspot_rate;
		hotspot_rate += speed_loss;
		accel_rate *= oh / hotspot_rate;
		speed_max *= oh / hotspot_rate;
		turn_rate = 1. / (1. / turn_rate + 1. / scale_turning(1));
		speed_loss = 0.0;
//should be revised*/

		double sl = (30/(mass+30)) * speed_loss;
		if (sl > 1)
			tw_error("Speed loss too large\n(%f)", sl);

		accel_rate *= 1 - sl * accel_rate / (accel_rate + scale_acceleration(2,4));
		hotspot_rate = (int)(hotspot_rate / (1 - sl * accel_rate / (accel_rate + scale_acceleration(2,4)) ) );
		speed_max *= 1 - sl * speed_max / (speed_max + scale_velocity(10));
		turn_rate *=  1 - sl * turn_rate / (turn_rate + scale_turning(4));
		speed_loss = 0;
	}
	return 1;
}

int Ship::handle_damage(SpaceLocation *source, double normal, double direct) {STACKTRACE

	if (death_counter >= 0) return 0; //added by Tau

	double total = normal + direct;

	crew -= total;
	if(crew > crew_max) {
		total += crew_max - crew;
		crew = crew_max;
	}

	if(crew <= 0) {
		total += crew;
		crew  = 0;
		play_sound((SAMPLE *)(melee[MELEE_BOOMSHIP].dat));
		//state = 0;
//modified by Tau - start
		if (meleedata.xpl1Sprite) {
			death_counter = 0;
			death_explosion_counter = 0;
			collide_flag_anyone = collide_flag_sameship = collide_flag_sameteam = 0;
		}
		else {
			state = 0;
			game->add(new Animation(this, pos, meleedata.kaboomSprite, 0, KABOOM_FRAMES, time_ratio, DEPTH_EXPLOSIONS));
		}
		if (attributes & ATTRIB_NOTIFY_ON_DEATH) {
			game->ship_died(this, source);
			attributes &= ~ATTRIB_NOTIFY_ON_DEATH;
		}
//modified by Tau - end
/*		game->add(new Animation(this, pos,	game->kaboomSprite, 0, KABOOM_FRAMES, time_ratio, DEPTH_EXPLOSIONS));
		if (attributes & ATTRIB_NOTIFY_ON_DEATH) {
			game->ship_died(this, source);
			attributes &= ~ATTRIB_NOTIFY_ON_DEATH;
		}*/
	}

	return 1;
}

void Ship::materialize() {
}


void Ship::assigntarget(SpaceObject *otarget)
{
	target = otarget;
}


void Ship::calculate_thrust() {STACKTRACE
	if (thrust)
		accelerate_gravwhip(this, angle, accel_rate * frame_time, speed_max);
	return;
}

void Ship::calculate_turn_left()
{STACKTRACE
  if(turn_left)
		turn_step -= turn_rate * frame_time;
}

void Ship::calculate_turn_right()
{STACKTRACE
  if(turn_right)
		turn_step += turn_rate * frame_time;
}

void Ship::calculate_fire_weapon() {STACKTRACE
	weapon_low = FALSE;

	if (fire_weapon) {
		if (batt < weapon_drain) {
			weapon_low = true;
			return;
		}

		if (weapon_recharge > 0)
			return;

		if (!activate_weapon())
			return;

		batt -= weapon_drain;
		if (recharge_amount > 1)
			recharge_step = recharge_rate;
		weapon_recharge += weapon_rate;

		if (weapon_sample >= 0)
			play_sound2(data->sampleWeapon[weapon_sample]);
	}
	return;
}

void Ship::calculate_fire_special()
{STACKTRACE
  special_low = FALSE;

  if(fire_special) {
    if(batt < special_drain) {
      special_low = TRUE;
      return;
    }

    if(special_recharge > 0)
      return;

    if(!activate_special())
      return;

    batt -= special_drain;
    special_recharge += special_rate;

    if (special_sample >= 0)
			play_sound2(data->sampleSpecial[special_sample]);
  }
}

void Ship::calculate_hotspots() {STACKTRACE
	if((thrust) && (hotspot_frame <= 0)) {
		game->add(new Animation(this,
			normal_pos() - unit_vector(angle) * size.x / 2.5,
			meleedata.hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, DEPTH_HOTSPOTS));
		hotspot_frame += hotspot_rate;
	}
	if (hotspot_frame > 0)
		hotspot_frame -= frame_time;
	return;
}

int Ship::activate_weapon()
{
  return(TRUE);
}

int Ship::activate_special()
{
  return(TRUE);
}

void Ship::animate(Frame *frame) {
	SpaceObject::animate(frame);
}

void Ship::animate_predict(Frame *frame, int time) {
	if (!game->lag_frames 
		|| !sprite 
		|| !control 
		|| !control->_prediction_keys) 
	{
		SpaceObject::animate_predict(frame, time);
		return;
	}

	double old_angle = angle;
	Vector2 old_velocity = vel;
	Vector2 old_position = pos;
	int old_index = sprite_index;

	double da = turn_step;

	int i, j, k;
	int base = control->_prediction_keys_index;
	int max = Control::_prediction_keys_size;
	i = game->lag_frames;
	if (i > max) i = max;
	j = i;
	while (i > 0) {
		int turn = 0;
		k = control->_prediction_keys[(base-i)&(max-1)];
		if (k & keyflag::left)  turn -= 1;
		if (k & keyflag::right) turn += 1;
		da += turn * turn_rate * frame_time;
		if (k & keyflag::thrust) {
			_accelerate(angle+da, accel_rate * frame_time, speed_max);
		}
		pos += vel * ((double)time / j);
		i -= 1;
	}

	angle = normalize(angle + da);
	pos = normalize(pos);
	sprite_index = get_index(angle);
	if (sprite_index >= sprite->frames()) sprite_index = 0;

	animate(frame);

	angle = old_angle;
	vel = old_velocity;
	pos = old_position;
	sprite_index = old_index;
}

double Ship::get_angle_ex() const
{
	return normalize(angle + turn_step, PI2);
}

ShipType *Ship::get_shiptype()
{
	return type;
}

/*! \brief add external ai script to ship 
	\param script - script file
*/
void Ship::install_external_ai(const char* script)
{
	if (ext_ai != NULL)
	{
		delete ext_ai;
	}
	if (script == NULL) 
		return;
	ext_ai = new ExternalAI(this, script );
}

/*! \brief destroy_external ai */
void Ship::destroy_external_ai()
{
	if (ext_ai != NULL)
		delete ext_ai;
	ext_ai = NULL;
}


Phaser::Phaser(
	SpaceLocation *creator, Vector2 opos, Vector2 _rpos, 
	Ship *ship, SpaceSprite *sprite, int osprite_index, int *ocolors, 
	int onum_colors, int ofsize, int steps, int step_size) :
	SpaceObject(creator, opos, 0.0, sprite),
	rel_pos(_rpos),
	ship(ship),
	sprite_index(osprite_index),
	colors(ocolors),
	num_colors(onum_colors),
	color_index(0),
	frame_size(ofsize),
	frame_step(ofsize),
	phaser_step_position(0),
	phaser_steps(steps),
	phaser_step_size(step_size)
{STACKTRACE
	layer = LAYER_HOTSPOTS;
	set_depth(DEPTH_HOTSPOTS);
	collide_flag_anyone = 0;
	mass = 0;

	attributes |= ATTRIB_UNDETECTABLE;

	// extra check
	// note that if this happens, there's something wrong in the ships' constructor...
	if (sprite_index >= sprite->frames())
		sprite_index = 0;

	return;
}

void Phaser::animate(Frame *space) {
	sprite->animate_character(pos, 
		sprite_index, pallete_color[colors[color_index]], space);
	return;
}

void Phaser::calculate() {STACKTRACE
	if (!exists())
		return;
	frame_step -= frame_time;

	while (frame_step < 0) {
		frame_step += frame_size;
		color_index++;
		if (color_index == num_colors)
			state = 0;
	}

	if (phaser_step_position < phaser_step_size) {
		if (ship && !ship->exists())
			ship = NULL;
		phaser_step_position += frame_time;
		if (phaser_step_position >= phaser_step_size) {
			if (phaser_steps > 1) {
				Vector2 d = rel_pos / phaser_steps;
				game->add(new Phaser(this, pos + d, rel_pos-d, ship, sprite, sprite_index, colors, num_colors, frame_size, phaser_steps-1, phaser_step_size));
			}
			else if (ship) {
				game->add(ship);
				ship->materialize();
				ship = NULL;
			}
		}
	}
	SpaceObject::calculate();
}

SpaceLocation *Ship::get_ship_phaser() {STACKTRACE
	return new Phaser(this,
		pos - unit_vector(angle ) * PHASE_MAX * size.x,
		unit_vector(angle ) * PHASE_MAX * size.x,
		this, sprite, sprite_index, hot_color, HOT_COLORS,
		PHASE_DELAY, PHASE_MAX, PHASE_DELAY
	);
}
