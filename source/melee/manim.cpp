/* $Id$ */ 
#include "../melee.h"
REGISTER_FILE


#include "../ship.h"
#include "mgame.h"
#include "manim.h"

#include "../util/aastr.h"

Animation::Animation(SpaceLocation *creator, Vector2 opos, 
	SpaceSprite *osprite, int first_frame, int num_frames, int frame_length, 
	double depth, double _scale) 
:
SpaceObject(creator, opos, 0.0, osprite),
frame_count(num_frames),
frame_size(frame_length),
frame_step(frame_length),
scale(_scale),
transparency(0)
{
	 
	if (frame_size <= 0) {
		throw("Animation::Animation - frame_size == %d in %s", frame_size, get_identity());
		frame_size = 1;
	}

	if (first_frame < 0 || first_frame >= sprite->frames() || first_frame + num_frames > sprite->frames())
	{
		// debug info...
		SpaceLocation *l = ship;
		int N = sprite->frames();
		throw("Animation: frame count error in %s.", get_identity());
	}
	sprite_index = first_frame;
	layer = LAYER_HOTSPOTS;
	set_depth(depth);
	collide_flag_anyone = collide_flag_sameteam = collide_flag_sameship = 0;
	mass = 0;

	// it's got no physical interaction with the rest of the world, so, remove it
	// from the query list:
	attributes |= ATTRIB_UNDETECTABLE;

	attributes &= ~ATTRIB_STANDARD_INDEX;
}

void Animation::calculate() {
	 

	frame_step -= frame_time;
	
	while (frame_step < 0)
	{
		frame_step += frame_size;
		sprite_index += 1;
		if (sprite_index == sprite->frames())
			sprite_index = 0;

		frame_count -= 1;

		if (!frame_count)
			state = 0;
	}

	if (sprite_index < 0 || sprite_index >= sprite->frames())
	{
		throw("Animation: sprite index overflow.");
	}
	
	SpaceObject::calculate();
}

void Animation::animate(Frame *space) { 
	if (transparency != 0) {
		int old = aa_get_trans();
		aa_set_trans( iround(old * (1 - transparency) + 255 * transparency) );
		sprite->animate(pos, sprite_index, space, scale);
		aa_set_trans( old );

	}
	else
	{
		if (sprite)
		{
			if (sprite_index >= sprite->frames())
			{
				throw("SpaceSprite::animate - index %d in %s >= count %d", sprite_index, get_identity(), sprite->frames());
				return;
			}
			
			sprite->animate(pos, sprite_index, space, scale);
		}
	}
}

FixedAnimation::FixedAnimation(SpaceLocation *creator, SpaceLocation *opos, 
	SpaceSprite *osprite, int first_frame, int num_frames, int frame_length, 
	double depth) 
	:
	Animation(creator, opos->normal_pos(), osprite, first_frame, num_frames, frame_length, depth),
	follow(opos)
	{ 
	if(!follow->exists()) {
		state = 0;
		follow = NULL;
		}
	}

void FixedAnimation::calculate()
{
	 

	if (follow->exists())
	{
		pos = follow->normal_pos();
	} 
	else
	{
		state = 0;
		follow = NULL;
	}

	Animation::calculate();
}

PositionedAnimation::PositionedAnimation(SpaceLocation *creator, 
	SpaceLocation *opos, Vector2 rel_pos, SpaceSprite *osprite, int first_frame,
	int num_frames, int frame_length, double depth)
	:
	FixedAnimation(creator, opos, osprite, first_frame, num_frames, frame_length, depth),
	relative_pos(rel_pos)
	{ 
	if (!follow || !follow->exists()) {
		state = 0;
		return;
	}
	pos += rotate(relative_pos, follow->get_angle());
	}

void PositionedAnimation::calculate() { 
	FixedAnimation::calculate();
	if (!exists()) return;
	pos += rotate(relative_pos, follow->get_angle());
	}
