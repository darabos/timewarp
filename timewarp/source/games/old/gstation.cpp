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
#include "../melee/mship.h"    //ships
#include "../melee/mlog.h"     //networking / demo recording / demo playback
#include "../melee/mcontrol.h" //controllers & AIs  
#define STATION_LOG

#define Num_Planet_Pics 8
#define Num_Star_Pics 18
class Station : public NormalGame
{
	SpaceSprite *PlanetPics[Num_Planet_Pics];
	SpaceSprite *StarPic;

	virtual void init_objects();
	virtual void Station::init(Log *_log);
   TeamCode station_team;

	bool GetSprites(SpaceSprite *Pics[], char *fileName, char *cmdStr, int numSprites);
	SpaceSprite *GetSprite(char *fileName, char *spriteName);
};

/*void iMessage(char *cmdstr, int num)
{
	char buf[200];
	sprintf(buf, cmdstr,num);
	message.out(buf);
}   */

SpaceSprite *Station::GetSprite(char *fileName, char *spriteName)
{
	char msgStr[100];
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

	return new SpaceSprite(tmpdata, 1, SpaceSprite::MASKED, 64);	
}

bool Station::GetSprites(SpaceSprite *Pics[], char *fileName, char *cmdStr, int numSprites)
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

void Station::init_objects()
{

	if(GetSprites(PlanetPics,"solar.dat","Station_Planet%03d",Num_Planet_Pics)==FALSE)
		error("File error, planet pics.  Bailing out...");

/*	StarPic=GetSprite("station.dat",starName);
	if(StarPic==NULL)	error("File error, star pic.  Bailing out...");   */

//	if(GetSprites(StarPics,"station.dat","Star%03d",Num_Star_Pics)==FALSE)
//		error("File error, star pics.  Bailing out...");

	Planet *Centre;

	//Ship *Station1;
	OrbitHandler *handler;

	Centre = new Planet(width/2, height/2, PlanetPics[rand()%Num_Planet_Pics],0);
	game->add(Centre);

	Ship *Station1 = create_ship(channel_none, "staba", "WussieBot", width/2, height/2, 0 /*random(PI2)*/, station_team);


	handler = new OrbitHandler(Centre,width/2,height/2,0, (SpaceLocation *)Centre,
	(SpaceLocation *)Station1, 400, 10.0,1);

	add(Station1);
	game->add(handler);

	/*Planet *c;
	int z;
	z = get_config_int(NULL, "Comets", 0);
	for (i = 0; i < z; i += 1) {
		c = new Planet (random()%int(width), random()%int(height), kaboomSprite, 1);
		c->mass = get_config_int(NULL, "Comet_mass", 2);;
		c->gravity_force *= 0;
		c->gravity_whip = 0;
		c->accelerate(NULL, random(PI2), get_config_int(NULL, "Comet_acc", 2), get_config_int(NULL, "Comet_max", 2));
		addItem (c);
		}        */


	for (int i = 0; i < 4; i += 1) add(new Asteroid());


}

void Station::init(Log *_log) {
	NormalGame::init(_log);
	station_team = new_team();
	return;
	}

REGISTER_GAME (Station, "Melee with station");