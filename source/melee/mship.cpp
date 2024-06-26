/* $Id$ */ 
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


/*------------------------------*
 *		Ship Class Registration *
 *------------------------------*/

int num_shipclasses = 0;
ShipClass *shipclasses[max_shipclasses];

void register_shipclass (
	const char *name, 
	const char *source_name, 
	Ship *(*func)(Vector2 pos, double a, ShipData *data, unsigned int code)
) 
{
	num_shipclasses += 1;
	if (num_shipclasses >= max_shipclasses)
		throw("Too many ship classes ; increase the array size...");

	//shipclasses = (ShipClass*)realloc(shipclasses, num_shipclasses * sizeof(ShipClass));

	int i = num_shipclasses-1;
	shipclasses[i] = new ShipClass();
	shipclasses[i]->link_order = i;
	shipclasses[i]->name = name;
	shipclasses[i]->source = source_name;
	shipclasses[i]->_get_ship = func;

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
		if (!strcmp(shipclasses[i]->name, name)) return shipclasses[i];
	}
	return NULL;
}


/*------------------------------*
 *		Ship Type Registration  *
 *------------------------------*/


int num_shiptypes = 0;
ShipType *shiptypes = NULL;


Ship *ShipType::get_ship(Vector2 pos, double angle, unsigned int team) {
	set_config_file(file);
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
		if (strstr(shipclasses[i]->source, buffy)) return shipclasses[i]->name;
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
	if (!tmp || (tmp - buffy < 5)) throw("bad ship file name (%s)", file);
	*tmp = 0;
	shiptypes[i].id = strdup(tmp - 5);

	int &ori = shiptypes[i].origin;
	ori = SHIP_ORIGIN_NONE;
	char *origname = (char*)get_config_string("Info", "Origin", NULL);
    strupr(origname);
	if ((strcmp(origname, "SC1") == 0) || (strcmp(origname, "SC2") == 0) || (strcmp(origname, "SC3") == 0))
		ori = SHIP_ORIGIN_SC;

	if ( (strcmp(origname, "TWA") == 0) || (strcmp(origname, "TW") == 0) || (strcmp(origname, "TWB") == 0))
		ori = SHIP_ORIGIN_TW_ALPHA;

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
			throw("init_ships - error initializing name (%s)", file);
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
	else
		shiptypes[i].data = shipdata(data);

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

	if (!shiptypes[i].code)
	{
		throw("A ship without code? No way! [%s]", code);
	}
	
	if (!shiptypes[i].data || !shiptypes[i].code) {
		if (!data) data = "none";
		if (!code) code = "none";
		char buffy[2048];
		char *tmp = buffy;
		tmp += sprintf(tmp, "Ship registration failed\n%s (%s)\n",
			shiptypes[i].name, shiptypes[i].file);
		if (!shiptypes[i].data) tmp += sprintf(tmp, "Data not found (%s)\n", data);
		if (!shiptypes[i].code) tmp += sprintf(tmp, "Code not found (%s)\n", code);
		throw("%s", buffy);
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

	// you can only recurse into a directory
	if (!(attrib & FA_DIREC))
		return;

	// do not go to directories containing a .
	if (strstr(fn, "."))
		return;

	// do not go to subdirectories that start with shp, because those are reserved for ship-data
	if (strncmp(fn, "shp", 3) == 0)
		return;

	if (param > 0) {
		sprintf(buffy, "%s/*", fn);
		for_each_file ( buffy, FA_DIREC|FA_RDONLY|FA_ARCH, _register_shiptype_dir, param-1 );
	}
	sprintf(buffy, "%s/shp*.ini", fn);
	for_each_file ( buffy, FA_ARCH|FA_RDONLY, _register_shiptype, 0 );
}

void init_ships() { 
	_register_shiptype_dir ( "ships", FA_DIREC|FA_RDONLY|FA_ARCH, 3 );

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
{ 
	attributes |= ATTRIB_SHIP;
	layer = LAYER_SHIPS;
	set_depth(DEPTH_SHIPS);
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;
	nextkeys = 0;
	id = SPACE_SHIP;

	type = 0;
	code = 0;
	// data is initialized in SpaceLocation constructor

	// modified otherwise the kat poly crashes
	if (creator && creator->isShip())
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
	first_override_control = 0;
	last_override_control = 0;
	}

Ship::Ship(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int ally_flag) :
	SpaceObject(NULL, opos, shipAngle, shipData->spriteShip),
	death_counter(-1),
	update_panel(false),
	target_pressed(false),
	control(NULL)
{ 
	attributes |= ATTRIB_SHIP;
	layer = LAYER_SHIPS;
	set_depth(DEPTH_SHIPS);

	first_override_control = 0;
	last_override_control = 0;

	type = NULL;
	code = NULL;

	captain_name[0] = '\0';
	ship = this;

	if (data)
	{
		throw("ship with data out of nowhere? Shouldn't be possible.");
	}
	data = shipData;
	if (!data)
	{
		throw("A ship without data.");
	}
	data->lock();

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
}

void Ship::death() { 
	if (attributes & ATTRIB_NOTIFY_ON_DEATH) {
		game->ship_died(this, NULL);
		attributes &= ~ATTRIB_NOTIFY_ON_DEATH;
	}
	return;
}

Ship::~Ship()
{
	 

	delete spritePanel;
	spritePanel = 0;

//f (override_control)
//delete override_control;
	if (first_override_control)
	{
		//throw("Ship deleted before all control overrides are removed...");
		// if you interrupt a game while some ship is under someones control, this can happen...

		OverrideControl *c, *cnext;
		c = first_override_control;
		while (c)
		{
			cnext = c->next;
			delete c;
			c = cnext;
		}

		first_override_control = 0;
		last_override_control = 0;
	}
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




void Ship::locate() { 
	int tries = 0;
	double mindist = 1000;
	while (tries < 15) {
		pos = tw_random(map_size);
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
{ 


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
		/*
		if (sprite->frames() > 64)
			sprite_index = get_index(angle);
		else
			sprite_index = get_index(angle, PI/2, sprite->frames());
			*/
		calculate_index();


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
		{
			state = 0; //die already
			//message.print(1500, 12, "Ship [%s] is now state=0", get_identity());
		}

		return;
	}
//added by Tau - end


	if (control && !control->exists())
		control = 0;

	if (control)
	{

		/* this test does not work in case of the teron builder
		#ifdef _DEBUG
		if (exists() && (control->ship != this) && (control->ship != ship))
		{
			throw("Control has the wrong ship...");
		}
		#endif
		*/

		// it makes most sense, to place this in FRONT of these commands here ...
		nextkeys = control->keys;

		// however, other ships may affect the controls... do it like this...
		OverrideControl *c;
		c = first_override_control;
		while (c)
		{
			c->calculate(&nextkeys);
			c = c->next;
		}

		thrust           = 1&&(nextkeys & keyflag::thrust);
		thrust_backwards = 1&&(nextkeys & keyflag::back);
		turn_left        = 1&&(nextkeys & keyflag::left);
		turn_right       = 1&&(nextkeys & keyflag::right);
		fire_weapon      = 1&&(nextkeys & keyflag::fire);
		fire_special     = 1&&(nextkeys & keyflag::special);
		fire_altweapon   = 1&&(nextkeys & keyflag::altfire);
		target_next      = 1&&(nextkeys & keyflag::next);
		target_prev      = 1&&(nextkeys & keyflag::prev);
		target_closest   = 1&&(nextkeys & keyflag::closest);

        if (nextkeys & keyflag::suicide)
		{
			handle_damage(this, 999, 999);
			/*
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
			*/
        }


        

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
	if (target_pressed && (!target_pressed_prev) && control)
	{

		if (target_next)
		{
			if (control && targets->N)
			{
				i = control->index;
				if (i < 0)
					i = 0;
				if (i > targets->N - 1)
					i = targets->N - 1;

				while (1)
				{
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
				
				if (i < 0)
					i = 0;
				if (i > targets->N - 1)
					i = targets->N - 1;


				int k = i;
				while (1)
				{
					--k;
					if ( k < 0 )
						k = targets->N - 1;

					if (k == i)
					{
						control->set_target(-1);
						break;	// no more targets..
					}

					
					//i = (i + targets->N - 1) % targets->N;
					if (control->valid_target(targets->item[i])) {
						control->set_target(i);
						break;
					}
					/*
					if (control->index != -1) {
						if (i == control->index)
							break;
					}
					else {
						if (i == 0) break;
					}
					*/
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

	/*
	// communication code
	if (  (nextkeys & keyflag::communicate) && (target->exists ()) && (target->isShip()))
	{
		Ship * s = (Ship*)target;
		if( s->ext_ai != NULL )
			s->ext_ai->Dialog(this);
	}
	*/
	

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

	
	//sprite_index = get_index(angle);
	// should be the same as earlier in this routine:
	/*
	int n = sprite->frames();
	if (n > 64)
		sprite_index = get_index(angle);
	else
		sprite_index = get_index(angle, PI/2, n);
	*/
	calculate_index();


	// hotspots are too much a luxury to include in massive games (lots of objects)
	if (hashotspots)
		calculate_hotspots();

	SpaceObject::calculate();
}

int Ship::handle_fuel_sap(SpaceLocation *source, double normal) { 

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

double Ship::handle_speed_loss(SpaceLocation *source, double normal) { 
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
			throw("Speed loss too large\n(%f)", sl);

		accel_rate *= 1 - sl * accel_rate / (accel_rate + scale_acceleration(2,4));
		hotspot_rate = (int)(hotspot_rate / (1 - sl * accel_rate / (accel_rate + scale_acceleration(2,4)) ) );
		speed_max *= 1 - sl * speed_max / (speed_max + scale_velocity(10));
		turn_rate *=  1 - sl * turn_rate / (turn_rate + scale_turning(4));
		speed_loss = 0;
	}
	return 1;
}

int Ship::handle_damage(SpaceLocation *source, double normal, double direct) { 

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
			
			// problem:
			// this resets the counter. This is reset, while the ship is still alive (dying),
			// and the control still "owns" the ship.
			// IF by some mishap, another ship died just a little earlier, and the counter has already
			// been reset ... then, this ship could still be in the dying phase while the moment
			// for choosing a new ship has passed (and it'll not be re-set either, cause this own
			// moment to reset counter has already passed...)... therefore, remove control. So that
			// choose-new-ships will know to choose a new ship.
			if (control)
			{
				control->select_ship(0, 0);
				control = 0;
			}
			// ok, end of the addition (geo).
			// ... note that originally, the death_counter check was also included somewhere, but
			// I think this is more explicit...

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

void Ship::materialize()
{
	// nothing
}

void Ship::handle_phasing()
{
	// keeps track of the default pointers (target)
	SpaceObject::calculate();
}

void Ship::assigntarget(SpaceObject *otarget)
{
	target = otarget;
}


void Ship::calculate_thrust() { 
	if (thrust)
		accelerate_gravwhip(this, angle, accel_rate * frame_time, speed_max);
	return;
}

void Ship::calculate_turn_left()
{ 
  if(turn_left)
		turn_step -= turn_rate * frame_time;
}

void Ship::calculate_turn_right()
{ 
  if(turn_right)
		turn_step += turn_rate * frame_time;
}

void Ship::calculate_fire_weapon() { 
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
{ 
  special_low = FALSE;

  if(fire_special)
  {
    if (batt < special_drain) {
      special_low = TRUE;
      return;
    }

    if(special_recharge > 0)
      return;

    if(activate_special() == 0)
      return;

    batt -= special_drain;
    special_recharge += special_rate;

    if (special_sample >= 0)
			play_sound2(data->sampleSpecial[special_sample]);
  }
}

void Ship::calculate_hotspots() { 
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


void Ship::set_target(SpaceObject *o)
{
	// both need to be set, cause control check the ship, and ship checks the control...
	if (control)
		control->target = o;

	target = o;
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
{ 
	layer = LAYER_HOTSPOTS;
	set_depth(DEPTH_HOTSPOTS);
	collide_flag_anyone = 0;
	mass = 0;

	attributes |= ATTRIB_UNDETECTABLE;

	// extra check
	// note that if this happens, there's something wrong in the ships' constructor...
	if (sprite_index < 0 || sprite_index >= sprite->frames())
		sprite_index = 0;

	// overwrite this... cause it's not really defined...
	target = 0;

	count_delay_iterations = 0;

	return;
}

void Phaser::animate(Frame *space)
{
	sprite->animate_character(pos, 
		sprite_index, pallete_color[colors[color_index]], space);
	return;
}

void Phaser::calculate()
{
	 

	++count_delay_iterations;
	if (count_delay_iterations >= DEATH_FRAMES-3)	// I'm not sure, how many you need.
	{
		// this is needed, because sometimes ships have components which are created on the spot...
		// these are vulnerable, and can die. Then, ship-specific pointers get invalid (objects
		// are removed from memory). Thus, we need need (hackish) some time before objects are removed.
		// notice that this can be avoided by the ... "materialize" function.
		throw("Phasing takes too long; need more time for pointer-checks!");
	}

	if (!exists())
		return;

	/* no... cause for the pkunk, you also need "empty" phasers...
	if (!(ship && ship->exists()))
	{
		ship = 0;
		state = 0;
		return;
	}
	*/

	frame_step -= frame_time;

	while (frame_step < 0) {
		frame_step += frame_size;
		color_index++;
		if (color_index == num_colors)
			state = 0;
	}

	if (exists() && (color_index < 0 || color_index >= num_colors))
	{
		throw("phaser index mismatch: should not occur");
	}

	if (phaser_step_position < phaser_step_size) {
		if (ship && !ship->exists())
			ship = NULL;
		phaser_step_position += frame_time;
		if (phaser_step_position >= phaser_step_size) {
			if (phaser_steps > 1) {
				Vector2 d = rel_pos / phaser_steps;
				game->add(new Phaser(this, pos + d, rel_pos-d, ship, sprite, sprite_index, colors, num_colors, frame_size, phaser_steps-1, phaser_step_size));
				ship = 0;
			}
			else if (ship) {
				game->add(ship);
				ship->materialize();
				ship = 0;
			}
		}
	}

	// needed to keep track of important pointers... which need to be checked each game iteration
	// to see if they are still valid or not... hmm.. in principle at least. For now, DEATH_COUNTER
	// is set to a high value so that objects aren't deleted while the ship is still phasing in.
	if (ship)
		ship->handle_phasing();

	SpaceObject::calculate();
}

SpaceLocation *Ship::get_ship_phaser() { 
	return new Phaser(this,
		pos - unit_vector(angle ) * PHASE_MAX * size.x,
		unit_vector(angle ) * PHASE_MAX * size.x,
		this, sprite, sprite_index, hot_color, HOT_COLORS,
		PHASE_DELAY, PHASE_MAX, PHASE_DELAY
	);
}



// add a control override to a ship
void OverrideControl::add(Ship *s, OverrideControl *newcontrol)
{
	newcontrol->next = 0;

	if (!s->first_override_control)
	{
		s->first_override_control = newcontrol;
		s->last_override_control = newcontrol;
	} else {
		s->last_override_control->next = newcontrol;
		s->last_override_control = newcontrol;
	}
}

// remove a control override from a ship
void OverrideControl::del(Ship *s, OverrideControl *oldcontrol)
{
	OverrideControl *c, *prev = 0;
	c = s->first_override_control;
	
	while (c && c != oldcontrol)
	{
		prev = c;
		c = c->next;
	}

	if (!c)
	{
		//throw("Error: trying to remove a control override that is not present in the list");
		// this is possible, if you interrupt a game while someones ship is under control... so, don't
		// worry about it. Enable it for debugging.
	} else {

		// remove the control from the list

		// update the head reference
		if (c == s->first_override_control)
		{
			s->first_override_control = c->next;
		}

		// update the tail reference
		if (c == s->last_override_control)
		{
			s->last_override_control = prev;
		}

		// skip the link
		if (prev)
			prev->next = c->next;
	}
}


//xxx for e.g. the TauEMP, a list of such actions would be mucho better !!
void Ship::set_override_control(OverrideControl *newcontrol)
{
	/*
	if (override_control)
		delete override_control;

	override_control = newcontrol;
	*/

	OverrideControl::add(this, newcontrol);

}

void Ship::del_override_control(OverrideControl *delthiscontrol)
{
	/*
	if (override_control == delthiscontrol)	// check this, because the control could be overridden by another one
	{
		delete override_control;
		override_control = 0;
	}
	*/

	OverrideControl::del(this, delthiscontrol);
	delete delthiscontrol;
}

bool Ship::exists()
{
	return (Presence::exists());// & (death_counter == -1);
};  //returns 0 if dead, non-zero if alive




void Ship::copy_values(Ship *owner)
{
	mass = owner->mass;

	crew = owner->crew;
	crew_max = owner->crew_max;
	batt = owner->batt;
	batt_max = owner->batt_max;
	
	recharge_rate = owner->recharge_rate;
	recharge_step = recharge_rate;
	recharge_amount = owner->recharge_amount;

	turn_rate = owner->turn_rate;
	accel_rate = owner->accel_rate;
	speed_max = owner->speed_max;

	damage_factor = 0;
	isblockingweapons = owner->isblockingweapons;
}


