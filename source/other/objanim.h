#ifndef _MFRAME_H
#include "../melee/mframe.h"
#endif

#ifndef _MANIM_H
#include "../melee/manim.h"
#endif

#ifndef _OBJANIM_H
#define _OBJANIM_H

class ObjectAnimation : public Animation {
// maintains speed and angle

  double angle;

  public:
  ObjectAnimation( SpaceLocation *creator, Vector2 opos, Vector2 ovel,
    double oangle, SpaceSprite *osprite, int first_frame, int num_frames, int frame_size,
    double depth );

  virtual void calculate();
};

#endif
