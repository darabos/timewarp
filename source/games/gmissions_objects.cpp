
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
#include "../util/aastr.h"


#include "twgui.h"

#include "gmissions_objects.h"

#include <string.h>
#include <stdio.h>




// perhaps an extra argument, namely dataobjectsubname, since dataobjectname
// point to a data-file (inside a data file).
SpaceSprite *gensprite(char *datafilename, char *dataobjectname, char *ident)
{
	STACKTRACE

	DATAFILE *d_raw, *d, *config, *bmpdata;

	d_raw = load_datafile_object(datafilename, dataobjectname);
	d = (DATAFILE*) d_raw->dat;
	// read as: this particular data part of the datafile is a datafile itself.
	// this datafile part in turn contains an info_txt part, and bitmaps/ sounds, other data?
	// (but only bmp's are used here).
				
	// a config statement should show how many sprites there are (or need to be, rotated)
	
	config = find_datafile_object(d, "INFO_TXT");
	if (!config)
	{
		tw_error("could not find object/ship info for the mission sprite");
	}
	set_config_data((char*) config->dat, config->size);

	int Nbmp, Nrot;
	Nbmp = get_config_int(ident, "Nbmp", 1);
	Nrot = get_config_int(ident, "Nrot", 1);

	char ident2[512];
	strcpy(ident2, ident);
	strcat(ident2, "01_BMP");	// the first (in a sequence)
	bmpdata = find_datafile_object(d, ident2);

	// I'm assuming here it contains only 1 image (should be changed!)
	return new SpaceSprite(bmpdata, Nbmp, SpaceSprite::MASKED | SpaceSprite::MIPMAPED, Nrot);

	unload_datafile_object(d_raw);
	// this isn't efficient, but it works...
}



MissionObject::MissionObject(Vector2 opos, char *datafilename, char *dataobjectname)
:
SpaceObject(0, opos, random(PI2),
		gensprite(datafilename, dataobjectname, "object")
		)
{
	set_depth(DEPTH_SPECIAL);
	
	layer = LAYER_SPECIAL;	// important that this is specified, otherwise it cannot collide.
}






MissionShip::MissionShip(Vector2 opos, char *datafilename, char *dataobjectname,
		char *ininame, char *sectionname, TeamCode team)
:
Ship(0, opos, random(PI2),
		gensprite(datafilename, dataobjectname, "ship")
		)
{

	//  set_config_data((char *)(data->data[0].dat), data->data[0].size);

	set_config_file(ininame);

	crew     = get_config_int(sectionname, "Crew", 0);
	crew_max = get_config_int(sectionname, "CrewMax", 0);
	batt     = get_config_int(sectionname, "Batt", 0);
	batt_max = get_config_int(sectionname, "BattMax", 0);

	recharge_amount  = get_config_int(sectionname, "RechargeAmount", 0);
	recharge_rate    = scale_frames(get_config_float(sectionname, "RechargeRate", 0));
	recharge_step    = recharge_rate;
	weapon_drain     = get_config_int(sectionname, "WeaponDrain", 0);
	weapon_rate      = scale_frames(get_config_float(sectionname, "WeaponRate", 0));
	weapon_sample    = 0;
	weapon_recharge  = 0;
	weapon_low       = FALSE;
	special_drain    = get_config_int(sectionname, "SpecialDrain", 0);
	special_rate     = scale_frames(get_config_float(sectionname, "SpecialRate", 0));
	special_sample   = 0;
	special_recharge = 0;
	special_low      = FALSE;

	double raw_hotspot_rate = get_config_float(sectionname, "HotspotRate", 0);
	hotspot_rate  = scale_frames(raw_hotspot_rate);
	hotspot_frame = 0;
	turn_rate     = scale_turning(get_config_float(sectionname, "TurnRate", 0));
	turn_step     = 0.0;
	speed_max     = scale_velocity(get_config_float(sectionname, "SpeedMax", 0));
	accel_rate    = scale_acceleration(get_config_float(sectionname, "AccelRate", 0), raw_hotspot_rate);
	mass          = (get_config_float(sectionname, "Mass", 0));


	weapon_sample = -1;		// no sounds to play (until said otherwise elsewhere)
	special_sample = -1;

	ally_flag |= team << team_shift;
}

/*
SpaceSprite *MissionShip::gensprite(char *datafilename, char *dataobjectname)
{
	DATAFILE *d;

	// here should be analysis of a text file to see how many sprites there are ...

	// d should be a data file itself, containing the ship bitmaps
	d = (DATAFILE*) load_datafile_object(datafilename, dataobjectname)->dat;
				
	// I'm assuming here it contains only 1 image (should be changed!)
	return new SpaceSprite(&d[0], 1, SpaceSprite::MASKED | SpaceSprite::MIPMAPED, 64);

	unload_datafile_object(d);
}
*/



void MissionShip::calculate()
{
	STACKTRACE

	Ship::calculate();

	missioncontrol();
}


void MissionShip::missioncontrol()
{
	// for each ship, you could place something here, depending on the particular mission
	// and their particular properties.
}








SpaceLineVoid::SpaceLineVoid(SpaceLocation *creator, Vector2 lpos, double langle, 
									double llength, int lcolor)
:
SpaceLine(creator, lpos, langle, llength, lcolor)
{
}


void SpaceLineVoid::setline(Vector2 p1, Vector2 p2)
{
	STACKTRACE

	Vector2 seg;
	
	seg = p2 - p1;
	
	angle = seg.atan();
	length = seg.length();
}




SpecialArea::SpecialArea(Vector2 apos, double aR)
:
SpaceLocation(0, apos, 0)
{
	R = aR;
	rping = 0;

	center = apos;

	int i;
	for ( i = 0; i < 10; ++i )
	{
		line[i] = new SpaceLineVoid(this, center, 0, 1, makecol(0,128,0));
		line[i]->vel = 0;
		game->add(line[i]);
	}

}

bool SpecialArea::inreach(SpaceLocation *s)
{
	STACKTRACE

	if (distance(s) <= R)
		return true;
	else
		return false;
}


void SpecialArea::animate(Frame *space)
{
	STACKTRACE

	// create a ring of line segments, growing outward in 1 second:
	rping += R * frame_time * 1E-3;
	if (rping > R)
		rping -= R;

	int i, k;

	for ( i = 0; i < 10; ++i )
	{
		double a;
		a = i * PI2 / 10;
		line[i]->pos = center + rping * unit_vector(a);
	}

	for ( i = 0; i < 10; ++i )
	{
		k = i + 1;
		if (k >= 10)
			k -= 10;

		line[i]->setline(line[i]->pos, line[k]->pos);
	}

}




AsteroidBelt::AsteroidBelt(Vector2 ocenter, double oRmin, double oRmax, double ov)
:
Asteroid()
{
	Rmin = oRmin;
	Rmax = oRmax;
	center = ocenter;

	double a;

	a = random(PI2);
	R = random(Rmin, Rmax);
	v_unscaled = ov;
	v = scale_velocity(ov);

	vel = v * unit_vector(a+0.5*PI);

	pos = center + R * unit_vector(a);

	double x;
	x = v * frame_time;	// the distance you'd like to travel.

	double c;
	a = asin(x / (2 * R));

	c = vel.atan();

	vel += v * 2*sin(0.5*a) * unit_vector(c + 0.5*a + 0.5*PI);
	// correction, so that it points to the next point on the circle

	pos += vel * frame_time;
}



void AsteroidBelt::death()
{
	STACKTRACE


	Animation *a = new Animation(this, pos,
			explosion, 0, explosion->frames(), time_ratio, get_depth());
	a->match_velocity(this);
	game->add(a);
	
	game->add(new AsteroidBelt(center, Rmin, Rmax, v_unscaled));
	return;
}


Vector2 AsteroidBelt::correction()
{
	STACKTRACE

	if (frame_time == 0)
		return 0;

	// correction value for a distance x ?
	double x;
	x = v * frame_time;	// the distance you'd like to travel.

	double a, c;
	// half the angle change over 1 time-slice of the orbit
	a = asin(x / (2 * R));

	// current angle of propagation
	c = vel.atan();

	return v * 2*sin(a) * unit_vector(c + a + 0.5*PI);
}

void AsteroidBelt::calculate()
{
	STACKTRACE

	
	// at a distance R, and vel v, you need the following correction towards the center to maintain orbit:


	vel += correction();

	Asteroid::calculate();

	double rtest;
	rtest = min_delta(pos, center).length();
	if (rtest < Rmin || rtest > Rmax)
		state = 0;
}


//	virtual int accelerate(SpaceLocation *source, double angle, double vel, double max_speed=MAX_SPEED); //accelerates an object by vel at angle, to a maximum of max_speed
//	virtual int accelerate(SpaceLocation *source, Vector2 delta_v, double max_speed=MAX_SPEED); //changes an objects velocity by delta_v, to a maximum of max_speed
int AsteroidBelt::accelerate(SpaceLocation *source, double angle, double vel, double max_speed)
{
	STACKTRACE

	if (source->isPlanet())
		return 0;	// do nothing.
	else
		return Asteroid::accelerate(source, angle, vel, max_speed);
}

int AsteroidBelt::accelerate(SpaceLocation *source, Vector2 delta_v, double max_speed)
{
	STACKTRACE

	if (source->isPlanet())
		return 0;	// do nothing.
	else
		return Asteroid::accelerate(source, delta_v, max_speed);
}


AsteroidZone::AsteroidZone(Vector2 ocenter, double oRmax, double ov)
:
Asteroid()
{
	Rmax = oRmax;
	center = ocenter;

	double a;

	a = random(PI2);
	v_unscaled = ov;
	v = scale_velocity(ov);

	vel = random(0.1*v, v) * unit_vector(random(PI2));

	pos = center + random(Rmax) * unit_vector(random(PI2));
}



void AsteroidZone::death()
{
	STACKTRACE


	Animation *a = new Animation(this, pos,
			explosion, 0, explosion->frames(), time_ratio, get_depth());
	a->match_velocity(this);
	game->add(a);
	
	game->add(new AsteroidZone(center, Rmax, v_unscaled));
	return;
}


void AsteroidZone::calculate()
{
	
	STACKTRACE

	Asteroid::calculate();

	double rtest;
	rtest = (pos - center).length();
	if (rtest > Rmax)
		state = 0;
}









MissionMessageBox::MissionMessageBox(BITMAP *bmp, int oxpos, int oypos, double operiod)
{
	bmp_display = bmp;

	time = 0;
	period = operiod;

	xpos = oxpos;
	ypos = oypos;

	set_depth(999.0);
}

MissionMessageBox::~MissionMessageBox()
{
	destroy_bitmap(bmp_display);
}

void MissionMessageBox::calculate()
{
	Presence::calculate();

	time += frame_time * 1E-3;
	if (time > period)
		state = 0;
}

// some stuff copied from SpaceSprite::draw
void MissionMessageBox::animate(Frame *space)
{
	if (!exists())
		return;

//	int aa = aa_get_mode();
//	aa |= AA_MASKED;
//	aa |= AA_ALPHA;
//	aa |= AA_NO_AA;
//	aa_set_mode(aa);

	int aa = 0;
	aa |= AA_NO_AA;
//	aa |= AA_BLEND;
//	aa |= AA_MASKED;
//	aa |= AA_ALPHA;
	aa_set_mode(aa);

//	int aat = aa_get_trans();
//	aa_set_trans(240);

	if ( int(time/0.1) % 2 == 0 || time > 1.0 )		// first 1 second, flicker.
	aa_stretch_blit(bmp_display, space->surface,
		0, 0, bmp_display->w, bmp_display->h, 
		xpos, ypos, bmp_display->w, bmp_display->h);

//	aa_set_trans(aat);

	space->add_box(xpos, ypos, bmp_display->w, bmp_display->h);

}




SpecialAreaTag::SpecialAreaTag(SpaceLocation *s, double aR)
:
SpecialArea(s->pos, aR)
{
	tagship = s;
}

void SpecialAreaTag::calculate()
{
	if (tagship && tagship->exists())
		center = tagship->pos;
	else
		tagship = 0;
}


void SpecialAreaTag::animate(Frame *space)
{
	if (tagship)
		SpecialArea::animate(space);
}






MissionPointer::MissionPointer(SpaceLocation *otarget)
:
WedgeIndicator(otarget, 100, 1)
{
	hidden = false;
	missiontarget = otarget;
}

void MissionPointer::target(SpaceLocation *otarget)
{
	missiontarget = otarget;
	WedgeIndicator::target = missiontarget;
}

void MissionPointer::hide()
{
	hidden = true;
}

void MissionPointer::show()
{
	hidden = false;
}


void MissionPointer::calculate()
{
	WedgeIndicator::target = missiontarget;

	if (!(missiontarget && missiontarget->exists() ))
		missiontarget = 0;

	if (missiontarget && !hidden)
		WedgeIndicator::calculate();
}

void MissionPointer::animate(Frame *frame)
{
	if (missiontarget && !hidden)
		WedgeIndicator::animate(frame);
}


