#ifndef __OBJANIM_H__
#define __OBJANIM_H__

#include "../melee/mframe.h"
#include "../melee/manim.h"

class ObjectAnimation : public Animation {
// maintains speed and angle

  double angle;

  public:
  ObjectAnimation( SpaceLocation *creator, Vector2 opos, Vector2 ovel,
    double oangle, SpaceSprite *osprite, int first_frame, int num_frames, int frame_size,
    double depth );

  virtual void calculate();
};

#endif // __OBJANIM_H__
