#ifndef __MANIM_H__
#define __MANIM_H__

#include "mframe.h"

class Animation : public SpaceObject {
	protected:
	int frame_count;
	int frame_size;
	int frame_step;
	double scale;

	public:
	Animation(SpaceLocation *creator, Vector2 opos, SpaceSprite *osprite, 
			int first_frame, int num_frames, int frame_size, double depth, double scale = 1.0) ;

	virtual void calculate();
	virtual void animate ( Frame * space ) ;
	public:
	float transparency;
};

class FixedAnimation : public Animation {
	public:
	SpaceLocation *follow;

	FixedAnimation(SpaceLocation *creator, SpaceLocation *opos, SpaceSprite *osprite,
		int first_frame, int num_frames, int frame_length, double depth) ;

	virtual void calculate();
};

class PositionedAnimation : public FixedAnimation {
protected:
	Vector2 relative_pos;

public:
	PositionedAnimation(SpaceLocation *creator, SpaceLocation *opos, Vector2 orel_pos,
		SpaceSprite *osprite, int first_frame,
		int num_frames, int frame_length, double depth);
	virtual void calculate();
};

#endif // __MANIM_H__
