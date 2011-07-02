/* $Id$ */ 
/*hacked version of the solar system melee by UAF and Corona688.
I can't really code, so I probably made many a coding faux pas.  And its ugly.
Unused code from solar melee was either removed or commented or still
exists with no function*/

#include <allegro.h>
#include <stdio.h>
#include "../melee.h"

REGISTER_FILE
#include "../melee/mframe.h"
#include "../melee/mgame.h"
#include "../melee/mmain.h"
#include "../melee/mcbodies.h"
#include "../melee/mview.h"
#include "../other/orbit.h"
#include "../melee/mitems.h"
#include "../util/aastr.h"
//#define STATION_LOG

#define Num_Planet_Pics 7

class Huge : public NormalGame
{

	SpaceSprite *HugePics[Num_Planet_Pics];

	virtual void init_objects();

	bool GetSprites(SpaceSprite *Pics[], char *fileName, char *cmdStr, int numSprites, int attribs);
	SpaceSprite *GetSprite(char *fileName, char *spriteName, int attribs);
};



SpaceSprite *Huge::GetSprite(char *fileName, char *spriteName, int attribs)
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

bool Huge::GetSprites(SpaceSprite *Pics[], char *fileName, char *cmdStr, int numSprites, int attribs)
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

void Huge::init_objects()
{
	 

	// set the anti-alias to 0:
	int antia_on =  get_config_int(NULL, "Anti-AliasingOn",0);;
	set_tw_aa_mode(antia_on);


	// load the sprite, but set the anti-alias to 0 for them - they're _big_:
	if(GetSprites(HugePics,"plhuge.dat","Station_Planet%03d",Num_Planet_Pics,
		SpaceSprite::MASKED | SpaceSprite::NO_AA)==FALSE)
		throw("File error, planet pics.  Bailing out...");


	//opening your .ini file.

	log_file("plhuge.ini");
	int Comets = get_config_int(NULL, "Comets",0);
	int CoMass = get_config_int(NULL, "Comet_mass",0);
	int ComMax = get_config_int(NULL, "Comet_max",0);
	int Asteroids=get_config_int(NULL,"Asteroids",0);
	int ComAcc = get_config_int(NULL, "Comet_acc",0);
	int MapSize=get_config_int(NULL,"MapSize",0);
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

	
//Select planet
	Planet *Centre;
	if (PlanetType == -1) {
		PlanetType = tw_random(Num_Planet_Pics);
		iMessage("PlanetType= %d *RANDOM*",PlanetType);
		}
	else iMessage("PlanetType= %d *PRESET*",PlanetType);
	Centre = new Planet(size/2, HugePics[PlanetType],0);
	game->add(Centre);
	game->add(new WedgeIndicator(Centre, 75, 4));

	// copied from Planet::Planet :
	// note that Planet constructor calls another log file.
	log_file("plhuge.ini");
	Centre->gravity_mindist = scale_range(get_config_float("GPlanet", "GravityMinDist", 0));
	Centre->gravity_range = scale_range(get_config_float("GPlanet", "GravityRange", 0));
	Centre->gravity_power = get_config_float("GPlanet", "GravityPower", 0);
	Centre->gravity_force = scale_acceleration(get_config_float("GPlanet", "GravityForce", 0), 0);
	Centre->gravity_whip = get_config_float("GPlanet", "GravityWhip", 0);
	Centre->gravity_whip2 = get_config_float("GPlanet", "GravityWhip2", 0);

	double vx, vy;
	vx = scale_velocity( get_config_float("GPlanet", "VelX", 0) );
	vy = scale_velocity( get_config_float("GPlanet", "VelY", 0) );
	Centre->vel = Vector2( vx, vy );

	size.x = size.y = MapSize;
	iMessage("Size   = %d *PRESET*",MapSize);

	int num;

	//comet code
	for (num = 0; num < Comets; num++)
	{
		Planet *c;
		c = new Planet (random(size), meleedata.kaboomSprite, 1);
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
			a->pos = Centre->pos + (400+tw_random(400)) * unit_vector(tw_random(PI2));
			a->vel = 500*unit_vector(a->trajectory_angle(Centre)+PI/2) / a->distance(Centre);
		}
	} else {
		 for (int i = 0; i < 4; i += 1)
			 add(new Asteroid());
	}

	}



REGISTER_GAME (Huge, "Melee w/ Gargantuan planet (INI)");

