#include "../ship.h"
REGISTER_FILE

#include "../melee/mmain.h"
#include "../melee/mcbodies.h"

#include "../frame.h"

#define BCC 3

class ConfederationHornet : public Ship {
  int          regenrateFrames;
  int          regenrateCount;
  int          regenrating;
  int          regenrateAmount;

  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;

  double       specialRange;
  double       specialVelocity;
  int          specialDamage;
  int          specialDDamage;
  int          specialArmour;
  double       specialTurnRate;

  int          shield_max;
  int          shield;
  int          shield_old;
  int          shield_x;
  int          shield_y;

  public:
  ConfederationHornet(Vector2 opos, double shipAngle,
		ShipData *shipData, unsigned int code);

  protected:
  virtual int activate_weapon();
  virtual int activate_special();
  virtual void calculate();
  virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};

class TorpedoMissile : public HomingMissile {
	protected:
	int Direct_Damage;

        public:
        TorpedoMissile(double ox, double oy, double oangle, double ov, int odamage,
          int oddamage, double orange, int oarmour, double otrate, SpaceLocation *creator, 
	  SpaceLocation *opos, SpaceSprite *osprite, SpaceObject *otarget);
	virtual void inflict_damage(SpaceObject *other);
};

ConfederationHornet::ConfederationHornet(Vector2 opos, double shipAngle,
		ShipData *shipData, unsigned int code) :
  Ship(opos, shipAngle, shipData, code)
{
  regenrateFrames = get_config_int("Extra", "Frames", 0);
  regenrating     = FALSE;
  regenrateAmount = get_config_int("Extra", "RechargeAmount", 0);

  weaponRange     = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity  = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage    = get_config_int("Weapon", "Damage", 0);
  weaponArmour    = get_config_int("Weapon", "Armour", 0);

  specialRange    = scale_range(get_config_float("Special", "Range", 0));
  specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
  specialDamage   = get_config_int("Special", "Damage", 0);
  specialDDamage   = get_config_int("Special", "DDamage", 0);
  specialArmour   = get_config_int("Special", "Armour", 0);
  specialTurnRate = scale_turning(get_config_float("Special", "TurnRate", 0));

  shield_max      = get_config_int("Extra", "Thickness", 0);
  shield          = shield_max; 
  shield_old      = 0;
  shield_x        = 8;
  shield_y        = 51;

  if (crew_max > 2) {
    if  (((int)ceil(crew_max) % 2) == 0) {
      shield_y -= ((crew_max / 2)*2) - 2;
    } else {
      shield_y -= ((crew_max / 2)*2);
    }
  }
}

int ConfederationHornet::activate_weapon() {
  add(new Missile(this, Vector2(7,35),
    angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
    this, data->spriteWeapon));
  add(new Missile(this, Vector2(-7,35),
    angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
    this, data->spriteWeapon));
  return(TRUE);
}

int ConfederationHornet::activate_special()
{
  add(new TorpedoMissile(0.0, (size.y / 2.0),
    angle, specialVelocity, specialDamage, specialDDamage, specialRange, specialArmour, specialTurnRate, this, this, data->spriteSpecial, target));
  return(TRUE);
}

void ConfederationHornet::calculate()
{
   int shield_color = 9; // Blue
   int i, bar_x, bar_y, shield_panel;

   if (regenrating) {
     if (shield < shield_max) {
       if ((regenrateCount -= frame_time) < 0) {
         shield_old = shield;
         shield += regenrateAmount;
         if (shield > shield_max) shield = shield_max;
         regenrateCount = regenrateFrames;
     } } else regenrating = FALSE;
   } else if (!(regenrating) && (shield < shield_max)) {
     regenrating = TRUE;
     regenrateCount = regenrateFrames;
   }
   if(shield != shield_old) {
     bar_x = 0;
     bar_y = 0;
     shield_panel = ((shield*8)/shield_max) + 8;
		 BITMAP *bmp = spritePanel->get_bitmap(0);
     blit(ship->spritePanel->get_bitmap(shield_panel), bmp, 0, 0, 13, 14, 50, 39);
     for(i = 0; i < shield_max; i++) {
        if((i - shield) < 0) {
         putpixel(bmp, shield_x + bar_x, shield_y + bar_y, pallete_color[shield_color]);
         putpixel(bmp, shield_x + bar_x + 1, shield_y + bar_y, pallete_color[shield_color]);
       } else {
         putpixel(bmp, shield_x + bar_x, shield_y + bar_y, 0);
         putpixel(bmp, shield_x + bar_x + 1, shield_y + bar_y, 0);
       }
       if((i % 2) == 0)
         bar_x = -3;
       else {
         bar_x = 0;
         bar_y -= 2;
       }
     }
     ship->update_panel = TRUE;
     shield_old = shield;
   }
   Ship::calculate();
}

int ConfederationHornet::handle_damage(SpaceLocation *source, double normal, double direct) {
   if ((normal - shield) <= 0) {
     shield -= normal;
	 normal = 0;
   } else {
     normal -= shield;
     shield = 0;
   }
   
   return Ship::handle_damage(source, normal, direct);
}

/** Torpedo Definitions ************************************************************/

TorpedoMissile::TorpedoMissile(double ox, double oy, double oangle, double ov,
  int odamage, int oddamage, double orange, int oarmour, double otrate, SpaceLocation *creator, 
	SpaceLocation *opos, SpaceSprite *osprite, SpaceObject *otarget) :
  HomingMissile(creator, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, otrate, opos, osprite, otarget),
  Direct_Damage(oddamage)
{
}

void TorpedoMissile::inflict_damage(SpaceObject *other) {
	damage(other, 0, Direct_Damage);
  HomingMissile::inflict_damage(other);
}

/** End Torpedo Definitions ********************************************************/


REGISTER_SHIP(ConfederationHornet)
