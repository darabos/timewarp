#ifndef _MFRAME_H
#include "mframe.h"
#endif
#ifndef _MGAME_H
#include "mgame.h"
#endif

#ifndef _MSHOT_H
#define _MSHOT_H

class Shot : public SpaceObject {
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
	};

class AnimatedShot : public Shot {
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

class Missile : public Shot {
	public:
	Missile(SpaceLocation *creator, Vector2 rpos, double oangle, double ov, double odamage,
			double orange, double oarmour, SpaceLocation *opos,
			SpaceSprite *osprite, double relativity = game->shot_relativity);

	virtual void changeDirection(double oangle);
//	virtual void animate_predict(Frame *space, int time);
	};

class HomingMissile : public Missile {
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

class Laser : public SpaceLine {
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

class PointLaser : public Laser {
  protected:

  SpaceObject *target;

  public:
	PointLaser(SpaceLocation *creator, int lcolor, double ldamage, int lfcount, 
		SpaceLocation *lsource, SpaceObject *ltarget, Vector2 rel_pos = Vector2(0,0)) ;

  void calculate();
  int canCollide(SpaceObject *other);
};


#endif