/* $Id$ */ 
#ifndef __VTARGET__
#define __VTARGET__

//this file should be placed in the /other directory
class AimSys {
  public:
  double pursuitAngle;
  SpaceLocation* source;
  SpaceLocation* target;
  double weaponOffsetX, weaponOffsetY;
  double weaponAngle;
  double relativity;
  double degTolerance;
  double lagSetting;
  double maxRange;
  double nx1, ny1, nx2, ny2; //nx1 ny1 for firing ship.
  double nx1center, ny1center;
  double alphaDegree, thetaDegree, alphaRadian, thetaRadian;
  double alphaDegOld;
  double vx1, vy1, vx2, vy2; //X,Y velocities vx/y1 for firing ship, vx/y2 for target.
  double vxRel, vyRel, vRel;// the relativistic vector
  double weaponSpeed, targetSpeed, relSpeed, distance;
  double RawDistance(double x1, double y1, double x2, double y2);
  double vectorAngle, vectorAngleRadians, beta;
  double relVectorAngle, relVectorAngleRadians;
  double bestTrialAlpha, bestTrialTheta, bestTrialEpsilon;
  double bestTrialBeta, bestTrialBetaPrime;
  double trialDistance[9];
  double trialShotDistance[9];
  double trialAlpha[9];
  double trialAlphaRad[9];
  double trialBeta[9];// used for rel adjust
  double trialTheta[9]; 
  double trialEpsilon[9];
  double trialAlphaPrime[9];
  double trialBetaPrime[9]; // used for rel adjust
  double trialThetaPrime[9];
  double trialSinAlpha[9];
  double X2[9]; double Y2[9];
  double gamma;
  
  void SetupDefaults(void);
  AimSys(SpaceLocation* sourceP, 
			   SpaceLocation* targetP, 
			   double weaponSpeed, double Relativity,
			   double WeaponOffsetX, 
			   double WeaponOffsetY, double WeaponAngle,
			   double MaxRange,
			   double degTolerance, int lagSetting);
  AimSys(SpaceLocation* source, SpaceLocation* target, double weaponSpeed, double Relativity);
  AimSys(SpaceLocation* sourceP, SpaceLocation* targetP, double weaponSpeed);
  AimSys(SpaceLocation* sourceP, SpaceLocation* targetP);
  double CalcVectorAngle(double x1, double y1, double x2, double y2);
  double CalcVectorAngle(void);
  int CalcTrialValues(void);
  void Update(void);
  void setNewTarget(SpaceObject* newTarget);
  double getPursuitAngle(void);
  double getAngleOfShortestDistance(void);
  double getBestTrialAlpha(void);
  double getBestTrialTheta(void);
  int shouldFireNow(void);
  int shouldTurnLeft(void);
  int shouldTurnRight(void);
  double absAngleDifference(double A1, double A2);
  void CalcWeaponOffset(void);
  void CalcWeaponOffset(double xOff, double yOff);
  void SetSpaceLine(SpaceLine* SL);


};


#endif // __VTARGET__

