/* $Id$ */ 
#include "../ship.h"
REGISTER_FILE

#include "../sc1ships.h"

class EarthlingMissile : public HomingMissile {
  public:
  EarthlingMissile(Vector2 opos, double oangle, double ov,
    int odamage, double orange, int oarmour, double otrate, Ship *oship,
    SpaceSprite *osprite);
};

EarthlingCruiser::EarthlingCruiser(Vector2 opos, double angle, ShipData *data, unsigned int code) 
	:
	Ship(opos, angle, data, code)
	{

  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);
  weaponTurnRate = scale_turning(get_config_float("Weapon", "TurnRate", 0));


  specialColor  = get_config_int("Special", "Color", 0);
  specialRange  = scale_range(get_config_float("Special", "Range", 0));
  specialFrames = get_config_int("Special", "Frames", 0);
  specialDamage = get_config_int("Special", "Damage", 0);

	debug_id = 1007;
}

int EarthlingCruiser::activate_weapon() {
	game->add(new EarthlingMissile(
		Vector2(0.0, (size.y * 1.0)), angle, weaponVelocity, weaponDamage, weaponRange,
		weaponArmour, weaponTurnRate, this, data->spriteWeapon));
	return(TRUE);
}

int EarthlingCruiser::activate_special() {
	int fire = FALSE;
	SpaceObject *o;

	Query a;
	for (a.begin(this, bit(LAYER_SHIPS) + bit(LAYER_SHOTS) + bit(LAYER_SPECIAL) + 
			bit(LAYER_CBODIES), specialRange); a.current; a.next()) {
		o = a.currento;
		if( (!o->isInvisible()) && !o->sameTeam(this) && (o->collide_flag_anyone & bit(LAYER_LINES))) {
			SpaceLocation *l = new PointLaser(this, pallete_color[specialColor], 1, 
					specialFrames, this, o, Vector2(0.0, 10.0));
			game->add(l);
			if (l->exists()) {
                                fire = TRUE;
                                l->set_depth(LAYER_EXPLOSIONS); }
			}
		}
	if (fire) play_sound((SAMPLE *)(melee[MELEE_BOOM + 0].dat));

	return(fire);
	}

EarthlingMissile::EarthlingMissile(Vector2 opos, double oangle,
  double ov, int odamage, double orange, int oarmour, double otrate,
  Ship *oship, SpaceSprite *osprite) :
  HomingMissile(oship, opos, oangle, ov, odamage, orange, oarmour, otrate, 
		oship, osprite, oship->target)
{
	collide_flag_sameship = bit(LAYER_SHIPS) | bit(LAYER_SHOTS);
  explosionSprite     = data->spriteWeaponExplosion;
  explosionFrameCount = 10;
  explosionFrameSize  = 50;
	debug_id = 1008;
}


REGISTER_SHIP ( EarthlingCruiser )
