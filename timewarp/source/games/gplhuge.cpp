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
//#define STATION_LOG

#define Num_Planet_Pics 7

class Huge : public NormalGame
{

	SpaceSprite *HugePics[Num_Planet_Pics];

	virtual void init_objects();

	bool GetSprites(SpaceSprite *Pics[], char *fileName, char *cmdStr, int numSprites);
	SpaceSprite *GetSprite(char *fileName, char *spriteName);
};


SpaceSprite *Huge::GetSprite(char *fileName, char *spriteName)
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

	#ifdef STATION_LOG
		sprintf(msgStr, "Succesfully loaded %s#%s!  Hot damn!",fileName,spriteName);
		message.out(msgStr);
	#endif

	SpaceSprite *spr=new SpaceSprite(tmpdata, 1, SpaceSprite::MASKED, 1);
	unload_datafile_object(tmpdata);

	return spr;
}

bool Huge::GetSprites(SpaceSprite *Pics[], char *fileName, char *cmdStr, 
int numSprites)
{

	SpaceSprite *spr;
	char dataStr[100];

	for(int num=0; num<numSprites; num++)
	{
		sprintf(dataStr,cmdStr,num);
		spr=GetSprite(fileName, dataStr);
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


	if(GetSprites(HugePics,"plhuge.dat","Station_Planet%03d",Num_Planet_Pics)==FALSE)
		error("File error, planet pics.  Bailing out...");


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
		PlanetType = random()%Num_Planet_Pics;
		iMessage("PlanetType= %d *RANDOM*",PlanetType);
		}
	else iMessage("PlanetType= %d *PRESET*",PlanetType);
	Centre = new Planet(size/2, HugePics[PlanetType],0);



	//mapsize
/*if (MapSize < 2000) 
	{
		width = height = 2 * 2 * distancetemp[NumMoons-1]; // two times diamter = 4 times radius of the system
		//(NumMoons*Radius+200)*2;   //size formula
		if (width < 3600) width = height =3600; // minimum screen size of 3600
	
		iMessage("Size   = %d *FORMULA*",(width));
		game->add(Centre);
	}
	else {*/
		size.x = size.y = MapSize;
		iMessage("Size   = %d *PRESET*",(size.x));
		game->add(Centre);	

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
		//double a=(random(PI2));
	  //	c->vx=(ComMax/CoMass)*cos(a);
	  //	c->vy=(ComMax/CoMass)*sin(a);
		c->accelerate(NULL, random(PI2), get_config_int(NULL, "Comet_acc", 2), 
		get_config_int(NULL, "Comet_max", 2));
		add (c);
	}

	//asteroids code
	Asteroids = get_config_int(NULL, "Asteroids", 0);
	if (Asteroids > 0) for (int num = 0; num < Asteroids; num += 1) add(new Asteroid());
	else {
//		NumMoons = get_config_int(NULL, "NPlanets", 2);
//		Radius = get_config_int(NULL, "Radius", 2);
//		if ((NumMoons*Radius+200)*2>3840) for (int i = 0; i < ((NumMoons*Radius+200)*2)/900; i += 1) add(new Asteroid());
		 for (int i = 0; i < 4; i += 1) add(new Asteroid());
	}

	}



REGISTER_GAME (Huge, "Melee w/ Gargantuan planet (INI)");

