/* $Id$ */ 
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
#include "../ship.h"
#define STATION_LOG




const int	Num_Planet_Pics	= 8;
const int	Num_Giant_Pics	= 2;
const int	Num_Moon_Pics	= 3;
const int	Num_Star_Pics	= 18;


class Solar : public NormalGame
{
	SpaceSprite *PlanetPics[Num_Planet_Pics];
	SpaceSprite *GiantPics[Num_Giant_Pics];
    SpaceSprite *MoonPics[Num_Moon_Pics];
	SpaceSprite *StarPic;

//   virtual void Solar::init(Log *_log);
	virtual void init_objects();

	bool GetSprites(SpaceSprite *Pics[], char *fileName, char *cmdStr, int numSprites);
	SpaceSprite *GetSprite(char *fileName, char *spriteName);

	// initialization of the comets in the solar melee game
	void init_comets ();
};


	
static const int	Num_Tail_Pics = 6;


class Comet : public SpaceObject {
public:
	double gravity_force;
	double gravity_mindist;
	double gravity_range;
	double gravity_power;
	double gravity_whip;
	
	double	wait_duration, wait_time;
	int		Num_Tail_Pic;

	SpaceSprite **TailPic;
	Comet(Vector2 p, SpaceSprite *sprite_comet, SpaceSprite **sprite_tail, int index, int Nframes);
	
	virtual void inflict_damage(SpaceObject *other);
	virtual void calculate();
};



// Adaptation of the Mycon plasma:

class CometPlasma : public Missile {
	double v;
	
	int frame_count;
	int max_damage;
	SpaceSprite   **spritelist;
	int			iSpaceSprite, nSpaceSprite;
	int			iframecount;
	
	double time_existence[10];
	double total_time_existence, time_inexistence;
	
public:
	CometPlasma(Vector2 opos, double oangle, double ov, int odamage,
		double orange, double otrate, Comet *oship, SpaceSprite *osprite[], int ofcount);
	
	virtual void calculate();
	
	virtual void inflict_damage(SpaceObject *other);
	virtual int handle_damage(SpaceLocation *source, int normal, int direct);
};



void distribute_time_ran(double *x, int N, double xmax)
{
	STACKTRACE;

	int i;
	int imax = 1000;

	for ( i = 0; i < N; ++i )
		x[i] = tw_random(imax);

	for ( i = 1; i < N; ++i )
		x[i] += x[i-1];			// cumulative

	for ( i = 0; i < N; ++i )
		x[i] *= xmax / x[N-1];

	
}



CometPlasma::CometPlasma(Vector2 opos, double oangle, double ov,
						 int odamage, double orange, double otrate, Comet *oship,
						 SpaceSprite **osprite, int Nframes) :
Missile( oship,opos, oangle, ov, odamage, orange, 0, oship,
		osprite[0]),
		v(ov),
		frame_count(Nframes),
		max_damage(odamage)
{
	
	collide_flag_sameship |= bit(LAYER_SHIPS);
	collide_flag_sameteam |= bit(LAYER_SHIPS);
	
	sprite_index = 0;		// this refers to a frame in an animated sprite
	iSpaceSprite = 0;
	nSpaceSprite = Nframes;
	
	iframecount = 0;
	
	spritelist = osprite;		// this is a different list of sprites; each could be an
	// animation itself in principle, but isnt.


	total_time_existence = 1500 + random(500);
	distribute_time_ran(time_existence, Nframes, total_time_existence);

	time_inexistence = 0;
	
}

void CometPlasma::calculate()
{
	STACKTRACE;

	
	
	Missile::calculate();
	
	
	damage_factor = max_damage / (iSpaceSprite+1);

	sprite_index = 0;	// cause there is only 1 picture for this sprite element in the list...
	
	// use random to make the sprites change and disappear slowly over time:
	
	
	time_inexistence += frame_time;
	
	if ( time_inexistence > time_existence[iSpaceSprite] )	// evolve
	{
		++iSpaceSprite;
		
		if (iSpaceSprite < nSpaceSprite )
		{
			sprite = spritelist[iSpaceSprite];
			
			// the new sprite has a different size, so:
			size = sprite->size();
		}
		else
			state = 0;
			//iSpaceSprite = nSpaceSprite-1;
		
	}
	
	
	
}



void CometPlasma::inflict_damage(SpaceObject *other)
{
	STACKTRACE;

	
	SpaceObject::inflict_damage(other);
	
	state = 0;
}


int CometPlasma::handle_damage(SpaceLocation *source, int normal, int direct)
{
	STACKTRACE;

	
	int total = normal + direct;
	
	state = 0;		// disappear at once if it gets hit.
	
	return total;
}






Comet::Comet(Vector2 p, SpaceSprite *sprite_comet, SpaceSprite **sprite_tail, int index, int Nframes) 
:
SpaceObject(NULL, p, 0.0, sprite_comet) 
{
	collide_flag_sameship = ALL_LAYERS;
	layer = LAYER_CBODIES;
	set_depth(DEPTH_PLANETS);
	//id         |= ID_PLANET;
	id         |= SPACE_SHIP;
	mass        = 9999999.0;
	//use remote .ini file
	game->log_file ("server.ini");
	sprite_index = index;
	
	TailPic = sprite_tail;
	Num_Tail_Pic = Nframes;
	
	wait_duration = 100.0;		// in milliseconds
	wait_time = wait_duration;
}



void Comet::inflict_damage(SpaceObject *other) {
	STACKTRACE;

	int i = 1;
	if (other->isShip()) {
		i = (int) ceil(((Ship*)other)->crew / 3.0);
	}
	if (other->mass == 0) other->state = 0;
	damage(other, 0, i);
	i /= 2;
	if (i >= BOOM_SAMPLES) i = BOOM_SAMPLES - 1;
	if (!other->isShot()) 
		play_sound((SAMPLE *)(melee[MELEE_BOOM + i].dat));
	return;
}



void Comet::calculate() {
	STACKTRACE;

	SpaceObject::calculate();
	
	wait_time -= frame_time;
	
	if ( wait_time < 0.0 )
	{
		wait_time += wait_duration;
		
		// the comet auto-fires new plasma objects:
		double weaponVelocity = 0.2;
		double weaponDamage = 3.0;
		double weaponRange = 5000.0;
		double weaponHome = 0.0;		// turn rate is not used here.
		
		
		// leave a trail of smoke behind - moving away from the comet
		
		
		
		//In the routine Shot::Shot( ... , ox, oy, ....)
		//the weapon has speed in the direction where the ship (comet) points to.
		//the weapon has orientation in the angle that you specify
		
		//In order to leave a trail of smoke we've to trick it, by temporarily changing the
		//orientation of the comet...
		
		
		
		double anglev = atan3(vel);		// direction of motion
		double angler = PI + anglev + tw_random(-10*ANGLE_RATIO, 10*ANGLE_RATIO); // orientation of the gas cloud
		
		double angleold = angle;
		this->angle = anglev;		// temporarily reset the orientation of the comet since the gas
		// is always ejected in that direction.
		
		double cometradius = size.y;
		double xoffs = 0.0;					// this is already compensated for direction ??
		double yoffs = -cometradius;
		
		//angler = 0.0;
		add(new CometPlasma(Vector2(xoffs, yoffs),
			angler, weaponVelocity, (int)weaponDamage, weaponRange, weaponHome, this,
			TailPic, Num_Tail_Pic));
		
		this->angle = angleold;	// re-orientate the comet.
		
	}
	
	return;
}









void Solar::init_comets ()
{
	STACKTRACE;

	

	SpaceSprite *CometPic;
	SpaceSprite **TailPic;
	
	
	log_file("solar_comet.ini");
	int CometNum  = get_config_int(NULL, "Comet_num", 0);
	int CometMass = get_config_int(NULL, "Comet_mass", 0);
	double CometVel  = scale_velocity( get_config_int(NULL, "Comet_vel", 0) );
	
	
	
	CometPic = GetSprite("solar_comet.dat","Comet000");
	if(CometPic==NULL)	error("File error, comet pic.  Bailing out...");
	
	TailPic = new SpaceSprite* [Num_Tail_Pics];	// the subroutine does not do this for us ...
	if(GetSprites(TailPic,"solar_comet.dat","Tail%03d",Num_Tail_Pics)==FALSE)
		error("File error, comet pics.  Bailing out...");
	
	
	Comet	*Cometbody;
	
	
	// add all the comets you need ...
	for ( int num = 0; num < CometNum; ++num )
	{
		
	
		Cometbody = new Comet (random(size), CometPic, TailPic, 0, Num_Tail_Pics);
		Cometbody->id=COMET_ID;
		Cometbody->mass = CometMass;
		Cometbody->gravity_force *= 0;
		Cometbody->gravity_whip = 0;
		//Cometbody->accelerate(NULL, random(PI2), get_config_int(NULL, "Comet_acc", 2), get_config_int(NULL, "Comet_max", 2));
		
		double angle = tw_random(PI2);
		Cometbody->vel = CometVel * unit_vector(angle);
		
		add (Cometbody);
	}
	
}



SpaceSprite *Solar::GetSprite(char *fileName, char *spriteName)
{
	STACKTRACE;

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

	SpaceSprite *spr=new SpaceSprite(tmpdata, 1, SpaceSprite::MASKED, 1);

	unload_datafile_object(tmpdata);

	return spr;
}

bool Solar::GetSprites(SpaceSprite *Pics[], char *fileName, char *cmdStr, int numSprites)
{
	STACKTRACE;

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


void Solar::init_objects()
{
	STACKTRACE;

	add(new Stars());
	char starName[100];

	if(GetSprites(PlanetPics,"solar.dat","Station_Planet%03d",Num_Planet_Pics)==FALSE)
		error("File error, planet pics.  Bailing out...");

	if(GetSprites(GiantPics,"solar.dat","Station_Giant%03d",Num_Giant_Pics)==FALSE)
		error("File error, giant pics.  Bailing out...");

	if(GetSprites(MoonPics,"solar.dat","Station_Moon%03d",Num_Moon_Pics)==FALSE)
		error("File error, moon pics.  Bailing out...");

	sprintf(starName,"Star%03d",random(Num_Star_Pics));

	StarPic=GetSprite("solar.dat",starName);
	if(StarPic==NULL)	error("File error, star pic.  Bailing out...");

	Sun *Centre;
	Planet *Satellite;
	Planet *Moon;
	OrbitHandler *handler;
	
	Centre = new Sun(size/2, StarPic,0);

	//Probably a good idea to get all your variables immediately after
	//opening your .ini file.
	log_file("solar.ini");
	int GasGrav = get_config_int(NULL, "Gasgravity",0);
	double MoonGrav = get_config_float(NULL, "Moongravity", 0);
	int NumPlanets = get_config_int(NULL, "NPlanets", 0);
	int NumMoons = get_config_int(NULL, "Moons", 0);
	int Radius = get_config_int(NULL, "Radius", 0);
	int MRadius = get_config_int(NULL, "MRadius", 0);
	int Asteroids=get_config_int(NULL,"Asteroids",0);


	//Make sure they're being read properly.
//	iMessage("NPlanets = %d",NumPlanets);
//	iMessage("Radius   = %d",Radius);
//	iMessage("GasGrav  = %d",GasGrav);
//	iMessage("MoonGrav = %d",MoonGrav);
	//iMessage("Comets   = %d",Comets);
	//iMessage("CoMass   = %d",CoMass);
	//iMessage("ComMax   = %d",ComMax);
//	iMessage("Asteroids= %d",Asteroids);
	//iMessage("ComAcc   = %d",ComAcc);

  //	Centre->gravity_force *= Grav;
	size.x = size.y = (NumPlanets*Radius+200)*2;   //size formula
	//iMessage("Size   = %d",(NumPlanets*Radius+200)*2);
	game->add(Centre);

	// planets creating loop
	int kind;
	int moons;
	int num;
	for(num=0; num<NumPlanets; num++)
	{
		kind = random(2);
		moons = random(NumMoons+1);
		if ((kind == 0) && (((num+1)*Radius)> 1600)) {      // gas giant
			Satellite = new Planet(size/2,GiantPics[random(Num_Giant_Pics)],0);
			Satellite->gravity_force *= GasGrav;

			handler = new OrbitHandler(Centre,size/2,random(PI2), (SpaceLocation *)Centre,
			(SpaceLocation *)Satellite, (num+1)*Radius, 10.0 * PI/180.0, 1);

		}
		else {               // normal planet
			Satellite = new Planet(size/2,PlanetPics[random(Num_Planet_Pics)],0);

			handler = new OrbitHandler(Centre,size/2,random(PI2), (SpaceLocation *)Centre,
			(SpaceLocation *)Satellite, (num+1)*Radius, 10.0 * PI/180.0, 1);
		}

		game->add(Satellite);
		game->add(handler);

									// moons code
		int i;
		for (i=0; i<moons; i+=1)
		{
			Moon = new Planet(size/2,MoonPics[random(Num_Moon_Pics)],0);
			Moon->gravity_force *= MoonGrav;
	        Moon->gravity_range = 8;
			Moon->id=MOON_ID;

			handler = new OrbitHandler(Satellite,size/2,random(PI2), (SpaceLocation *)Satellite,
			(SpaceLocation *)Moon, (i+1)*MRadius, 10.0 * PI/180.0, 0);

			game->add(Moon);
			game->add(handler);

		}

		iMessage("Planet #%d added",num+1);
	}





	// the new comet code - is in a different file, for clarity!

	this->init_comets();


	//asteroids code
	Asteroids = get_config_int(NULL, "Asteroids", 0);
	if (Asteroids > 0) for (int num = 0; num < Asteroids; num += 1) add(new Asteroid());
	else {
		NumPlanets = get_config_int(NULL, "NPlanets", 2);
		Radius = get_config_int(NULL, "Radius", 2);
		if ((NumPlanets*Radius+200)*2>3840) for (int i = 0; i < ((NumPlanets*Radius+200)*2)/900; i += 1) add(new Asteroid());
		else for (int i = 0; i < 4; i += 1) add(new Asteroid());
	}

}

/*void Solar::init(Log *_log) {
	NormalGame::init(_log);
	log_file("Solar.ini");
	int j,m;
	j = get_config_int(NULL, "NPlanets", 2);
	m = get_config_int(NULL, "Radius", 2);
	if ((j*m+200)*2>3840) {
	width = (j*m+200)*2;
	height = (j*m+200)*2;
	}
	else {
	width = 3840;
	height = 3840;
	}
	return;
	}    */

REGISTER_GAME (Solar, "Melee in solar system (INI)");
