#include "melee.h"
REGISTER_FILE
#include "melee/mgame.h"
#include "nullphas.h"

NullPhaser::NullPhaser( Ship* oship ):
  Phaser( oship, oship->normal_pos() - unit_vector( oship->get_angle()) * PHASE_MAX *
  oship->size, unit_vector( oship->get_angle()) * PHASE_MAX * oship->size,
  oship, oship->get_sprite(), oship->get_sprite_index(), hot_color, HOT_COLORS,
  PHASE_DELAY, PHASE_MAX, PHASE_DELAY ){
}

void NullPhaser::animate( Frame* space ){}
void NullPhaser::calculate(){
	STACKTRACE

  if( state > 0 ){
    game->add( ship );
    ship->materialize();
    ship = NULL;
    state = 0;
  }
}

