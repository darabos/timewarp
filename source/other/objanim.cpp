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
}

void ObjectAnimation::calculate(){
	STACKTRACE

  sprite_index >>= 6;
  Animation::calculate();
  sprite_index <<= 6;
  int i = get_index(angle);
  sprite_index += i;
}

