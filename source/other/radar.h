#ifndef __RADAR_H__
#define __RADAR_H__


#include "../melee.h"
#include "../id.h"
#include "../scp.h"
#include "../frame.h"

#include "../melee/mgame.h"
#include "../melee/mview.h"
#include "../melee/mcbodies.h"
#include "../melee/manim.h"
#include "../melee/mship.h"

#include "../melee/mframe.h"

class ZRadar : public Presence
{
public:
	BITMAP *Blank, *Painted;
	Presence *t;
	double size;
	bool active;
	ZRadar(BITMAP *BlankSlate, Presence *target, double Size);
	virtual void animate(Frame *space);

	//ALL the code for painting on the radar screen is in here.
	virtual void Paint(BITMAP *slate, Vector2 T);

	double shiftscale(double r_center, double v_center, double scale, double n);

	void setTarget(SpaceLocation *target);
	void setSize(double Size);
	void toggleActive();
	~ZRadar();
};

#endif // __RADAR_H__
