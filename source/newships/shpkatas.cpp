#include "../ship.h"
REGISTER_FILE

/*
 * created by: cyhawk@sch.bme.hu
 */
class KatAssim : public Ship {
  Ship* morph;

  public:
  KatAssim( Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code );

  virtual int  activate_weapon(){ return false; }
  virtual void inflict_damage( SpaceObject* other );
  virtual void calculate();
};

KatAssim::KatAssim( Vector2 opos, double shipAngle,
  ShipData *shipData, unsigned int code ):
  Ship( opos, shipAngle, shipData, code )
{
  morph = NULL;
}

void KatAssim::inflict_damage( SpaceObject* other ){
	STACKTRACE
  Ship::inflict_damage( other );
  if( special_recharge <= 0 || !other->isShot() || !other->ship ) return;
  if( morph ) {
	  add(morph);
	  if (morph->exists()) morph->die();
  }

  morph = game->create_ship( other->ship->type->id, control, pos, angle, get_team() );
  recharge_rate   = morph->recharge_rate;
  recharge_amount = morph->recharge_amount;
  if( batt > morph->batt_max ) batt = morph->batt_max;
  morph->recharge_amount = 0;
  int i;
  for( i = 0; game->target[i] != morph; i++ );
  game->num_targets--;
  game->target[i] = game->target[game->num_targets];
  update_panel = true;

  control->select_ship( this, type->id );
}

void KatAssim::calculate(){
	STACKTRACE
  Ship::calculate();
  if( morph ){
    if( batt > morph->batt_max ) batt = morph->batt_max;
    morph->nextkeys = 0;
    morph->pos = pos; morph->angle = angle; morph->vel = vel;
    if( fire_weapon ){
      morph->batt = batt;
      morph->nextkeys = keyflag::fire;
      morph->calculate();
      batt = morph->batt;
    }else{
      morph->calculate();
    }
  }
}

REGISTER_SHIP(KatAssim)
