#ifndef _MSHIP_H_
#include "../melee/mship.h"
#endif

#ifndef _NULLPHAS_H_
#define _NULLPHAS_H_

class NullPhaser : public Phaser {
// this phaser instantly calls the materialize function

  public:
  NullPhaser( Ship* oship );

  virtual void animate( Frame* space );  // it does not show
  virtual void calculate();  // it instantly dies and adds the ship
};

#endif
