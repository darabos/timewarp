#include <allegro.h>
#include <stdio.h>

#include "../melee.h"
REGISTER_FILE
#include "../melee/mframe.h"
#include "../melee/mgame.h"
#include "../melee/mmain.h"
#include "../melee/mcbodies.h"
#include "../melee/mview.h"

#ifndef _V_BODIES_H
#include "../other/vbodies.h"
#endif

class VGenSystem : public NormalGame {
public:
  SpaceSprite* AstSpr1[64];
  SpaceSprite* AstSpr2[64];
  SpaceSprite* AstSpr3[64];
  SpaceSprite* AstSpr4[64];
  SpaceSprite* PlanetMoon;
  SpaceSprite* PlanetHabitable;
  SpaceSprite* PlanetInhospitable;
  SpaceSprite* PlanetRockball;
  SpaceSprite* PlanetGasGiant;
  SpaceSprite* Flare1[64];
  SpaceSprite* Flare2[64];
  SpaceSprite* Flare3[64];
  SpaceSprite* Flare4[64];
  SpaceSprite* BrownDwarfSpr;
  SpaceSprite* RedDwarfSpr;
  SpaceSprite* WhiteDwarfSpr;
  SpaceSprite* RedStarSpr;
  SpaceSprite* OrangeStarSpr;
  SpaceSprite* YellowStarSpr;
  SpaceSprite* WhiteStarSpr;
  SpaceSprite* RedGiantSpr;
  SpaceSprite* BlueGiantSpr;
  SpaceSprite* NeutronStarSpr[64];
  SpaceSprite* BlackHoleSpr[64];
  SpaceSprite* DustCloudSpr;
  SpaceSprite* GasCloudSpr[2];
  SpaceSprite* SpaceMineSpr;
  SpaceSprite* SpaceMineExplosionSpr;
  SpaceSprite* DefSatSpr;
  SpaceSprite* SpaceOutpostSpr;
  SpaceSprite* SpaceColonySpr;
  SpaceSprite* SpaceStationSpr;

  SpaceLocation* mapCenter;
  void setupSprites(void);
  void setupBinary(SpaceObject* S1, SpaceObject* S2, double radius1, double radius2, double speed1);
	virtual void init_objects();
  bool GetSpriteTo64Rot(SpaceSprite *Pics[], char *fileName, char *cmdStr, 
    int numSprites = 1, int attribs = -1);
  SpaceSprite* GetSprite(char *fileName, char *spriteName, int attribs, int rotations=1, int numSprites = 1);
  bool GetSpriteGroup(SpaceSprite *Pics[], char *fileName, char *cmdStr, int numSprites, int attribs=-1, int firstSpriteNumber=1);
  SpaceSprite* GetMultiframeSprite(char *fileName, char *spriteName, int attribs, int numberOfFrames);
  virtual ~VGenSystem();
};


class VAncientBattlefield : public VGenSystem {
  virtual void init_objects(void);
};

void VAncientBattlefield::init_objects(void) {
  int i;
  VGenSystem::init_objects();
  for(i=0; i<20; i++) {
    game->add(new VDustCloud());
    game->add(new VGasCloud());
    game->add(new VMetalShard());
    game->add(new VSpaceMine());
  }
  game->add(new VSmallAsteroid());
  game->add(new VLargeAsteroid());
  game->add(new VMetalAsteroid());
}


class VSpaceStationGame : public VGenSystem {
  virtual void init_objects(void);
};

void VSpaceStationGame::init_objects(void) {
  VDeepSpaceStation* DSS;
  VGenSystem::init_objects();
  DSS = new VDeepSpaceStation();
  game->add(DSS);
  game->add(new VLargeAsteroid());
  DSS->AddInstallation(new VGroundDefenseLaser(DSS));
  DSS->AddInstallation(new VGroundMissileLauncher(DSS));
  DSS->AddInstallation(new VGroundIonCannon(DSS));
}

class VImperialCapitol : public VGenSystem {
  virtual void init_objects(void);
};

void VImperialCapitol::init_objects() {
  VHabitablePlanet* VHP;
  VGroundDefenseLaser *GDL;
  VGenSystem::init_objects();
  VHP = new VHabitablePlanet();
  game->add(VHP);
  // Laser #1:
  GDL = new VGroundDefenseLaser(VHP);
  VHP->AddInstallation(GDL);
  // Laser #2:
  GDL = new VGroundDefenseLaser(VHP);
  VHP->AddInstallation(GDL);
  GDL->weaponColor = 7;
  GDL->actionsPerSecond = 0.4;
  GDL->weaponRange = scale_range(40.0);
  // Laser #3
  GDL = new VGroundDefenseLaser(VHP);
  VHP->AddInstallation(GDL);
  GDL->weaponColor = 10;
  GDL->actionsPerSecond = 3.5;
  GDL->weaponRange = scale_range(4.0);
  VHP->AddInstallation(new VGroundIonCannon(VHP));
  VHP->AddInstallation(new VGroundMissileLauncher(VHP));
  VHP->AddInstallation(new VFortification(VHP));
  VHP->AddInstallation(new VColony(VHP));
  VHP->AddInstallation(new VColony(VHP));
  VHP->AddInstallation(new VColony(VHP));
  VHP->AddInstallation(new VColony(VHP));
  VHP->AddInstallation(new VMine(VHP));
  VHP->AddInstallation(new VPlanetaryShield(VHP));
  game->add(new VSmallAsteroid());
  game->add(new VLargeAsteroid());
  game->add(new VDustCloud());
  game->add(new VMetalAsteroid());
}


class VDefendedPlanet : public VGenSystem {
  virtual void init_objects(void);
};

void VDefendedPlanet::init_objects(void) {
  VRockballPlanet* VRP;
  VGenSystem::init_objects();
  VRP = new VRockballPlanet();
  game->add(VRP);
  game->add(new VLargeAsteroid());
  VRP->AddInstallation(new VGroundDefenseLaser(VRP));
  VRP->AddInstallation(new VGroundMissileLauncher(VRP));
  VRP->AddInstallation(new VGroundIonCannon(VRP));
}


class VMineField : public VGenSystem {
  virtual void init_objects(void);
};

void VMineField::init_objects(void) {
  int i;
  VSpaceMine* VSM;
  VGenSystem::init_objects();
  for(i=0; i<90; i++){
    VSM = (new VSpaceMine());
    //VSM->data->spriteWeaponExplosion = SpaceMineExplosionSpr;
    //VSM->explosionSprite = SpaceMineExplosionSpr;
    //VSM->explosionSprite = VSpaceMine::explosionSprite;
    game->add(VSM);
  }
  game->add(new VSmallAsteroid());
  game->add(new VLargeAsteroid());
  game->add(new VMetalAsteroid());
}

class VSysNebula : public VGenSystem {
  virtual void init_objects(void);
};

void VSysNebula::init_objects(void) {
  game->add(new VNebula());
  VGenSystem::init_objects();
  game->add(new VSmallAsteroid());
  game->add(new VLargeAsteroid());
  game->add(new VMetalAsteroid());
}

class VSysDusty : public VGenSystem {
  virtual void init_objects(void);
};

void VSysDusty::init_objects(void) {
  VGenSystem::init_objects();
  for(int i=0; i<33; i++)
    game->add(new VDustCloud());
}

class VSysGassy : public VGenSystem {
  virtual void init_objects(void);
};

void VSysGassy::init_objects(void) {
  VGenSystem::init_objects();
  for(int i=0; i<33; i++)
    game->add(new VGasCloud());
}


class VSysBlueGiant : public VGenSystem {
  virtual void init_objects(void);
};

void VSysBlueGiant::init_objects(void) {
  VGenSystem::init_objects();
  game->add(new VBlueGiant());
}

class VSysWhiteDwarf : public VGenSystem {
  virtual void init_objects(void);
};

void VSysWhiteDwarf::init_objects(void) {
  VGenSystem::init_objects();
  game->add(new VWhiteDwarf());
}

class VSysWhiteStar : public VGenSystem {
  virtual void init_objects(void);
};

void VSysWhiteStar::init_objects(void) {
  VGenSystem::init_objects();
  game->add(new VWhiteStar());
}


class VSysYellowThreePlanets : public VGenSystem {
  virtual void init_objects(void);
};

void VSysYellowThreePlanets::init_objects(void) {
  int i;
  VYellowStar* YS;
  VRockballPlanet* RP;
  VHabitablePlanet* HP;
  VGasGiant* GG;
  VMoon* M1;
  VMoon* M2;
  VMoon* M3;
  OrbitHandler* OH;
  VGenSystem::init_objects();
  YS = new VYellowStar();
  RP = new VRockballPlanet();
  HP = new VHabitablePlanet();
  GG = new VGasGiant();
  M1 = new VMoon();
  M2 = new VMoon();
  M3 = new VMoon();
  game->add(YS);
  game->add(RP);
  game->add(HP);
  game->add(GG);
  game->add(M1);
  game->add(M2);
  game->add(M3);
  OH = new OrbitHandler(mapCenter, Vector2(0, 0),
        0, YS, RP, scale_range(10), scale_velocity(5), 1);
  game->add(OH);
  OH = new OrbitHandler(mapCenter, Vector2(0, 0),
        0, YS, HP, scale_range(15), scale_velocity(3.75), 1);
  game->add(OH);
  OH = new OrbitHandler(mapCenter, Vector2(0, 0),
        0, YS, GG, scale_range(22.5), scale_velocity(2.5), 1);
  game->add(OH);
  OH = new OrbitHandler(GG, GG->pos,
        0, GG, M1, scale_range(2), scale_velocity(4), 0);
  game->add(OH);
  OH = new OrbitHandler(GG, GG->pos,
        PI/2, GG, M2, scale_range(3), scale_velocity(3), 0);
  game->add(OH);
  OH = new OrbitHandler(GG, GG->pos,
        PI, GG, M3, scale_range(4), scale_velocity(2), 0);
  game->add(OH);
  for(i=0; i<4; i++)
    game->add(new VSmallAsteroid());
}

class VSysVoid : public VGenSystem {
  virtual void init_objects(void);
};

void VSysVoid :: init_objects(void) {
  VGenSystem::init_objects();
}


class VSysBrownDwarf : public VGenSystem {
  virtual void init_objects(void);
};

void VSysBrownDwarf::init_objects(void) {
  VGenSystem::init_objects();
  game->add(new VBrownDwarf());
}

class VSysBinRedDwarf : public VGenSystem {
  virtual void init_objects(void);
};

void VSysBinRedDwarf::init_objects(void) {
  int i;
  VRedDwarf* RD1;
  VRedDwarf* RD2;
  VGenSystem::init_objects();
  RD1 = new VRedDwarf();
  RD2 = new VRedDwarf();
  game->add(RD1);
  game->add(RD2);
  this->setupBinary(RD1, RD2, scale_range(5), scale_range(5), scale_velocity(10));
  for (i=0; i<3; i++)
    game->add(new VMetalShard());
  for(i=0; i<1; i++) 
    game->add(new VMetalAsteroid());
  for(i=0; i<2; i++)
    game->add(new VSmallAsteroid());
  for(i=0;i<2; i++)
    game->add(new VLargeAsteroid());

}


class VSysBinRedGiantNeutron : public VGenSystem {
  virtual void init_objects(void);
};

void VSysBinRedGiantNeutron::init_objects(void) {
  int i;
  VRedGiant* RG1;
  VNeutronStar* NS2;
  VGenSystem::init_objects();
  RG1 = new VRedGiant();
  NS2 = new VNeutronStar();
  game->add(RG1);
  game->add(NS2);
  this->setupBinary(RG1, NS2, scale_range(5), scale_range(2.5), scale_velocity(25));
  for (i=0; i<3; i++)
    game->add(new VMetalShard());
  for(i=0; i<1; i++) 
    game->add(new VMetalAsteroid());
  for(i=0; i<3; i++)
    game->add(new VSmallAsteroid());
  for(i=0;i<3; i++)
    game->add(new VLargeAsteroid());
}

class VSysHypermass : public VGenSystem {
  virtual void init_objects(void);
};

void VSysHypermass::init_objects(void) {
  int i;
  VHypermass* H1;
  VGenSystem::init_objects();
  H1 = new VHypermass();
  game->add(H1);
  for (i=0; i<3; i++)
    game->add(new VMetalShard());
  for(i=0; i<1; i++) 
    game->add(new VMetalAsteroid());
  for(i=0; i<3; i++)
    game->add(new VSmallAsteroid());
  for(i=0;i<3; i++)
    game->add(new VLargeAsteroid());
}

class VSysTartarus : public VGenSystem {
  virtual void init_objects(void);
};

void VSysTartarus::init_objects(void) {
  int i;
  VHypermass* H1;  
  VHypermass* H2;
  VBlueGiant* BG;
  VGenSystem::init_objects();
  H1 = new VHypermass();
  H2 = new VHypermass();
  BG = new VBlueGiant();
  game->add(H1);
  game->add(H2);
  this->setupBinary(H1, H2, scale_range(3), scale_range(3), scale_velocity(15));
  game->add(BG);
  BG->pos = Vector2(scale_range(45), scale_range(45));
  for (i=0; i<6; i++)
    game->add(new VMetalShard());
  for(i=0; i<6; i++) 
    game->add(new VMetalAsteroid());
  for(i=0; i<3; i++)
    game->add(new VSmallAsteroid());
  for(i=0;i<2; i++)
    game->add(new VLargeAsteroid());
  for(i=0;i<3; i++)
    game->add(new VDustCloud());
  for(i=0;i<2; i++)
    game->add(new VGasCloud());
}

class VSysSmallAsteroids : public VGenSystem {
  virtual void init_objects(void);
};

void VSysSmallAsteroids::init_objects(void) {
  int i;
  VGenSystem::init_objects();
  for(i=0; i<35; i++)
    game->add(new VSmallAsteroid());
}

class VSysLargeAsteroids : public VGenSystem {
  virtual void init_objects(void);
};

void VSysLargeAsteroids::init_objects(void) {
  int i;
  VGenSystem::init_objects();
  for(i=0;i<30; i++)
    game->add(new VLargeAsteroid());
}

class VSysHeavyAsteroids : public VGenSystem {
  virtual void init_objects(void);
};

void VSysHeavyAsteroids::init_objects(void) {
  int i;
  VGenSystem::init_objects();
  for (i=0; i<4; i++)
    game->add(new VMetalShard());
  for(i=0; i<4; i++) 
    game->add(new VMetalAsteroid());
  for(i=0; i<30; i++)
    game->add(new VSmallAsteroid());
  for(i=0;i<15; i++)
    game->add(new VLargeAsteroid());
}

class VSysExtremeAsteroids : public VGenSystem {
  virtual void init_objects(void);
};

void VSysExtremeAsteroids::init_objects(void) {
  int i;
  VGenSystem::init_objects();
  for (i=0; i<5; i++)
    game->add(new VMetalShard());
  for(i=0; i<5; i++) 
    game->add(new VMetalAsteroid());
  for(i=0; i<50; i++)
    game->add(new VSmallAsteroid());
  for(i=0;i<30; i++)
    game->add(new VLargeAsteroid());
}

class VSysMetallicAsteroids : public VGenSystem {
  virtual void init_objects(void);
};

void VSysMetallicAsteroids::init_objects(void) {
  int i;
  VGenSystem::init_objects();
  for(i=0; i<30; i++) 
    game->add(new VMetalAsteroid());
}




void VGenSystem::setupBinary(SpaceObject* S1, SpaceObject* S2, double radius1, double radius2, double speed1) {
  OrbitHandler* OH;
  double speed2;
  speed2 = speed1 * (radius2 / radius1);

  //S1->pos = Vector2(0, radius1);
  OH = new OrbitHandler(mapCenter, Vector2(0, 0),
        0, mapCenter, S1, radius1, speed1, 1);
  game->add(OH);
  //S2->pos = Vector2(0, -radius2);
  OH = new OrbitHandler(mapCenter, Vector2(0, 0),
        PI, mapCenter, S2, radius2, speed2, 1);
  game->add(OH);
  return;
}

void VGenSystem::setupSprites(void) {
  //GasCloudSpr = new(**SpaceSprite);
	if(GetSpriteTo64Rot(AstSpr1,"vobject.dat","Asteroid000",1,-1)==FALSE)
		error("File error, things pictures.  Bailing out...");
	if(GetSpriteTo64Rot(AstSpr2,"vobject.dat","Asteroid001",1,-1)==FALSE)
		error("File error, things pictures.  Bailing out...");
	if(GetSpriteTo64Rot(AstSpr3,"vobject.dat","Asteroid002",1,-1)==FALSE)
		error("File error, things pictures.  Bailing out...");
	if(GetSpriteTo64Rot(AstSpr4,"vobject.dat","Asteroid003",1,-1)==FALSE)
		error("File error, things pictures.  Bailing out...");
	if(GetSpriteTo64Rot(Flare1,"vobject.dat","Flare000",1,-1)==FALSE)
		error("File error, things pictures.  Bailing out...");
	if(GetSpriteTo64Rot(Flare2,"vobject.dat","Flare001",1,-1)==FALSE)
		error("File error, things pictures.  Bailing out...");
	if(GetSpriteTo64Rot(Flare3,"vobject.dat","Flare002",1,-1)==FALSE)
		error("File error, things pictures.  Bailing out...");
	if(GetSpriteTo64Rot(Flare4,"vobject.dat","Flare003",1,-1)==FALSE)
		error("File error, things pictures.  Bailing out...");

  DefSatSpr = GetSprite("vobject.dat", "DefSat001", -1);
  SpaceOutpostSpr = GetSprite("vobject.dat", "SpaceOutpost001", -1);
  SpaceColonySpr = GetSprite("vobject.dat", "SpaceColony001", -1);
  SpaceStationSpr = GetSprite("vobject.dat", "SpaceStation001", -1);

  DustCloudSpr = GetSprite("vobject.dat", "Cloud001", -1);
  GasCloudSpr[0] = GetSprite("vobject.dat", "Cloud002A", -1);
  GasCloudSpr[1] = GetSprite("vobject.dat", "Cloud002B", -1);
  SpaceMineSpr = GetSprite("vobject.dat", "SpaceMine001", -1);
  SpaceMineExplosionSpr = GetMultiframeSprite("vobject.dat", "SpaceMineBoom001", -1, 6);

  PlanetMoon = GetSprite("vobject.dat", "Planet000", -1);
  PlanetRockball = GetSprite("vobject.dat", "Planet001", -1);
  PlanetInhospitable = GetSprite("vobject.dat", "Planet002", -1);
  PlanetHabitable = GetSprite("vobject.dat", "Planet003", -1);
  PlanetGasGiant = GetSprite("vobject.dat", "Planet004", -1);

  BrownDwarfSpr = GetSprite("vobject.dat", "Star000", -1);
  RedDwarfSpr = GetSprite("vobject.dat", "Star001", -1);
  WhiteDwarfSpr = GetSprite("vobject.dat", "Star002", -1);
  RedStarSpr = GetSprite("vobject.dat", "Star003", -1);
  OrangeStarSpr = GetSprite("vobject.dat", "Star004", -1);
  YellowStarSpr = GetSprite("vobject.dat", "Star005", -1);
  WhiteStarSpr = GetSprite("vobject.dat", "Star006", -1);
  RedGiantSpr = GetSprite("vobject.dat", "Star007", -1);
  BlueGiantSpr = GetSprite("vobject.dat", "Star008", -1);

	if(GetSpriteTo64Rot(NeutronStarSpr,"vobject.dat","Star009",1,-1)==FALSE)
		error("File error, things pictures.  Bailing out...");
	if(GetSpriteTo64Rot(BlackHoleSpr,"vobject.dat","Star010",1,-1)==FALSE)
		error("File error, things pictures.  Bailing out...");

  VMetalShard::SetMySprite(AstSpr1[0]);
  VMetalAsteroid::SetMySprite(AstSpr2[0]);
  VSmallAsteroid::SetMySprite(AstSpr3[0]);
  VLargeAsteroid::SetMySprite(AstSpr4[0]);
  VRedFlare::SetMySprite(Flare1[0]);
  VOrangeFlare::SetMySprite(Flare2[0]);
  VYellowFlare::SetMySprite(Flare3[0]);
  VWhiteFlare::SetMySprite(Flare4[0]);
  VDustCloud::SetMySprite(DustCloudSpr);
  VGasCloud::SetMySprite1(GasCloudSpr[0]);
  VGasCloud::SetMySprite2(GasCloudSpr[1]);
  VMoon::SetMySprite(PlanetMoon);
  VRockballPlanet::SetMySprite(PlanetRockball);
  VInhospitablePlanet::SetMySprite(PlanetInhospitable);
  VHabitablePlanet::SetMySprite(PlanetHabitable);
  VGasGiant::SetMySprite(PlanetGasGiant);
  VBrownDwarf::SetMySprite(BrownDwarfSpr);
  VRedDwarf::SetMySprite(RedDwarfSpr);
  VWhiteDwarf::SetMySprite(WhiteDwarfSpr);
  VRedStar::SetMySprite(RedStarSpr);
  VOrangeStar::SetMySprite(OrangeStarSpr);
  VYellowStar::SetMySprite(YellowStarSpr);
  VWhiteStar::SetMySprite(WhiteStarSpr);
  VRedGiant::SetMySprite(RedGiantSpr);
  VBlueGiant::SetMySprite(BlueGiantSpr);
  VNeutronStar::SetMySprite(NeutronStarSpr[0]);
  VHypermass::SetMySprite(BlackHoleSpr[0]);
  VSpaceMine::SetMySprite(SpaceMineSpr, SpaceMineExplosionSpr);
  VDefSat::SetMySprite(DefSatSpr);
  VDeepSpaceOutpost::SetMySprite(SpaceOutpostSpr);
  VDeepSpaceColony::SetMySprite(SpaceColonySpr);
  VDeepSpaceStation::SetMySprite(SpaceStationSpr);
}

void VGenSystem::init_objects() {
	size *= 1;
	prepare();
  VGenSystem::setupSprites();
  mapCenter = new SpaceLocation(NULL, Vector2(0,0), 0);
  add(new Stars());
  VMetalShard::InitStatics();
  VSmallAsteroid::InitStatics();
  VLargeAsteroid::InitStatics();
  VMetalAsteroid::InitStatics();
}

bool VGenSystem::GetSpriteTo64Rot(SpaceSprite *Pics[], char *fileName, char *cmdStr, 
int numSprites, int attribs)
{

	SpaceSprite *spr;
	char dataStr[100];

	for(int num=0; num<numSprites; num++)
	{
		sprintf(dataStr,cmdStr,num);
		spr=GetSprite(fileName, dataStr, attribs, 64);
		if(!spr)
		{
			return FALSE;
		}
		Pics[num]=spr;
	}
	return TRUE;
}


  SpaceSprite *VGenSystem::GetSprite(char *fileName, char *spriteName, int attribs, int rotations, int numSprites)
{

  //DATAFILE *dfile;
	DATAFILE *tmpdata;
  //dfile = load_datafile(fileName);
	tmpdata= load_datafile_object(fileName,spriteName);
  //tmpdata = find_datafile_object(dfile, spriteName);
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
  //note, important line below!!!!!
	SpaceSprite *spr=new SpaceSprite(tmpdata, numSprites, attribs, rotations);
	unload_datafile_object(tmpdata);
  //unload_datafile(dfile);

	return spr;
}

  SpaceSprite *VGenSystem::GetMultiframeSprite(char *fileName, char *spriteName, int attribs, int numberOfFrames)
{

  DATAFILE *dfile;
	DATAFILE *tmpdata;
  dfile = load_datafile(fileName);
	//tmpdata= load_datafile_object(fileName,spriteName);
  tmpdata = find_datafile_object(dfile, spriteName);
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
  //note, important line below!!!!!
	SpaceSprite *spr=new SpaceSprite(tmpdata, numberOfFrames, attribs, 1);
	//unload_datafile_object(tmpdata);
  unload_datafile(dfile);

	return spr;
}



bool VGenSystem::GetSpriteGroup(SpaceSprite *Pics[], char *fileName, char *cmdStr, 
int numSprites, int attribs, int firstSpriteNumber)
{

	SpaceSprite *spr;
	char dataStr[100];

	for(int num=firstSpriteNumber; num<(numSprites+firstSpriteNumber); num++)
	{
		sprintf(dataStr,cmdStr,num);
    message.print(500,5,dataStr);
		spr=GetSprite(fileName, dataStr, attribs, 1);
		if(!spr)
		{
			return FALSE;
		}
		Pics[num]=spr;
	}
	return TRUE;
}


VGenSystem::~VGenSystem(void) {
  int i;
  //NormalGame::~NormalGame();
  for(i=0; i<64; i++) {
    delete AstSpr1[i];
    delete AstSpr2[i];
    delete AstSpr3[i];
    delete AstSpr4[i];
    delete Flare1[i];
    delete Flare2[i];
    delete Flare3[i];
    delete Flare4[i];
    delete NeutronStarSpr[i];
    delete BlackHoleSpr[i];
  }
  delete PlanetMoon;
  delete PlanetHabitable;
  delete PlanetInhospitable;
  delete PlanetRockball;
  delete PlanetGasGiant;

  delete BrownDwarfSpr;
  delete RedDwarfSpr;
  delete WhiteDwarfSpr;
  delete RedStarSpr;
  delete OrangeStarSpr;
  delete YellowStarSpr;
  delete WhiteStarSpr;
  delete RedGiantSpr;
  delete BlueGiantSpr;
}
/*
REGISTER_GAME ( VSysVoid, "V-Void");
REGISTER_GAME ( VSysDusty, "V-Dusty");
REGISTER_GAME ( VSysGassy, "V-Gas Clouds");
REGISTER_GAME ( VSysNebula, "V-Nebula");
REGISTER_GAME ( VMineField, "V-Minefield");
REGISTER_GAME ( VSpaceStationGame, "V-Space Station");
REGISTER_GAME ( VDefendedPlanet, "V-Defended Planet");
REGISTER_GAME ( VImperialCapitol, "V-ImperialCapitol");
REGISTER_GAME ( VAncientBattlefield, "V-AncientBattlefield");
REGISTER_GAME ( VSysBrownDwarf, "V-BrownDwarf");
REGISTER_GAME ( VSysWhiteDwarf, "V-WhiteDwarf");
REGISTER_GAME ( VSysWhiteStar, "V-WhiteStar");
REGISTER_GAME ( VSysBlueGiant, "V-BlueGiant");
REGISTER_GAME ( VSysYellowThreePlanets, "V-Yellow-P3");
REGISTER_GAME ( VSysSmallAsteroids, "V-Small Asteroids");
REGISTER_GAME ( VSysLargeAsteroids, "V-Large Asteroids");
REGISTER_GAME ( VSysHeavyAsteroids, "V-Heavy Asteroids");
REGISTER_GAME ( VSysExtremeAsteroids, "V-Extreme Asteroids");
REGISTER_GAME ( VSysMetallicAsteroids, "V-Metallic Asteroids");
REGISTER_GAME ( VSysBinRedGiantNeutron, "V-Red Giant/Neutron star binary");
REGISTER_GAME ( VSysBinRedDwarf, "V-Binary Red Dwarf");
REGISTER_GAME ( VSysHypermass, "V-Hypermass (Black hole)");
REGISTER_GAME ( VSysTartarus, "V-Tartarus (Space Hell)");
*/

