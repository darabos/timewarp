
#ifndef __GMISSION_OBJECTS_H__
#define __GMISSION_OBJECTS_H__


SpaceObject *find_closest_team(SpaceLocation *s, int test_attr, double R, TeamCode team);


SpaceSprite *gensprite(char *datafilename, char *dataobjectname, char *ident);

class MissionObject : public SpaceObject
{
public:
	MissionObject(Vector2 opos, char *datafilename, char *dataobjectname);

};


class MissionShip : public Ship
{
public:
	MissionShip(Vector2 opos, char *datafilename, char *dataobjectname,
		char *ininame, char *sectionname, TeamCode team);

	virtual void calculate();
	virtual void missioncontrol();	// the intelligence controlling this vessel

};


class SpaceLineVoid : public SpaceLine
{
public:
	SpaceLineVoid(SpaceLocation *creator, Vector2 lpos, double langle, 
									double llength, int lcolor);
	void setline(Vector2 p1, Vector2 p2);
};


class SpecialArea : public SpaceLocation
{
public:

	double R;
	double rping;
	Vector2 center;

	SpaceLineVoid	*line[10];

	SpecialArea(Vector2 apos, double aR);

	bool inreach(SpaceLocation *s);

	virtual void animate(Frame *space);
};



class SpecialAreaTag : public SpecialArea
{
public:
	SpaceLocation *tagship;
	SpecialAreaTag(SpaceLocation *s, double aR);
	virtual void calculate();
	virtual void animate(Frame *space);
};


class AsteroidBelt : public Asteroid
{
public:
	double	R, Rmin, Rmax, v, v_unscaled;
	Vector2	center;

	AsteroidBelt(Vector2 ocenter, double oRmin, double oRmax, double ov);
	virtual void calculate();
	virtual void death();
	Vector2 correction();

	// overwrite this with 0, so that planets won't create some extra force which would
	// pull the asteroids out of their orbit.
	virtual int accelerate(SpaceLocation *source, double angle, double vel, double max_speed=MAX_SPEED); //accelerates an object by vel at angle, to a maximum of max_speed
	virtual int accelerate(SpaceLocation *source, Vector2 delta_v, double max_speed=MAX_SPEED); //changes an objects velocity by delta_v, to a maximum of max_speed
};



class AsteroidZone : public Asteroid
{
public:
	double	Rmax, v, v_unscaled;
	Vector2	center;
	AsteroidZone(Vector2 ocenter, double oRmax, double ov);
	virtual void calculate();
	virtual void death();
};



class MissionMessageBox : public Presence
{
public:
	double time, period;
	BITMAP *bmp_display;
	int xpos, ypos;

	MissionMessageBox(BITMAP *bmp, int oxpos, int oypos, double operiod);
	~MissionMessageBox();

	virtual void calculate();
	virtual void animate(Frame *space);
};


class MissionPointer : public WedgeIndicator
{
private:
	int hidden;
	SpaceLocation *missiontarget;

public:
	MissionPointer(SpaceLocation *otarget);
	void target(SpaceLocation *otarget);
	void hide();
	void show();
	virtual void calculate();
	virtual void animate(Frame *frame);
};



#endif // __GMISSION_OBJECTS_H__

