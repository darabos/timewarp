/* $Id$ */ 
#include "../melee.h"
REGISTER_FILE

#include "objanim.h"

ObjectAnimation::ObjectAnimation( SpaceLocation *creator, Vector2 opos,
  Vector2 ovel, double oangle, SpaceSprite *osprite, int first_frame,
  int num_frames, int frame_size, double depth ):
Animation( creator, opos, osprite, first_frame, num_frames, frame_size, depth ),
  angle(oangle)
{
	vel = ovel;
	
	sprite_index = get_index(angle);
	sprite_index += first_frame * 64;

	//attributes &= ~ATTRIB_STANDARD_INDEX;
	remember_num_frames = num_frames;
}

void ObjectAnimation::calculate()
{
	STACKTRACE;
	
	// divide by 64 (yields the animation frame)
	sprite_index >>= 6;

	// the animation increases the frame index to the next frame
	Animation::calculate();


	// note that an animation "dies" if the last frame is reached. In that case, you don't need to
	// check for errors anymore.
	if (exists())
	{

		int k = sprite_index;
		if (k >= remember_num_frames || k < 0)
		{
			tw_error("Object Animation error in animation frame !! Should have died.");
		}
		
		// this is the same as multiplying by 64
		sprite_index <<= 6;
		
		// this adds the angle
		int i = get_index(angle);
		sprite_index += i;
		
		// note, that you've got 1 set of animations for 1 angle, and there are 64 angles.
		
		if (sprite_index >= sprite->frames())
		{
			tw_error("SpaceSprite::animate - index %d in %s >= count %d", sprite_index, get_identity(), sprite->frames());
			return;
		}

	}
}

