/* $Id$ */ 
class HealthBar : public Presence
{
public:
	double	scale;
	int		*bartoggle;
	Ship	*mother;

	HealthBar(Ship *creator, int *toggle);
	void calculate();
	void animate(Frame *space);

	void draw_bar(Ship *s, double yoffs, int len, double H, double fraction, int col1, int col2, Frame *space);
};

class Indicator : public Presence {
	public:
	Indicator();
	bool coords(Frame *space, SpaceLocation *l, Vector2 *pos, Vector2 *apos = NULL) ;
	void animate(Frame *space) = 0;
	};

class BlinkyIndicator : public Indicator {
	public:
	SpaceObject *target;
	int color;
	BlinkyIndicator(SpaceObject *target, int color = -1);
	virtual void animate(Frame *space);
	virtual void calculate();
	};

class WedgeIndicator : public Indicator {
	public:
	SpaceLocation *target;
	int color;
	int length;
	WedgeIndicator(SpaceLocation *target, int length, int color);
	virtual void animate(Frame *space);
	virtual void calculate();
	};

class Orbiter : public SpaceObject {
public:
	SpaceLocation *center;
	double radius;
	virtual void calculate();
	Orbiter ( SpaceSprite *sprite, SpaceLocation *orbit, double distance);
};
