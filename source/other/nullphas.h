#ifndef __NULLPHAS_H__
#define __NULLPHAS_H__

#include "../melee/mship.h"

class NullPhaser : public Phaser {
// this phaser instantly calls the materialize function

  public:
  NullPhaser( Ship* oship );

  virtual void animate( Frame* space );  // it does not show
  virtual void calculate();  // it instantly dies and adds the ship
};

#endif // __NULLPHAS_H__
