#include "../ship.h"
#include "../frame.h"
#include "vtarget.h"

#include "../melee.h"
REGISTER_FILE
#include "../id.h"

#include "../libs.h"

#include "../melee/mframe.h"
#include "../melee/mgame.h"
#include "../melee/manim.h"
#include "../melee/mview.h"
#include "../melee/mship.h"
#include "../melee/mcbodies.h"

//this file should be placed in the /other directory
//MAX_X is the critical constant to remember when
//dealing with aliasing!

/* this object assumes that all incoming values are scaled
(velocity, range, acceleration, etc.).  so, if using a raw
number, remember to call scale_velocity, scale_range, etc
on the values going into this object.  */

/*weaponOffsetX is for the X coord offset, for guns
that are not on the center.  Use negative values for left,
positive values for right.  Not yet implemented. (it's messy)

 weaponOffsetY is to adjust this if the gun is mounted forward
 or backward of the center.  Most guns are mounted forward
 of the center.  Use positive values for forward, negative
 values for backward.

weaponAngle is for weapons that do not fire straight forward
along the facing of the ship.

turrets can be done by making sourceP the turret, not the
firing ship.

degTolerance is the size of the window that will allow firing.
degTolerance of zero means a very narrow window is used.

lagSetting is T/F.  if false, it is set to err on the side of
anticipation.  If true, it is set to err on the side of lagging
behind.

Angles should always be passed back and forth as degrees.
sometimes radian angles are held, but they are for internal
routine use only, not for passing back and forth.

I'm trying to make sure that any variable which is an
angle holds a degree reading unless it ends with the word
Radian in the variable name.

*/
AimSys::AimSys(SpaceLocation* sourceP, 
			   SpaceLocation* targetP, 
			   double WeaponSpeed, double Relativity,
			   double WeaponOffsetX, 
			   double WeaponOffsetY, double WeaponAngle,
			   double MaxRange,
			   double DegTolerance, int LagSetting) {
	AimSys::SetupDefaults();
	source = sourceP; target = targetP;
	relativity = Relativity;
	nx1center = source->pos.x; ny1center = source->pos.y; 
	nx2 = target->pos.x; ny2 = target->pos.y;
	vx1 = source->vel.x; vy1 = source->vel.y;
	vx2 = target->vel.x; vy2 = target->vel.y;
	weaponSpeed = WeaponSpeed;
	targetSpeed = sqrt(vx2 * vx2 + vy2 * vy2);
	weaponOffsetX = WeaponOffsetX;
	weaponOffsetY = WeaponOffsetY;
	weaponAngle = WeaponAngle;
	degTolerance =DegTolerance;
	lagSetting = LagSetting;
  AimSys::CalcWeaponOffset();

}


AimSys::AimSys(SpaceLocation* sourceP, SpaceLocation* targetP, double WeaponSpeed, double Relativity) {
	//not anti-aliased here
	AimSys::SetupDefaults();
	source = sourceP; target = targetP;
	nx1center = source->pos.x; ny1center = source->pos.y; 
	nx2 = target->pos.x; ny2 = target->pos.y;
	vx1 = source->vel.x; vy1 = source->vel.y;
	vx2 = target->vel.x; vy2 = target->vel.y;
	weaponSpeed = WeaponSpeed;
	targetSpeed = sqrt(vx2 * vx2 + vy2 * vy2);
	relativity=Relativity;
  AimSys::CalcWeaponOffset();

}

AimSys::AimSys(SpaceLocation* sourceP, SpaceLocation* targetP, double WeaponSpeed) {
	//not anti-aliased here
	AimSys::SetupDefaults();
    source = sourceP; target = targetP;
	nx1center = source->pos.x; ny1center = source->pos.y; 
	nx2 = target->pos.x; ny2 = target->pos.y;
	vx1 = source->vel.x; vy1 = source->vel.y;
	vx2 = target->vel.x; vy2 = target->vel.y;
	weaponSpeed = WeaponSpeed;
	targetSpeed = sqrt(vx2 * vx2 + vy2 * vy2);
  AimSys::CalcWeaponOffset();
}

AimSys::AimSys(SpaceLocation* sourceP, SpaceLocation* targetP) {
	//not anti-aliased here
	//use with caution -- weapon speed is taken to be the
	//current ship speed.  Would work for the Tau Bomber.
	AimSys::SetupDefaults();
	source = sourceP; target = targetP;
	nx1center = source->pos.x; ny1center = source->pos.y; 
	nx2 = target->pos.x; ny2 = target->pos.y;
	vx1 = source->vel.x; vy1 = source->vel.y;
	vx2 = target->vel.x; vy2 = target->vel.y;
	weaponSpeed = sqrt(vx1 * vx1 + vy1 * vy1); // normally wrong!
	targetSpeed = sqrt(vx2 * vx2 + vy2 * vy2);
  AimSys::CalcWeaponOffset();
}

void AimSys::SetupDefaults(void) { //sets up defaults
	relativity = 0.5;
	source = NULL; target = NULL;
  nx1center = 0; ny1center = 0;
	nx1 = 0; ny1 = 0; nx2 = 0; ny2 = 0;
	vx1 = 0; vx2 = 0; vy1 = 0; vy2 = 0;
	weaponSpeed = scale_velocity(100); targetSpeed = 0;
	weaponOffsetX = 0; weaponOffsetY = 0; weaponAngle = 0;
	lagSetting = FALSE;
	maxRange = scale_range(50);
	degTolerance = 0.1;
}

void AimSys::CalcWeaponOffset(void) {
  
  double dx, dy;
	gamma = CalcVectorAngle(0,0,-weaponOffsetY, weaponOffsetX);
	dx = cos((gamma+source->angle) * PI / 180);
	dy = sin((gamma+source->angle) * PI / 180);
	nx1 = nx1center + weaponOffsetY * dx - weaponOffsetX * dy;
	ny1 = ny1center + weaponOffsetY * dy + weaponOffsetX * dx;
  //the above angle calculations are almost certainly mathematically
  //incorrect in x,y,sin,cosine assignments, but it works empirically
  //the errors must cancel out

}

void AimSys::CalcWeaponOffset(double xOff, double yOff) {
  weaponOffsetX = xOff;
  weaponOffsetY = yOff;
  AimSys::CalcWeaponOffset();
}

void AimSys::SetSpaceLine(SpaceLine* SL) {
  SL->pos.x = nx1;
  SL->pos.y = ny1;
  SL->angle = gamma+source->angle;
  //SL->length = scale_range(10);
  return;
}



double AimSys::RawDistance(double x1, double y1, double x2, double y2) {
	return sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2 - y1));
	//not anti-aliased here
}

double AimSys:: CalcVectorAngle(void) {
	return AimSys::CalcVectorAngle(nx1, ny1, nx2, ny2);
}


double AimSys:: CalcVectorAngle(double x1, double y1, double x2, double y2) {
	//angle of the line from source to target, in normal cartesian coordinates
	//zero degrees is going straight up.
	//the computer uses X+ across, Y+ down.
	//shouldn't really matter
	double dx, dy;
	dx = x2 - x1; dy = y2 - y1;
	if(dx==0 && dy==0) return(0.0);
	if(dx==0){
		if(dy>0) return(0.0);
		else return (180.0); }
	if(dy==0) {
		if(dx>0) return (90.0);
		else return (270.0); }
	if((dx>0)&&(dy>0)) // cartesian quadrant 1
		vectorAngleRadians = atan(dy/dx);
	if((dx<0)&&(dy>0)) // cartesian quadrant 2
        vectorAngleRadians = atan(dy/dx) + PI; //atan(dy/dx);
	if((dx<0)&&(dy<0)) // cartesian quadrant 3
		vectorAngleRadians = atan(dy/dx) + PI;
	if((dx>0)&&(dy<0)) // cartesian quadrant 4
		vectorAngleRadians = atan(dy/dx);
    vectorAngle = vectorAngleRadians / PI * 180;
    return(vectorAngle);
}

int AimSys::CalcTrialValues(void) {
	//HERE is where the anti-ailasing happens!
	//all 8 wrap-around positions are tested, along with the non
	//wrapped one.
	//This can potentially make the AimSys aware of long 'wrap around' shots.
	int i, j, k, best;
	double x;
	x = 9E30;
	for(i=-1;i<=1;i++) {
		for(j=-1;j<=1; j++){
			k = 4 + (3 * i) + j; //subscripts
			X2[k]=nx2 + i * map_size.x;
			Y2[k]=ny2 + j * map_size.y;
			trialEpsilon[k] = CalcVectorAngle(nx1, ny1, X2[k], Y2[k]);
			trialThetaPrime[k] = CalcVectorAngle(0,0, vx2, vy2);
			trialTheta[k] = trialThetaPrime[k]-trialEpsilon[k];
			trialDistance[k] = RawDistance(nx1, ny1, X2[k], Y2[k]);
			trialBetaPrime[k] = CalcVectorAngle(0,0,vx1, vy1);
			trialBeta[k] = trialBetaPrime[k] - trialEpsilon[k];
			//questionable line below:
			trialShotDistance[k]=trialDistance[k] * (1+targetSpeed/weaponSpeed);
			trialSinAlpha[k] = (targetSpeed * sin(trialTheta[k] * PI / 180) / weaponSpeed)
				- (relSpeed * sin(trialBeta[k] * PI / 180) / weaponSpeed);
            if(trialSinAlpha[k]>-1&&trialSinAlpha[k]<1)
				trialAlphaRad[k] = asin(trialSinAlpha[k]);
			else
				trialAlphaRad[k] = PI; //outside the normal range of the arcsin function
			trialAlpha[k] = trialAlphaRad[k] * 180 / PI;
            if(trialDistance[k]<x) {
				x = trialDistance[k];
				best = k; 
			}
		}
	}
	bestTrialEpsilon = trialEpsilon[best];
    pursuitAngle = trialAlpha[best] + trialEpsilon[best];
	bestTrialAlpha = trialAlpha[best];
	bestTrialTheta = trialTheta[best];
	bestTrialBeta = trialBeta[best];
	bestTrialBetaPrime = trialBetaPrime[best];

	//while(pursuitAngle>360) pursuitAngle -= 360;
	//while(pursuitAngle<0) pursuitAngle += 360;
	return(TRUE);
}

double AimSys::getPursuitAngle() {
	return pursuitAngle;
}

double AimSys::getBestTrialAlpha() {
	return bestTrialAlpha;
}

double AimSys::getBestTrialTheta() {
	return bestTrialTheta;
}

double AimSys::getAngleOfShortestDistance() {
  return bestTrialEpsilon;
}

void AimSys::setNewTarget(SpaceObject* newTarget) {
	target = newTarget;
}

void AimSys::Update(void) {
	if(source==NULL||target==NULL) return;
	nx1center = source->pos.x; ny1center = source->pos.y; 
	nx2 = target->pos.x; ny2 = target->pos.y;
	vx1 = source->vel.x; vy1 = source->vel.y;
	vx2 = target->vel.x; vy2 = target->vel.y;
	targetSpeed = sqrt(vx2 * vx2 + vy2 * vy2);
	vxRel = vx1 * relativity;
	vyRel = vy1 * relativity;
  AimSys::CalcWeaponOffset();

	vRel = sqrt(vxRel * vxRel + vyRel * vyRel);
	relSpeed = sqrt(vxRel * vxRel + vyRel * vyRel);
	AimSys::CalcTrialValues();
}

double AimSys::absAngleDifference(double A1, double A2) {
	double x;
	x = abs(A2 - A1);
	if(x>180)
		return(360 - x);
	else
		return x;
}


int AimSys::shouldFireNow(void) {
	// at the moment, just checks for current angle
	//within tolerance
	//the quick and dirty answer
	int i;
	int fire;
	fire = FALSE;
	for(i=0;i<9;i++) { //should be trialShotDistance below
		if((trialShotDistance[i]<maxRange) &&
			((absAngleDifference(source->angle-weaponAngle,trialAlpha[i] + trialEpsilon[i])<degTolerance) ||
			(absAngleDifference(source->angle-weaponAngle,trialAlpha[i] + trialEpsilon[i])>360-degTolerance)))
			fire=TRUE;
	} //angle crossing logic would happen now.
	
	return(fire); 
}

int AimSys::shouldTurnLeft(void) {
	int x1, x2;
	x1 = source->angle-weaponAngle;
	x2 = pursuitAngle;
	while(x2<x1) x2 +=360;
	if(x2-x1<1||x2-x1>359) return (FALSE); //it's really close now.
	if((x2-x1)<180)
		return(FALSE);
	else
		return(TRUE);
}

int AimSys::shouldTurnRight(void) {
	int x1, x2;
	x1 = source->angle-weaponAngle;
	x2 = pursuitAngle;
	while(x2<x1) x2 +=360;
	if(x2-x1<1||x2-x1>359) return (FALSE); //it's really close now.
	if((x2-x1)>180)
		return(FALSE);
	else
		return(TRUE);

}