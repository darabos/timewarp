/* $Id$ */ 
#ifndef __MSHOT_H__
#define __MSHOT_H__

#include "mframe.h"
#include "mgame.h"

/** shot - uses one image, independent of angle */
class Shot : public SpaceObject {
	IDENTITY(Shot);
	public:
	double v;
	double d;
	double range;
	double armour;
  
	SpaceSprite *explosionSprite;
	SAMPLE      *explosionSample;
	int          explosionFrameCount;
	int          explosionFrameSize;

	Shot(SpaceLocation *creator, Vector2 rpos, double oangle, double ov, double odamage,
			double orange, double oarmour, SpaceLocation *opos, SpaceSprite *osprite, double relativity = game->shot_relativity);

	virtual void animate_predict(Frame *space, int time);

	virtual void calculate();
	virtual int handle_damage(SpaceLocation *source, double normal, double direct);
	virtual void inflict_damage(SpaceObject *other);
	virtual void death();

	virtual void animateExplosion();
	virtual void soundExplosion();

	void stop();
	void destroy();

	virtual void changeDirection(double oangle);
	int isHomingMissile();

	virtual void scale_vel(double scale);
	};

/** animated shot - flies straight ahead, and cycles through images in the spacesprite */
class AnimatedShot : public Shot {
	IDENTITY(AnimatedShot);
	protected:
	int frame_count;
	int frame_size;
	int frame_step;

	public:
	AnimatedShot(SpaceLocation *creator, Vector2 rpos, double oangle, 
			double ov, double odamage, double orange, double oarmour, SpaceLocation *opos,
			SpaceSprite *osprite, int ofcount, int ofsize, double relativity = game->shot_relativity);

	virtual void calculate();
	};

/** missile - flies straight ahead */
class Missile : public Shot {
	IDENTITY(Missile);
	public:
	Missile(SpaceLocation *creator, Vector2 rpos, double oangle, double ov, double odamage,
			double orange, double oarmour, SpaceLocation *opos,
			SpaceSprite *osprite, double relativity = game->shot_relativity);

	virtual void changeDirection(double oangle);
//	virtual void animate_predict(Frame *space, int time);
	};

/** homing missile - flies to target. Uses indexed image from the spacesprite */
class HomingMissile : public Missile {
	IDENTITY(HomingMissile);
	protected:
	double turn_rate;
	double turn_step;

	public:
	HomingMissile(SpaceLocation *creator, Vector2 rpos, double oangle, double ov, double odamage,
			double orange, double oarmour, double otrate, SpaceLocation *opos,
			SpaceSprite *osprite, SpaceObject *target);

	virtual void animate_predict(Frame *space, int time);
	virtual void calculate();
	};

/** a laser */
class Laser : public SpaceLine {
	IDENTITY(Laser);
	protected:
	double frame;
	double frame_count;

	SpaceLocation *lpos;
	Vector2 rel_pos;
	double relative_angle;
	bool sinc_angle;

	public:
	Laser(SpaceLocation *creator, double langle, int lcolor, double lrange, double ldamage, int lfcount,
		SpaceLocation *opos, Vector2 rpos = Vector2(0,0), bool osinc_angle=false);

	void calculate();
};

/** laser that goes from source point to target point  */
class PointLaser : public Laser
{
	IDENTITY(PointLaser);
  protected:

  SpaceObject *target;

  public:
	PointLaser(SpaceLocation *creator, int lcolor, double ldamage, int lfcount, 
		SpaceLocation *lsource, SpaceObject *ltarget, Vector2 rel_pos = Vector2(0,0)) ;

  void calculate();
  int canCollide(SpaceObject *other);
};


/** A shot that expires after a specified time */
class TimedShot : public SpaceObject
{
public:
IDENTITY(TimedShot);
public:
	double armour;
	double existtime, maxtime;

	TimedShot(SpaceLocation *creator, Vector2 orelpos, double orelangle, SpaceSprite *osprite,
		double ovel, double otime, double oarmour, double odamage);

	virtual void calculate();

	virtual void inflict_damage(SpaceObject *other);
	virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};


#endif  // __MSHOT_H__
