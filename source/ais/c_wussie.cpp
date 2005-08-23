/* $Id$ */ 
#include <string.h>
#include <stdio.h>
#include <allegro.h>

#include "../melee.h"
#include "../ais.h"
#include "../melee/mcontrol.h"
#include "../melee/mframe.h"
#include "../melee/mship.h"
#include "../melee/mmain.h"

enum {OPTION_UNKNOWN = 0, OPTION_FRONT, OPTION_NO_FRONT, OPTION_SIDES,
	OPTION_FEILD, OPTION_NARROW, OPTION_HOMING, OPTION_NO_RANGE, OPTION_BACK,
	OPTION_PRECEDENCE, OPTION_DEFENSE, OPTION_BATTERY, OPTION_MAX_BATT, OPTION_RESERVE_BATT,
    OPTION_CLOAK, OPTION_PROXIMITY, OPTION_PLUS_FIRE, OPTION_PLUS_SPECIAL, OPTION_LAUNCHED, OPTION_HOLD,
    OPTION_NO_PROXIMITY, OPTION_NEXT_STATE, OPTION_RESET_STATE, OPTION_ALWAYS_WHEN_FULL, OPTION_MINE,
    OPTION_NONE,
	OPTION_LAST // this must be the last in the line
};
/*
#define OPTION_UNKNOWN 0
#define OPTION_NONE 1
#define OPTION_FRONT 2
#define OPTION_NO_FRONT 3
#define OPTION_SIDES 4
#define OPTION_BACK 5
#define OPTION_FEILD 6
#define OPTION_NARROW 7
#define OPTION_HOMING 8
#define OPTION_PROXIMITY 9
#define OPTION_NO_PROXIMITY 10
#define OPTION_NO_RANGE 11
#define OPTION_HOLD 12
#define OPTION_PRECEDENCE 13
#define OPTION_DEFENSE 14
#define OPTION_CLOAK 15
#define OPTION_MAX_BATT 16
#define OPTION_BATTERY 17
#define OPTION_RESERVE_BATT 18
#define OPTION_PLUS_FIRE 19
#define OPTION_PLUS_SPECIAL 20
#define OPTION_LAUNCHED 21
#define OPTION_NEXT_STATE 22
#define OPTION_RESET_STATE 23
#define OPTION_ALWAYS_WHEN_FULL 24
#define OPTION_MINE 25
*/

#define STATE_TOO_CLOSE 0
#define STATE_TOO_FAR 1

#define TACTIC_UNKNOWN 1
#define TACTIC_INDIRECT_INTERCEPT 2
#define TACTIC_DIRECT_INTERCEPT 3
#define TACTIC_RANGE 4

// for testing (graphics):
#include "scp.h"

/*! \brief Check danger
  \return Max damage ship can get from his enemies
*/
double ControlWussie::check_danger ()
{
	STACKTRACE;

	double d = 0;
	Query q;
	q.begin (ship, OBJECT_LAYERS, ship->size.x + ship->size.y + 200);
	for (; q.currento; q.next ())
	{
		if ((ship->distance (q.currento) <
			(ship->size.x + ship->size.y + q.currento->size.x + q.currento->size.y + 5))
		    && (q.currento->damage_factor > 0)
		    && (!ship->sameShip (q.currento)))
		{
			Vector2 ai_pos = ship->normal_pos ();
			Vector2 ai_vel = ship->get_vel ();

			Vector2 enemy_pos = q.currento->normal_pos ();
			Vector2 enemy_vel = q.currento->get_vel ();

			double range = ship->distance (q.currento);
			double next_range = distance_from (
				ai_pos + ai_vel, enemy_pos + enemy_vel);

			if (next_range < range)
				d += q.currento->damage_factor;

//                      double range_step = range - next_range;
		}
	}
	return d;
}

/*! \brief Get control name */
const char *ControlWussie::getTypeName ()
{
	return "WussieBot";
}

// 1. waarom keren ze te snel terug op het pad?
// 2. waarom keren sommige sowieso niet ?

double ControlWussie::evasion (Ship * ship)
{
	STACKTRACE;

	double check_range;

	double frac = ship->turn_rate*1E3 / PI2; // turning-arc-fraction in 1 second.
	// a small value means that it turns slowly.
	if (frac < 0.5)
		frac = 0.5;
	if (frac > 1.5)
		frac = 1.5;

	// for ships that turn slowly, you should anticipate (much) further ahead
	check_range = 750.0 / frac;
	

	//double closetime = 1000 / frac;	// 1000 ms, scaled by more if you can respond only slowly
	double desiredangle = 0;
	Query b;
	SpaceObject *shot;
	//int collideshot;
	//double shipslope, shotslope, shottime, shiptime;
	//double shipint, shotint;
//  double shipspeed,shotspeed;
	//double xs = 0, ys = 0;
	//double velship, velshot;

	if (!ship->speed_max)
	{
		return desiredangle;
	}

	// place the search-circle ahead of the ship, if you move forward that is
	double r = ship->vel.length() / ship->speed_max;
	
	static SpaceLocation center_check(0,0,0);
	center_check.pos = ship->pos + 0.5*r*check_range * unit_vector(ship->vel);

	SpaceObject *collider_object = 0;
	double collider_distance = 1E6;
	double collider_t = 1E6;

	for ( b.begin (&center_check, OBJECT_LAYERS & ~bit (LAYER_CBODIES) & ~bit (LAYER_SHIPS), check_range); b.current; b.next() )
	{
		shot = b.currento;
		if ( shot->canCollide(ship) && shot != ship->target && !shot->isAsteroid() )
		{
			Vector2 dv, dp;

			// find time at which the 2 objects are closest
			dp = min_delta(shot->pos, ship->pos, map_size);
			//dp = shot->pos - ship->pos;
			dv = shot->vel - ship->vel;

			double a, b;
			a = dot_product(dv, dv);
			b = dot_product(dv, dp);

			double t = 0;
			if (a != 0)
			{
				t = -b / a;
			}
			// that's the time.

			// t is the intercept time, at which the distance between the two objects is minimal.

			if (t > 0)	// if they are coming closer
			{
				double distance;

				// this minimal distance is:
				distance = (dp + t * dv).length();

				if (distance < collider_distance && t < collider_t)	// consider it a danger if it is this close
				{
					// you have to evade it.
					collider_t = t;
					collider_object = shot;
					collider_distance = distance;
				}

			}
		}

	}

	/*
	if ( collider_object )
	{
		Vector2 P;
		double R;
		P = corner(ship->pos, collider_object->size* space_zoom);
		R = collider_distance * space_zoom;
		circle(video_screen, P.x, P.y, R, makecol(200,200,0));
	}
	*/


	if ( collider_object && (collider_distance < evasion_base_size + evasion_ship_multiplier * (ship->size.x + shot->size.x)) )
	{
		// there is a dangerous object, and it will also come quite near to your ship... so fear it !!
		
		shot = collider_object;

		double a;

		// however, if the intercept point is close anyway, then simply stay perpendicular on its path
		// whichever is closest to your current orientation
		if (collider_distance < 10*frame_time*ship->vel.length())
		{
			a = shot->vel.angle();
		} else {
			// you should avoid the point of closest approach.
			Vector2 pos_closest = shot->pos + collider_t * shot->vel;
		
			a = min_delta(pos_closest, ship->pos, map_size).atan();
		}



		double da = ship->get_angle() - a;
		
		while (da < -PI)	da += PI2;
		while (da >  PI)	da -= PI2;
		// angle is now between -PI and +PI

		double a_ev = evasion_angle_change;

		// if the enemy weapon is slow, then you should "simply" run away from it... if it is faster,
		// then you should try to move perpendicularly...
		double v_ship, v_shot;
		v_ship = ship->vel.length();
		v_shot = shot->vel.length();

		if (v_ship > 0.9 * v_shot)	// move straight away, if possible... and hope the shot loses damage after a while
		{
			a = ship->trajectory_angle(shot);
			a_ev = PI;	// move straight away
		} else {

			a_ev = evasion_angle_change;
		}
		
		// steer away from the dangerous object.
		if (da < 0) 
			desiredangle = a - a_ev;
		else
			desiredangle = a + a_ev;
	}

	return desiredangle;
}


int ControlWussie::think ()
{
	STACKTRACE;

	if (channel != channel_none)
	{
		tw_error("WussieBot is not on a local channel. It should be, though.");
	}

	if (ship && ship->control != this)
	{
		tw_error("error in control reference...");
	}

	if (!(ship && ship->exists()))
	{
		// maybe it's possible that this is called before the calculate() function... by a ship
		// calculate function or so. It shouldn't be possible, but I'll place it here anyway
		if (ship && temporary)
		{
			tw_error("think() ship=0, shouldn't be possible");
			state = 0;				// this control dies
			ship->control = 0;		// the ship should know.
		}

		ship = 0;
	}

	if (!ship)
		return 0;

	int action = 0;
	float velocity, distance;

	double angle_aim = 0, va, pangle;
	double angle_fire = PI;

	int avoid_planet = FALSE;
	Query ap;
	SpaceObject *p;

	if (ship->target && ship->target->state < -1 )
	{
		tw_error("Ship should clean up its target pointer");
	}
	
	if (ship->target && !ship->target->isInvisible ())
	{
		last_seen_time = game->game_time;
	} else if ((tw_random () & 32767) < frame_time)
	{
		last_seen_time = game->game_time - 1000;
	}

	for (ap.begin (ship, bit (LAYER_CBODIES), planet_safe[state]);
		ap.current; ap.next ())
	{
		p = ap.currento;
		if (p->isPlanet ())
		{
			pangle = ship->trajectory_angle (p);
			va = atan (ship->get_vel ());

			double da;
			da = va - pangle;
			while (da <= -PI)
				da += PI2;
			while (da > PI)
				da -= PI2;
			// now, -PI < da < PI

			if ( fabs(da) < PI/3 || ship->vel.length() < 0.1 )
			{
				avoid_planet = TRUE;

				if ( da < 0 )
					angle_aim = pangle - 0.5*PI;
				else
					angle_aim = pangle + 0.5*PI;
			}
		}
	}

	if (!avoid_planet)
	{
		if (!ship->target || (last_seen_time < game->game_time - 3000)) {
			if ((random() & 4095) < frame_time) {
				if (random() & 3) return keyflag::closest;
				else return keyflag::next;
			}
			return 0;
		}
	}

	if (!avoid_planet && (ship && ship->target && !ship->target->isInvisible()) )
	{

		if (!ship->target->exists ())
		{
			ship->target = NULL;
			return 0;
		}
		distance = ship->distance (ship->target);
		switch (tactic[state])
		{
		default:
		case TACTIC_UNKNOWN:
		case TACTIC_INDIRECT_INTERCEPT:
			{
				//xxx comment:
				// I'm afraid this approach is flawed, because it only takes the primary weapon
				// into account, not the secondary weapon. Thus, the AI can only aim the main,
				// not the special weapon.

			double relativity = 0;
			if (distance < option_range[state][0]) {
				velocity = option_velocity[state][0];
				if (velocity == 0) velocity = MAX_SPEED;
				relativity = rel[state][0];//game->shot_relativity;
			}
			else {
				velocity = ship->speed_max;
			}

			angle_aim = intercept_angle2(
				ship->normal_pos(),
				ship->get_vel() * relativity,	// ship velocity
				velocity,				// weapon velocity
				ship->target->normal_pos(),
				ship->target->get_vel()
				);
				//- ship->get_angle();


			// do another test, namely, check if the enemy ship is not moving away from you. If
			// it does, then you've a problem, cause hot pursuit can be deadly.

			double a;
			a = ship->target->vel.atan() - ship->vel.atan();
			while (a < -PI)	a += PI2;
			while (a > PI)	a -= PI2;

			if (fabs(a) < 0.3 * PI)
			{
				// and is moving away from you...
				// then, try engaging the enemy from another angle of attack ...
				
				
				if (distance > 0.9*option_range[state][0])
				{
					if (ship->target->vel.length() > intercept_abort_speed_factor * ship->speed_max)
					{
						// if the enemy is too fast for you...
						// just move to some other direction
						
						// hmm, actually, this is good practice in almost any occasion...
						// except if you're really much faster than the enemy
						
						double b;
						b = ship->trajectory_angle(ship->target);
						a = ship->angle - b;
						while (a < -PI)	a += PI2;
						while (a > PI)	a -= PI2;
						
						if ( a < 0 )
							angle_aim = b + 0.5*PI;
						else
							angle_aim = b - 0.5*PI;
					}
				}
				
			}
			
			Ship *t;
			if (ship->target->isShip())
			{
				t = (Ship*) ship->target;
			} else {
				t = 0;
			}

			// and what, if the enemy is facing you ? You should be really scared then ... unless ...
			if (t &&
				(t->vel - ship->vel).length() > scared_enemyship_speed_factor * ship->speed_max &&		// be scared if enemy is moving away very fast
				ship->speed_max > 1.1 * t->vel.length() &&		// be scared if you are able to run away from the enemy
				(t->crew/t->crew_max > scare_crew_factor && t->crew > scare_owncrew_minimum) &&		// be scared if the enemy has much crew left
				!ship->isInvisible() &&									// be scared if you're visible
				(distance > option_range[state][0] ||		// be scared if you're out of firing range
				distance < scare_close_distance)					// or be scared if you're *very* close.
				)
			{
				// well... only if the enemy isn't faster than you, otherwise, evading or running
				// away doesn't help you anything.
				// and, only if the enemy has lots of life left (say, 6 life, or only a small fraction of life).
				// and, if you're cloaked you don't have to worry either
				// and, if you're not within fire-range
				
				double d = 2.0 * option_range[state][0];
				if (d > 1000)
					d = 1000;
				
				// evade the enemy
				// the enemy is close
				if (distance < d)
				{
					double b = ship->trajectory_angle(ship->target) + PI;
					a = ship->target->angle - b;
					while (a < -PI)	a += PI2;
					while (a > PI)	a -= PI2;
					
					// the enemy is facing you
					if (fabs(a) < 0.2 * PI)
					{
						if (a > 0)
							angle_aim = b + 0.5*PI;
						else
							angle_aim = b - 0.5*PI;
					}
				}
			}


			/*double rx, ry;
			rx = -ship->normal_pos().x + ship->target->normal_pos().x +
				(ship->target->get_vx ()) * distance / velocity;
			rx = normalize (rx, X_MAX);
			if (rx > X_MAX / 2)
				rx -= X_MAX;
			ry = -ship->normal_pos().y + ship->target->normal_pos().y +
				(ship->target->get_vy ()) * distance / velocity;
			ry = normalize (ry, Y_MAX);
			if (ry > Y_MAX / 2)
				ry -= Y_MAX;
			angle = atan3 (ry, rx) - ship->get_angle ();*/
			}break;

		case TACTIC_DIRECT_INTERCEPT:
			{
			angle_aim = ship->trajectory_angle (ship->target);
				//-ship->get_angle ();
			}
			break;

		case TACTIC_RANGE:
			{
				if (distance > max_range[state])//(tactic_state == STATE_TOO_FAR)
				{
					// you're too far...
					angle_aim = ship->trajectory_angle (ship->target);
						// - ship->get_angle ();
					//if (distance < min_range[state])
					//	tactic_state = STATE_TOO_CLOSE;
				}
				else if ( (tactic_state == 1 && distance < 0.5*(min_range[state]+max_range[state])) ||	// retreat to the middle distance
					(tactic_state == 2 && distance < min_range[state])	// check for min-boundary while you're attacking
					)//(tactic_state == STATE_TOO_CLOSE)
				{
					tactic_state = 1;

					// you're too close, try to get away.
					angle_aim = ship->trajectory_angle (ship->target) + PI;
						//- ship->get_angle ();
				}
				else
				{
					// in good range:

					tactic_state = 2;
					
					velocity = option_velocity[state][0];
					angle_aim = intercept_angle2(
						ship->normal_pos(),
						ship->get_vel(),		// ship velocity
						velocity,				// primary weapon velocity
						ship->target->normal_pos(),
						ship->target->get_vel()
						);
						//- ship->get_angle();

					
					//if (distance > max_range[state])
					//	tactic_state = STATE_TOO_FAR;
				}
			}
			break;

		}

		// if aim is good, you can fire.
		// that's checked here...
		velocity = option_velocity[state][0];
		double relativity = rel[state][0];
		angle_fire = intercept_angle2(
			ship->normal_pos(),
			ship->get_vel() * relativity,		// ship velocity
			velocity,				// primary weapon velocity
			ship->target->normal_pos(),
			ship->target->get_vel()
			);
		// here, angle_fire is an absolute value.


		// and what, if the enemy is invisible
		if (ship && ship->target && ship->target->isInvisible())
		{
			angle_aim = random(PI2);
			if (ship->batt > 0.5*ship->batt_max ||
				ship->batt >= ship->batt_max - ship->weapon_drain)
				action |= keyflag::fire;
		}
		
		// THIS WILL OVERRIDE EARLIER DECISIONS.
		// if you're far away, then it makes sense to check for threats. Otherwise,
		// you shouldn't break off an attack to evade a lousy missile.
		if (distance > 400.0)//0.5*option_range[state][0])
		{
			double a;
			a = evasion (ship);
			
			if (a != 0)
				angle_aim = a;
		}
	}




	double da;
	da = angle_aim - ship->get_angle();	// so that it's the increment that you've to make, to achieve the desired angle
	while (da < -PI)
		da += PI2;
	while (da > PI)
		da -= PI2;


	if (da < 0)
	{
		//angle_aim -= PI2;
		action |= keyflag::left;
	}
	else
	{
		action |= keyflag::right;
	}

	action |= keyflag::thrust;

	// note, that angle_fire is RELATIVE, that's assumed in the rest of the routine...
	angle_fire -= ship->get_angle();
	while (angle_fire < -PI)
		angle_fire += PI2;
	while (angle_fire > PI)
		angle_fire -= PI2;


	int j;
	if (!ship->target)
		return action;
	if (!ship->target->exists ())
	{
		ship->target = NULL;
		return action;
	}

	distance = ship->distance (ship->target);

	// reduce the distance a bit by the respective ship sizes; this is important for shorter range weapons.
	distance -= 0.5 * (ship->size.x + ship->target->size.x);
	if (distance < 0)
		distance = 0;

	for (j = 0; j < 2; j++)
	{
		fireoption[j] = FALSE;
		dontfireoption[j] = FALSE;
		sweep[j] = 20 * ANGLE_RATIO;

		if (option_type[state][j][OPTION_NARROW])
			sweep[j] = 5 * ANGLE_RATIO;
		if (option_type[state][j][OPTION_HOMING])
			sweep[j] = 35 * ANGLE_RATIO;
	}

	Vector2 ship_unit_vector;
	double target_vel, weapon_vel;

	// ship orientation vector
	ship_unit_vector = unit_vector(ship->get_angle());

	// absolute movement of enemy relative to current ship direction
	// (and corrected for relativity... namely, if the weapon also has some of the ship speed!
	weapon_vel = option_velocity[state][j] + dot_product(rel[state][j] * ship->vel, ship_unit_vector);
	target_vel = dot_product(ship->target->vel, ship_unit_vector);


	double a;
	int range_fire, fire_front, field_fire;
	for (j = 0; j < 2; j++)
	{

		if (j == 0)
			range_fire = TRUE;
		else
			range_fire = FALSE;

		if (j == 0)
			fire_front = TRUE;	// primary = front by default.
		else
			fire_front = FALSE;	// secondary = back by default.

		if (j == 0)
			field_fire = FALSE;
		else
			field_fire = TRUE;

		// intercept time along the ship orientation (line of sight)
		double t_intercept;
		if (weapon_vel > target_vel)
			t_intercept = distance / (weapon_vel - target_vel);		// weapon_vel includes correction for weapon relativity
		else
			t_intercept = 1E6;

		// intercept distance along the line of sight
		double d_intercept;
		d_intercept = t_intercept * weapon_vel;
		//xxx this does not take relativity into account ?

		bool weapon_in_range = (option_range[state][j]+0.5*ship->size.x > 1.0 * d_intercept);


		/* hmmm... this is too detailed, and too general.
		// check, if the (uncorrected) velocity isn't too high for your weapon, i.e., the enemy ship
		// moves faster than you:
		if (option_velocity[state][j] < 0.9 * ship->target->vel.length() &&
			(option_range[state][j] > 0.4 * distance ||  distance > 500))
			dontfireoption[j] = TRUE;
			*/


		// fast ships shouldn't be targeted at great distances...
		double rV, rD;
		double v;
		v = ship->target->vel.length();
		rV = option_velocity[state][j] / (v + option_velocity[state][j]);
		rD = option_range[state][j] / (option_range[state][j] + distance);
		
		if ( rD < rV * out_of_range_multiplier  )	// out of range condition
			dontfireoption[j] = TRUE;

		//if (j == 0)// && weapon_in_range)
		//{
		//	message.print(100, 15, "in range  (weapon: %i) (rV=%4.2lf rD=%4.2lf:%i)", int(weapon_in_range), rV, rD);
		//}




		// ------------- CHECKING VARIOUS OPTIONS -------------
		
		if (option_type[state][j][OPTION_UNKNOWN])
		{
			// this is the case, if there were no options checked...
			if (j == 0)
			{
				if ((fabs (angle_fire) < sweep[j]) &&
					weapon_in_range )
					fireoption[0] = TRUE;
			}
		}

		if (option_type[state][j][OPTION_FRONT])
		{
			fire_front = TRUE;
			field_fire = FALSE;
		}
		
		if (option_type[state][j][OPTION_NO_FRONT])
		{
			fire_front = FALSE;
			field_fire = FALSE;
		}
		
		if (option_type[state][j][OPTION_BACK])
		{
			if (fabs (angle_fire) > (PI - sweep[j]))
				fireoption[j] = TRUE;
			field_fire = FALSE;
		}
		
		if (option_type[state][j][OPTION_SIDES])
		{
			if ((fabs (angle_fire) < PI/2 + sweep[j])
				&& (fabs (angle_fire) > PI/2 - sweep[j]))
				fireoption[j] = TRUE;
			field_fire = FALSE;
		}
		
		if (option_type[state][j][OPTION_FEILD])
		{
			//fireoption[j] = TRUE;	//xxx should you disable this here? I think so...
			field_fire = TRUE;
		}

		if (option_type[state][j][OPTION_NO_RANGE])
		{
			range_fire = FALSE;
		}
		
		if (option_type[state][j][OPTION_NO_PROXIMITY])
		{
			if (distance < option_range[state][j])
				dontfireoption[j] = TRUE;
			else
				fireoption[j] = TRUE;	// added... is needed for mrmrm state-change
			range_fire = FALSE;
		}
		
		if (option_type[state][j][OPTION_PROXIMITY])
		{
			if (distance > option_range[state][j])
				dontfireoption[j] = TRUE;
			else
				fireoption[j] = TRUE;	// added... is needed for mrmrm state-change
			range_fire = TRUE;
		}

		if (option_type[state][j][OPTION_BATTERY])
		{
			if (ship->batt <= batt_level[state][j])
				fireoption[j] = TRUE;
			field_fire = FALSE;
		}
		
		if (option_type[state][j][OPTION_MINE])
		{
			a = atan(ship->target->get_vel ());
			if (fabs
				(normalize
				(ship->target->trajectory_angle (ship) + PI/2,
				PI2) - a) < sweep[j])
				fireoption[j] = TRUE;
			field_fire = FALSE;
			fire_front = FALSE;
		}
		
		if (option_type[state][j][OPTION_CLOAK])
		{
			if (!(ship->isInvisible ()))
				fireoption[j] = TRUE;
			else
				dontfireoption[j] = TRUE;
		}

		if (option_type[state][j][OPTION_DEFENSE])
		{
			if (check_danger () > (4 / option_freq[state][j]))
				fireoption[j] = TRUE;
			field_fire = FALSE;
		}
		
		if (option_type[state][j][OPTION_MAX_BATT])
		{
			if (ship->batt != ship->batt_max)
			{
				if (!option_held[j])
					dontfireoption[j] = TRUE;
			}
		}
		
		if (option_type[state][j][OPTION_RESERVE_BATT])
		{
			if (ship->batt < batt_level[state][j])
				if (!option_held[j])
					dontfireoption[j] = TRUE;
		}

		if (option_type[state][j][OPTION_LAUNCHED])
		{
			if (option_held[j])
			{
				bombx[j] += (bombvx[j] * frame_time);
				bomby[j] += (bombvy[j] * frame_time);
				normalize (bombx[j], map_size.x);
				normalize (bomby[j], map_size.y);
				bomb =
					new SpaceLocation (NULL, Vector2(bombx[j], bomby[j]), 0);
				if (ship->target->distance (bomb) > bombdistance[j])
				{
					option_held[j] = FALSE;
					dontfireoption[j] = TRUE;
				}
				else
				{
					bombdistance[j] = ship->target->distance (bomb);
				}
				delete bomb;
			}
			
		}
		
		if (option_type[state][j][OPTION_NONE])
		{
			dontfireoption[j] = TRUE;
		}

		// ----------- DONE CHECKING THE VARIOUS OPTIONS --------




		if (range_fire)
		{
			if (distance > option_range[state][j])
				dontfireoption[j] = TRUE;
		}

		if (fire_front)
		{
			if (fabs (angle_fire) < sweep[j] && weapon_in_range)
				fireoption[j] = TRUE;
		}

		if (field_fire && distance < option_range[state][j])
		{
			fireoption[j] = TRUE;
		}
	}


	for (j = 0; j < 2; j++)
	{
		if (option_type[state][j][OPTION_LAUNCHED])
		{
			if ((!option_held[j]) && (fireoption[j])
				&& (!dontfireoption[j]))
			{
				bombx[j] = ship->normal_pos().x;
				bomby[j] = ship->normal_pos().y;
				bombvx[j] = (ship->get_vel().x * rel[state][j]) +
					(option_velocity[state][j] *
					cos (ship->get_angle ()));
				bombvy[j] =
					(ship->get_vel().y * rel[state][j]) +
					(option_velocity[state][j] *
					sin (ship->get_angle ()));
				bombdistance[j] = ship->distance (ship->target);
			}
		}
		if (option_type[state][j][OPTION_HOLD])
		{
			if ((option_held[j]) && (fireoption[j])
				&& (!dontfireoption[j]))
			{
				dontfireoption[j] = TRUE;
				option_held[j] = FALSE;
			}
			else if (option_held[j])
			{
				option_held[j] = TRUE;
				fireoption[j] = TRUE;
				dontfireoption[j] = FALSE;
			}
			else if (!option_held[j])
			{
				option_held[j] = TRUE;
				fireoption[j] = TRUE;
				dontfireoption[j] = FALSE;
			}
		}
	}


	for (j = 0; j < 2; j++)
	{
		if (option_time[j] > 0)
		{
			option_time[j] -= frame_time;
			dontfireoption[j] = TRUE;
		}

		if (option_type[state][j][OPTION_PRECEDENCE])
		{
			if (fireoption[j] && (!dontfireoption[j]))
			{
				int k;
				if ( j == 0 )
					k = 1;
				else
					k = 0;
				
				dontfireoption[k] = TRUE;
			}
		}
		
		
		if (option_type[state][j][OPTION_ALWAYS_WHEN_FULL])
		{
			if (ship->batt >= ship->batt_max)
			{
				fireoption[j] = TRUE;
				dontfireoption[j] = FALSE;
			}
		}

	}

	int newstate = state;
	for (j = 0; j < 2; j++)
	{
		if (fireoption[j] && (!dontfireoption[j]))
		{
			if (j == 0)
				action |= keyflag::fire;
			else
				action |= keyflag::special;

			option_time[j] = option_timer[state][j];

			if (option_type[state][j][OPTION_LAUNCHED])
				option_held[state] = TRUE;
			
			if (option_type[state][j][OPTION_HOLD])
				option_held[state] = TRUE;
			
			if (option_type[state][j][OPTION_PLUS_SPECIAL])
				action |= keyflag::special;
			
			if (option_type[state][j][OPTION_PLUS_FIRE])
				action |= keyflag::fire;
			
			if (option_type[state][j][OPTION_NEXT_STATE])
				newstate++;
			
			if (option_type[state][j][OPTION_RESET_STATE])
				newstate = 0;
		}
		else
		{
			option_held[j] = FALSE;
		}
	}

	state = newstate;

	if ((random () % 4000) < frame_time) {
		int r = random();
		if (r & 3) action |= keyflag::closest;
		else if (r & 4) {
			if (r&8) action |= keyflag::next;
			else action |= keyflag::prev;
		}
	}
	return action;
}

ControlWussie::ControlWussie (const char *name, int channel):Control (name,
	    channel)
{
}

//void ControlWussie::set_target(int i) {
//      Control::set_target(i);
//      }


void ControlWussie::select_ship (Ship * ship_pointer, const char *ship_name)
{
	STACKTRACE;;

	//char tmp[20];
	int i, j, k;
	Control::select_ship (ship_pointer, ship_name);

	if (ship && ship->control != this)
	{
		tw_error("error in control reference...");
	}
	
	if (ship_name)
	{
		//replace_extension (tmp, ship_name, "ini", 19);
		char states[20], sstr[20];
		const char *w, *s;
		for (k = 0; k < MAX_STATES; k++)
		{

			// check primary and secondary weapon
			for (j = 0; j < 2; j++)
			{
				// reset all posibilities.
				for ( i = 0; i < OPTION_LAST; ++i )
					option_type[k][j][i] = false;

				// initialize the AI options
				strcpy (states, "AI3_Default");
				if (k != 0)
					sprintf (states + strlen (states), "%d", k + 1);

				// read at most 10 possible weapon definitions...
				for (i = 0; i < 10; i++)		// 0 to 9
				{
					if (j == 0)
						strcpy (sstr, "Weapon");
					else
						strcpy (sstr, "Special");

					if (i != 0)
						sprintf (sstr + strlen (sstr), "%d", i + 1);

					w = get_config_string (states, sstr, "");




					if (!strcmp (w, "No_Front"))
						option_type[k][j][OPTION_NO_FRONT] = true;
					else if (!strcmp (w, "Front"))
						option_type[k][j][OPTION_FRONT] = true;
					else if (!strcmp (w, "Sides"))
						option_type[k][j][OPTION_SIDES] = true;
					else if (!strcmp (w, "Field"))
						option_type[k][j][OPTION_FEILD] = true;
					else if (!strcmp (w, "Narrow"))
						option_type[k][j][OPTION_NARROW] = true;
					else if (!strcmp (w, "Homing"))
						option_type[k][j][OPTION_HOMING] = true;
					else if (!strcmp (w, "No_range"))
						option_type[k][j][OPTION_NO_RANGE] = true;
					else if (!strcmp (w, "Back"))
						option_type[k][j][OPTION_BACK] = true;
					else if (!strcmp (w, "Precedence"))
						option_type[k][j][OPTION_PRECEDENCE] = true;
					else if (!strcmp (w, "Defense"))
						option_type[k][j][OPTION_DEFENSE] = true;
					else if (!strcmp (w, "Battery"))
					option_type[k][j][OPTION_BATTERY] = true;
					else if (!strcmp (w, "Max_Battery"))
						option_type[k][j][OPTION_MAX_BATT] = true;
					else if (!strcmp (w, "Reserve_Battery"))
						option_type[k][j][OPTION_RESERVE_BATT] = true;
					else if (!strcmp (w, "Cloak"))
						option_type[k][j][OPTION_CLOAK] = true;
					else if (!strcmp (w, "Proximity"))
						option_type[k][j][OPTION_PROXIMITY] = true;
					else if (!strcmp (w, "Plus_Fire"))
						option_type[k][j][OPTION_PLUS_FIRE] = true;
					else if (!strcmp (w, "Plus_Special"))
						option_type[k][j][OPTION_PLUS_SPECIAL] = true;
					else if (!strcmp (w, "Launched"))
						option_type[k][j][OPTION_LAUNCHED] = true;
					else if (!strcmp (w, "Hold"))
						option_type[k][j][OPTION_HOLD] = true;
					else if (!strcmp (w, "No_Proximity"))
						option_type[k][j][OPTION_NO_PROXIMITY] = true;
					else if (!strcmp (w, "Next_State"))
						option_type[k][j][OPTION_NEXT_STATE] = true;
					else if (!strcmp (w, "Reset_State"))
						option_type[k][j][OPTION_RESET_STATE] = true;
					else if (!strcmp (w, "Always_When_Full"))
						option_type[k][j][OPTION_ALWAYS_WHEN_FULL] = true;
					else if (!strcmp (w, "Mine"))
						option_type[k][j][OPTION_MINE] = true;
					else if (!strcmp (w, "None"))
						option_type[k][j][OPTION_NONE] = true;

					s = get_config_string (states, "Tactic", "");
					tactic[k] = TACTIC_UNKNOWN;
					if (!strcmp (s, "Indirect"))
						tactic[k] = TACTIC_INDIRECT_INTERCEPT;
					else if (!strcmp (s, "Direct"))
						tactic[k] = TACTIC_DIRECT_INTERCEPT;
					else if (!strcmp (s, "Range"))
						tactic[k] = TACTIC_RANGE;

					min_range[k] =
						scale_range (get_config_float
								   (states, "Tactic_Min", 10));
					max_range[k] =
						scale_range (get_config_float
								   (states, "Tactic_Max", 20));
					//tactic_state = 0;
					option_held[j] = FALSE;
					batt_level[k][j] =
						get_config_int (states, "BattRecharge", 0);
				}

				// in case nothing was defined, then default behaviour should be activated:
				bool some_option = false;
				for ( i = 0; i < OPTION_LAST; ++i )
				{
					if (option_type[k][j][i])
						some_option = true;
				}
				if (!some_option)
				{
					option_type[k][j][OPTION_UNKNOWN] = true;
				}
			}

			// default to 0, if there's no such override setting.
			planet_safe[k] = get_config_int (states, "Planet_Distance", 0);

			if (planet_safe[k] == 0) //Launchpad, is this supposed to be here? Geo: yes, otherwise you can't have auto/override settings
			{
				planet_safe[k] = 75 * get_config_int ("ship", "Mass", 8);
				if (planet_safe[k] > 900.0)
					planet_safe[k] = 900.0;		// let's not make it too big.
				if (planet_safe[k] < 500.0)
					planet_safe[k] = 500.0;		// let's not make it too small.
				// note that a planet can be pretty big by itself ...
			}

			option_velocity[k][0] =
				scale_velocity (get_config_float
							 (states, "Weapon_Velocity", 0));
			if (option_velocity[k][0] == 0)
				option_velocity[k][0] =
					scale_velocity (get_config_float
								 ("Weapon", "Velocity", 999));
			option_velocity[k][1] =
				scale_velocity (get_config_float
							 (states, "Special_Velocity", 0));
			if (option_velocity[k][1] == 0)
				option_velocity[k][1] =
					scale_velocity (get_config_float
								 ("Special", "Velocity", 999));
			option_range[k][0] =
				scale_range (get_config_float
						   (states, "Weapon_Range", 0));
			if (option_range[k][0] == 0)
				option_range[k][0] =
					scale_range (get_config_float
							   ("Weapon", "Range", 50));
			option_range[k][1] =
				scale_range (get_config_float
						   (states, "Special_Range", 0));
			if (option_range[k][1] == 0)
				option_range[k][1] =
					scale_range (get_config_float
							   ("Special", "Range", 50));
			option_freq[k][0] =
				0.5 + get_config_float (states, "WeaponFreq", 1);
			option_freq[k][1] =
				0.5 + get_config_float (states, "SpecialFreq", 1);
			option_timer[k][0] =
				scale_frames (get_config_int
						    (states, "Weapon_Timer", -999));
			option_timer[k][1] =
				scale_frames (get_config_int
						    (states, "Special_Timer", -999));
			rel[k][0] = get_config_float (states, "WeaponRel", game->shot_relativity);
			rel[k][1] = get_config_float (states, "SpecialRel", game->shot_relativity);
			option_time[0] = option_timer[0][0];
			option_time[1] = option_timer[0][1];
		}
		state = 0;
	}


	// and perhaps some unique "random" settings for this ship...
	evasion_base_size = 50.0 + random(100.0);
	evasion_ship_multiplier = 1.0 + random(1.0);
	evasion_angle_change = (0.4 + random(0.2)) * PI;

	// lower means, more cowardly
	intercept_abort_speed_factor = 0.3 + random(0.2);

	// scareness: when an attack is aborted..
	scare_crew_factor = 0.25 + random(0.1);
	scare_owncrew_minimum = 4 + random(4);
	scare_close_distance = 150 + random(100);
	scared_enemyship_speed_factor = 0.85 + random(0.1);

	// out of range (shoot or not, the further you are, the smaller the chance).
	out_of_range_multiplier = 0.1 + random(0.1);	// smaller means, less range




	return;
}
