#include <allegro.h>
#include <stdio.h>

#include "melee.h"
REGISTER_FILE
#include "melee/mframe.h"
#include "melee/mgame.h"
#include "melee/mmain.h"
#include "melee/mcbodies.h"
#include "melee/mview.h"

#include "twgui/twgui.h"
#include "twgui/twmenuexamples.h"

#include "melee/mlog.h"

#ifndef _V_BODIES_H
#include "other/vbodies.h"
#endif

class VGenSystem : public NormalGame {
public:
  SpaceSprite* AstSpr1;
  SpaceSprite* AstSpr2;
  SpaceSprite* AstSpr3;
  SpaceSprite* AstSpr4;
  SpaceSprite* PlanetMoon;
  SpaceSprite* PlanetHabitable;
  SpaceSprite* PlanetInhospitable;
  SpaceSprite* PlanetRockball;
  SpaceSprite* PlanetGasGiant;
  SpaceSprite* Flare1;
  SpaceSprite* Flare2;
  SpaceSprite* Flare3;
  SpaceSprite* Flare4;
  SpaceSprite* BrownDwarfSpr;
  SpaceSprite* RedDwarfSpr;
  SpaceSprite* WhiteDwarfSpr;
  SpaceSprite* RedStarSpr;
  SpaceSprite* OrangeStarSpr;
  SpaceSprite* YellowStarSpr;
  SpaceSprite* WhiteStarSpr;
  SpaceSprite* RedGiantSpr;
  SpaceSprite* BlueGiantSpr;
  SpaceSprite* NeutronStarSpr;
  SpaceSprite* BlackHoleSpr;
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

  bool GetSpriteTo64Rot(SpaceSprite *&Pics, char *fileName, char *cmdStr, 
    int numSprites = 1, int attribs = -1);

  SpaceSprite* GetSprite(char *fileName, char *spriteName, int attribs, int rotations=1, int numSprites = 1);

  bool GetSpriteGroup(SpaceSprite *Pics[], char *fileName, char *cmdStr, int numSprites, int attribs=-1, int firstSpriteNumber=1);

  SpaceSprite* GetMultiframeSprite(char *fileName, char *spriteName, int attribs, int numberOfFrames);

  virtual ~VGenSystem();


	void init_objectsVAncientBattlefield();
	void init_objectsVSpaceStationGame();
	void init_objectsVImperialCapitol();
	void init_objectsVDefendedPlanet();
	void init_objectsVMineField();
	void init_objectsVSysNebula();
	void init_objectsVSysDusty();
	void init_objectsVSysGassy();
	void init_objectsVSysBlueGiant();
	void init_objectsVSysWhiteDwarf();
	void init_objectsVSysWhiteStar();
	void init_objectsVSysYellowThreePlanets();
	void init_objectsVSysVoid();
	void init_objectsVSysBrownDwarf();
	void init_objectsVSysBinRedDwarf();
	void init_objectsVSysBinRedGiantNeutron();
	void init_objectsVSysHypermass();
	void init_objectsVSysTartarus();
	void init_objectsVSysSmallAsteroids();
	void init_objectsVSysLargeAsteroids();
	void init_objectsVSysHeavyAsteroids();
	void init_objectsVSysExtremeAsteroids();
	void init_objectsVSysMetallicAsteroids();

	typedef void (VGenSystem::* type_func) ();
	type_func	funclist[32];
	char		*functitle[32];
	int Ninit;
	void register_init(type_func f, char *description);
};



void VGenSystem::init_objectsVAncientBattlefield() {
  int i;
  //VGenSystem::init_objects();
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



void VGenSystem::init_objectsVSpaceStationGame() {
  VDeepSpaceStation* DSS;
  //VGenSystem::init_objects();
  DSS = new VDeepSpaceStation();
  game->add(DSS);
  game->add(new VLargeAsteroid());
  DSS->AddInstallation(new VGroundDefenseLaser(DSS));
  DSS->AddInstallation(new VGroundMissileLauncher(DSS));
  DSS->AddInstallation(new VGroundIonCannon(DSS));
}



void VGenSystem::init_objectsVImperialCapitol() {
  VHabitablePlanet* VHP;
  VGroundDefenseLaser *GDL;
  //VGenSystem::init_objects();
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



void VGenSystem::init_objectsVDefendedPlanet(void) {
  VRockballPlanet* VRP;
  //VGenSystem::init_objects();
  VRP = new VRockballPlanet();
  game->add(VRP);
  game->add(new VLargeAsteroid());
  VRP->AddInstallation(new VGroundDefenseLaser(VRP));
  VRP->AddInstallation(new VGroundMissileLauncher(VRP));
  VRP->AddInstallation(new VGroundIonCannon(VRP));
}



void VGenSystem::init_objectsVMineField(void) {
  int i;
  VSpaceMine* VSM;
  //VGenSystem::init_objects();
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



void VGenSystem::init_objectsVSysNebula(void) {
  game->add(new VNebula());
  //VGenSystem::init_objects();
  game->add(new VSmallAsteroid());
  game->add(new VLargeAsteroid());
  game->add(new VMetalAsteroid());
}



void VGenSystem::init_objectsVSysDusty(void) {
  //VGenSystem::init_objects();
  for(int i=0; i<33; i++)
    game->add(new VDustCloud());
}



void VGenSystem::init_objectsVSysGassy(void) {
  //VGenSystem::init_objects();
  for(int i=0; i<33; i++)
    game->add(new VGasCloud());
}




void VGenSystem::init_objectsVSysBlueGiant(void) {
  //VGenSystem::init_objects();
  game->add(new VBlueGiant());
}



void VGenSystem::init_objectsVSysWhiteDwarf(void) {
  //VGenSystem::init_objects();
  game->add(new VWhiteDwarf());
}



void VGenSystem::init_objectsVSysWhiteStar(void) {
  //VGenSystem::init_objects();
  game->add(new VWhiteStar());
}




void VGenSystem::init_objectsVSysYellowThreePlanets(void) {
  int i;
  VYellowStar* YS;
  VRockballPlanet* RP;
  VHabitablePlanet* HP;
  VGasGiant* GG;
  VMoon* M1;
  VMoon* M2;
  VMoon* M3;
  OrbitHandler* OH;
  //VGenSystem::init_objects();
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



void VGenSystem :: init_objectsVSysVoid(void) {
  //VGenSystem::init_objects();
}




void VGenSystem::init_objectsVSysBrownDwarf(void) {
  //VGenSystem::init_objects();
  game->add(new VBrownDwarf());
}



void VGenSystem::init_objectsVSysBinRedDwarf(void) {
  int i;
  VRedDwarf* RD1;
  VRedDwarf* RD2;
  //VGenSystem::init_objects();
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




void VGenSystem::init_objectsVSysBinRedGiantNeutron(void) {
  int i;
  VRedGiant* RG1;
  VNeutronStar* NS2;
  //VGenSystem::init_objects();
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



void VGenSystem::init_objectsVSysHypermass(void) {
  int i;
  VHypermass* H1;
  //VGenSystem::init_objects();
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



void VGenSystem::init_objectsVSysTartarus(void) {
  int i;
  VHypermass* H1;  
  VHypermass* H2;
  VBlueGiant* BG;
  //VGenSystem::init_objects();
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


void VGenSystem::init_objectsVSysSmallAsteroids(void) {
  int i;
  //VGenSystem::init_objects();
  for(i=0; i<35; i++)
    game->add(new VSmallAsteroid());
}


void VGenSystem::init_objectsVSysLargeAsteroids(void) {
  int i;
  //VGenSystem::init_objects();
  for(i=0;i<30; i++)
    game->add(new VLargeAsteroid());
}


void VGenSystem::init_objectsVSysHeavyAsteroids(void) {
  int i;
  //VGenSystem::init_objects();
  for (i=0; i<4; i++)
    game->add(new VMetalShard());
  for(i=0; i<4; i++) 
    game->add(new VMetalAsteroid());
  for(i=0; i<30; i++)
    game->add(new VSmallAsteroid());
  for(i=0;i<15; i++)
    game->add(new VLargeAsteroid());
}


void VGenSystem::init_objectsVSysExtremeAsteroids(void) {
  int i;
  //VGenSystem::init_objects();
  for (i=0; i<5; i++)
    game->add(new VMetalShard());
  for(i=0; i<5; i++) 
    game->add(new VMetalAsteroid());
  for(i=0; i<50; i++)
    game->add(new VSmallAsteroid());
  for(i=0;i<30; i++)
    game->add(new VLargeAsteroid());
}


void VGenSystem::init_objectsVSysMetallicAsteroids(void) {
  int i;
  //VGenSystem::init_objects();
  for(i=0; i<30; i++) 
    game->add(new VMetalAsteroid());
}





// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------




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

  VMetalShard::SetMySprite(AstSpr1);
  VMetalAsteroid::SetMySprite(AstSpr2);
  VSmallAsteroid::SetMySprite(AstSpr3);
  VLargeAsteroid::SetMySprite(AstSpr4);
  VRedFlare::SetMySprite(Flare1);
  VOrangeFlare::SetMySprite(Flare2);
  VYellowFlare::SetMySprite(Flare3);
  VWhiteFlare::SetMySprite(Flare4);
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
  VNeutronStar::SetMySprite(NeutronStarSpr);
  VHypermass::SetMySprite(BlackHoleSpr);
  VSpaceMine::SetMySprite(SpaceMineSpr, SpaceMineExplosionSpr);
  VDefSat::SetMySprite(DefSatSpr);
  VDeepSpaceOutpost::SetMySprite(SpaceOutpostSpr);
  VDeepSpaceColony::SetMySprite(SpaceColonySpr);
  VDeepSpaceStation::SetMySprite(SpaceStationSpr);
}


bool VGenSystem::GetSpriteTo64Rot(SpaceSprite *&Pics, char *fileName, char *cmdStr, 
int numSprites, int attribs)
{

	SpaceSprite *spr;
	char dataStr[100];

	for(int num=0; num<numSprites; num++)
	{
		sprintf(dataStr,cmdStr,num);
		spr = GetSprite(fileName, dataStr, attribs, 64);
		if(!spr)
		{
			return FALSE;
		}
		Pics = spr;
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
 // int i;
  //NormalGame::~NormalGame();
  //for(i=0; i<64; i++) {
    delete AstSpr1;
    delete AstSpr2;
    delete AstSpr3;
    delete AstSpr4;
    delete Flare1;
    delete Flare2;
    delete Flare3;
    delete Flare4;
    delete NeutronStarSpr;
    delete BlackHoleSpr;
  //}
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




class GeneralMelee : public NormalGame {
  virtual void init_objects(void);
};

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




void VGenSystem::register_init(type_func f, char *description)
{
	funclist[Ninit] = f;
	functitle[Ninit] = description;
	++Ninit;
}


void VGenSystem::init_objects() {
	size *= 1;
	prepare();

	// interface, and init-objects using the interface ...
	Ninit = 0;
	register_init(&VGenSystem::init_objectsVAncientBattlefield, "Ancient battlefield");
	register_init(&VGenSystem::init_objectsVSpaceStationGame, "Station");
	register_init(&VGenSystem::init_objectsVImperialCapitol, "Imperial capital");
	register_init(&VGenSystem::init_objectsVDefendedPlanet, "Defended planet");
	register_init(&VGenSystem::init_objectsVMineField, "Mine field");
	register_init(&VGenSystem::init_objectsVSysNebula, "Nebula");
	register_init(&VGenSystem::init_objectsVSysDusty, "Dusty");
	register_init(&VGenSystem::init_objectsVSysGassy, "Gassy");
	register_init(&VGenSystem::init_objectsVSysBlueGiant, "Blue giant");
	register_init(&VGenSystem::init_objectsVSysWhiteDwarf, "White dwarf");
	register_init(&VGenSystem::init_objectsVSysWhiteStar, "White star");
	register_init(&VGenSystem::init_objectsVSysYellowThreePlanets, "Yellow 3 planets");
	register_init(&VGenSystem::init_objectsVSysVoid, "Void");
	register_init(&VGenSystem::init_objectsVSysBrownDwarf, "Brown dwarf");
	register_init(&VGenSystem::init_objectsVSysBinRedDwarf, "Red dwarf");
	register_init(&VGenSystem::init_objectsVSysBinRedGiantNeutron, "Red giant neutron");
	register_init(&VGenSystem::init_objectsVSysHypermass, "Hypermass");
	register_init(&VGenSystem::init_objectsVSysTartarus, "Tartarus");
	register_init(&VGenSystem::init_objectsVSysSmallAsteroids, "Small asteroids");
	register_init(&VGenSystem::init_objectsVSysLargeAsteroids, "Large asteroids");
	register_init(&VGenSystem::init_objectsVSysHeavyAsteroids, "Heavy asteroids");
	register_init(&VGenSystem::init_objectsVSysExtremeAsteroids, "Extreme asteroids");
	register_init(&VGenSystem::init_objectsVSysMetallicAsteroids, "Metallic asteroids");

	//int i = random(Ninit);

	int i;

	// initialization is for non-clients only ...
	if (log->type == Log::log_net1server || log->type == Log::log_normal)
	{
		
		PopupList *popupl;
		popupl = new PopupList(screen, "interfaces/gametest/popuplist", "text/", 0, 0, videosystem.get_font(2), 0);
		popupl->tbl->set_optionlist(functitle, Ninit, makecol(255,255,128));
		popupl->xshift = 0;
		popupl->yshift = 0;
		popupl->close_on_defocus = false;
		
		popupl->tree_doneinit();
		popupl->show();

		//WindowManager *winman;
		//winman = new WindowManager;
		//winman->add(popupl);
		
		popupl->tree_setscreen(screen);
		popupl->center();

		show_mouse(screen);
		unscare_mouse();
		
		while (!popupl->returnvalueready)
		{
			idle(10);
			
			popupl->tree_calculate();

			//clear_to_color(screen, 0);
			popupl->tree_animate();
		}
		
		i = popupl->returnstatus;

	}
	
	// share the result of the initialization.
	// send (or receive) ... channel_server is locally either the server, or the client.
	log_int(channel_server, i);


	VGenSystem::setupSprites();
	mapCenter = new SpaceLocation(NULL, Vector2(0,0), 0);
	add(new Stars());
	VMetalShard::InitStatics();
	VSmallAsteroid::InitStatics();
	VLargeAsteroid::InitStatics();
	VMetalAsteroid::InitStatics();


	// call the selected init routine :
	(this->*funclist[i])();

}


REGISTER_GAME(VGenSystem, "Varith Systems")
