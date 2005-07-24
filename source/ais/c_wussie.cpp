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

#define STATE_TOO_CLOSE 0
#define STATE_TOO_FAR 1

#define TACTIC_UNKNOWN 1
#define TACTIC_INDIRECT_INTERCEPT 2
#define TACTIC_DIRECT_INTERCEPT 3
#define TACTIC_RANGE 4

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

	// minimum check-range is 500 pixels; but, for ships that turn slowly, you should anticipate
	// further ahead...
	check_range = 400.0 / frac;
	

	double closetime = 1000 / frac;	// 1000 ms, scaled by more if you can respond only slowly
	double desiredangle = 0;
	Query b;
	SpaceObject *shot;
	int collideshot;
	double shipslope, shotslope, shottime, shiptime;
	double shipint, shotint;
//  double shipspeed,shotspeed;
	double xs = 0, ys = 0;
	double velship, velshot;
	for ( b.begin (ship, OBJECT_LAYERS & ~bit (LAYER_CBODIES) & ~bit (LAYER_SHIPS), check_range); b.current; b.next() )
	{
		shot = b.currento;
		if (shot->canCollide (ship))
		{
			Vector2 ship_pos, shot_pos;
			ship_pos = ship->normal_pos();
			shot_pos = shot->normal_pos();
			if (fabs(ship->get_vel().y) < 0.0001)
				shipslope = sign(ship->get_vel().x) * 10000;
			else
				shipslope = ship->get_vel().x / ship->get_vel().y;
			if (fabs(shot->get_vel().y) < 0.0001)
				shotslope = sign(shot->get_vel().x) * 10000;
			else
				shotslope = shot->get_vel().x / shot->get_vel().y;
			collideshot = TRUE;
			shipint = ship_pos.y - (shipslope * ship_pos.x);
			shotint = shot_pos.y - (shotslope * shot_pos.x);
			if (fabs(shotint - shipint) < 1 || fabs(shotslope - shipslope) < 0.05)
			{
				collideshot = FALSE;
			}
			else
			{
				xs = (shipint - shotint) / (shotslope - shipslope);
				ys = (shipslope * xs) + shipint;
				//xs = normalize(xs, X_MAX);
				//ys = normalize(ys, Y_MAX);
			}
			velship =
				distance_from (0, ship->get_vel());
			if ((velship == 0) || (!collideshot))
				collideshot = FALSE;
			else
			{
				shiptime = distance_from (normalize2(Vector2(xs, ys), map_size), ship_pos) / velship;
				velshot = distance_from (0, shot->get_vel ());
				if (!((velshot == 0) || (!collideshot)))
				{
					shottime =
						distance_from (normalize2(Vector2(xs, ys), map_size), shot_pos)
						/ velshot;
					if (fabs (shottime - shiptime) < closetime)
					{
						double a = ship->trajectory_angle (shot);
						double da = ship->get_angle() - a;

						closetime = fabs (shottime - shiptime);
						/*
						if ((angle < PI/12) || (angle > PI2-PI/12))
							angle += PI;
						else if (normalize (angle, PI2) >
							    normalize (-angle, PI2))
							angle -= PI/2;
						else
							angle += PI/2;
							*/

						while (da < -PI)	da += PI2;
						while (da >  PI)	da -= PI2;
						// angle is now between -PI and +PI

						// steer away from the dangerous object.
						if (da < 0) 
							desiredangle = a - 0.75*PI;
						else
							desiredangle = a + 0.75*PI;
					}
				}
			}
		}
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

	if (!(ship && ship->exists()))
		ship = 0;

	if (!ship)
		return 0;

	int action = 0;
	float velocity, distance;

	double angle_aim = 0, va, pangle;
	double angle_fire = PI;

	int avoid_planet = FALSE;
	Query ap;
	SpaceObject *p;
	
	if (ship->target && !ship->target->isInvisible ())
	{
		last_seen_time = game->game_time;
	} else if ((random () & 32767) < frame_time)
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
			if (fabs (pangle - va) < PI/3)
			{
				avoid_planet = TRUE;
				if (normalize (pangle - va, PI2) <
				    normalize (va - pangle, PI2))
					angle_aim = -PI/2 + ship->get_angle ();// -
						//ship->trajectory_angle (p);
				else
					angle_aim = PI/2 + ship->get_angle ();// -
						//ship->trajectory_angle (p);
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
					if (ship->target->vel.length() > 0.3 * ship->speed_max)
					{
						// if the enemy is too fast for you...
						// just move to some other direction
						
						// hmm, actually, this is good practice in almost any occasion...
						// except if you're really much faster than the enemy
						
						double b;
						b = ship->trajectory_angle(target);
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
				(t->vel - ship->vel).length() > 0.9 * ship->speed_max &&		// be scared if enemy is moving away very fast
				ship->speed_max > 1.1 * t->vel.length() &&		// be scared if you are able to run away from the enemy
				(t->crew/t->crew_max > 0.3 && t->crew > 6) &&		// be scared if the enemy has much crew left
				!ship->isInvisible() &&									// be scared if you're visible
				(distance > option_range[state][0] ||		// be scared if you're out of firing range
				distance < 200)					// or be scared if you're *very* close.
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


		// if you're out of range anyway, then it makes sense to check for threats. Otherwise,
		// you shouldn't break off an attack to evade a lousy missile.
		if (distance > 0.5*option_range[state][0])
		{
			double a;
			a = evasion (ship);

			if (a != 0)
				angle_aim = a;
		}
	}

	// and what, if the enemy is invisible
	if (ship && ship->target && ship->target->isInvisible())
	{
		angle_aim = random(PI2);
		if (ship->batt > 0.5*ship->batt_max ||
			ship->batt >= ship->batt_max - ship->weapon_drain)
			action |= keyflag::fire;
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


	int i, j;
	if (!ship->target)
		return action;
	if (!ship->target->exists ())
	{
		ship->target = NULL;
		return action;
	}

	distance = ship->distance (ship->target);
	for (j = 0; j < 2; j++)
	{
		fireoption[j] = FALSE;
		dontfireoption[j] = FALSE;
		sweep[j] = 20 * ANGLE_RATIO;
		for (i = 0; i < MAX_OPTION; i++)
		{
			if (option_type[state][j][i] == OPTION_NARROW)
				sweep[j] = 5 * ANGLE_RATIO;
			if (option_type[state][j][i] == OPTION_HOMING)
				sweep[j] = 35 * ANGLE_RATIO;
		}
	}
	double a;
	int range_fire, fire_front, field_fire;
	for (j = 0; j < 2; j++)
	{

		if (j == 0)
			range_fire = TRUE;
		else
			range_fire = FALSE;

		if (j == 0)
			fire_front = TRUE;
		else
			fire_front = FALSE;

		if (j == 0)
			field_fire = FALSE;
		else
			field_fire = TRUE;

		for (i = 0; i < MAX_OPTION; i++)
		{

			switch (option_type[state][j][i])
			{
			default:
			case OPTION_UNKNOWN:
				if (i == 0)
					if (j == 0)
					{
						if ((distance < option_range[state][j])
						    && (fabs (angle_fire) < sweep[j]))
							fireoption[0] = TRUE;
					}
					else if (ship->batt != ship->batt_max)
						dontfireoption[1] = TRUE;
				break;

			case OPTION_FRONT:
				fire_front = TRUE;
				field_fire = FALSE;
				break;

			case OPTION_NO_FRONT:
				fire_front = FALSE;
				field_fire = FALSE;
				break;

			case OPTION_BACK:
				if (fabs (angle_fire) > (PI - sweep[j]))
					fireoption[j] = TRUE;
				field_fire = FALSE;
				break;

			case OPTION_SIDES:
				if ((fabs (angle_fire) < PI/2 + sweep[j])
				    && (fabs (angle_fire) > PI/2 - sweep[j]))
					fireoption[j] = TRUE;
				field_fire = FALSE;
				break;

			case OPTION_FEILD:
				//fireoption[j] = TRUE;	//xxx should you disable this here? I think so...
				field_fire = TRUE;
				break;

			case OPTION_NO_RANGE:
				range_fire = FALSE;
				break;

			case OPTION_NO_PROXIMITY:
				if (distance < option_range[state][j])
					dontfireoption[j] = TRUE;
				else
					fireoption[j] = TRUE;	// added... is needed for mrmrm state-change
				range_fire = FALSE;
				break;

			case OPTION_PROXIMITY:
				if (distance > option_range[state][j])
					dontfireoption[j] = TRUE;
				else
					fireoption[j] = TRUE;	// added... is needed for mrmrm state-change
				range_fire = TRUE;
				break;

			case OPTION_BATTERY:
				if (ship->batt <= batt_level[state][j])
					fireoption[j] = TRUE;
				field_fire = FALSE;
				break;

			case OPTION_MINE:
				a = atan(ship->target->get_vel ());
				if (fabs
				    (normalize
					(ship->target->trajectory_angle (ship) + PI/2,
					 PI2) - a) < sweep[j])
					fireoption[j] = TRUE;
				field_fire = FALSE;
				fire_front = FALSE;
				break;

			case OPTION_CLOAK:
				if (!(ship->isInvisible ()))
					fireoption[j] = TRUE;
				else
					dontfireoption[j] = TRUE;
				break;

			case OPTION_DEFENSE:
				if (check_danger () > (4 / option_freq[state][j]))
					fireoption[j] = TRUE;
				field_fire = FALSE;
				break;

			case OPTION_MAX_BATT:
				if (ship->batt != ship->batt_max)
				{
					if (!option_held[j])
						dontfireoption[j] = TRUE;
				}
				break;

			case OPTION_RESERVE_BATT:
				if (ship->batt < batt_level[state][j])
					if (!option_held[j])
						dontfireoption[j] = TRUE;
				break;

			case OPTION_LAUNCHED:
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
				break;

			case OPTION_NONE:
				dontfireoption[j] = TRUE;
				break;
			}
		}

		if (range_fire)
		{
			if (distance > option_range[state][j])
				dontfireoption[j] = TRUE;
		}

		if (fire_front)
		{
			if (fabs (angle_fire) < sweep[j] && distance < option_range[state][j])
				fireoption[j] = TRUE;
		}

		if (field_fire && distance < option_range[state][j])
		{
			fireoption[j] = TRUE;
		}
	}
	for (j = 0; j < 2; j++)
	{
		for (i = 0; i < MAX_OPTION; i++)
		{
			if (option_type[state][j][i] == OPTION_LAUNCHED)
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
			if (option_type[state][j][i] == OPTION_HOLD)
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
	}


	for (j = 0; j < 2; j++)
	{
		if (option_time[j] > 0)
		{
			option_time[j] -= frame_time;
			dontfireoption[j] = TRUE;
		}

		for (i = 0; i < MAX_OPTION; i++)
		{
			if (option_type[state][j][i] == OPTION_PRECEDENCE)
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


			if (option_type[state][j][i] == OPTION_ALWAYS_WHEN_FULL)
			{
				if (ship->batt >= ship->batt_max)
				{
					fireoption[j] = TRUE;
					dontfireoption[j] = FALSE;
				}
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

			for (i = 0; i < MAX_OPTION; i++)
			{
				if (option_type[state][j][i] == OPTION_LAUNCHED)
					option_held[state] = TRUE;
				if (option_type[state][j][i] == OPTION_HOLD)
					option_held[state] = TRUE;
				if (option_type[state][j][i] == OPTION_PLUS_SPECIAL)
					action |= keyflag::special;
				if (option_type[state][j][i] == OPTION_PLUS_FIRE)
					action |= keyflag::fire;
				if (option_type[state][j][i] == OPTION_NEXT_STATE)
					newstate++;
				if (option_type[state][j][i] == OPTION_RESET_STATE)
					newstate = 0;
			}
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

	char tmp[20];
	int i, j, k;
	Control::select_ship (ship_pointer, ship_name);
	if (ship_name)
	{
		replace_extension (tmp, ship_name, "ini", 19);
		char states[20], sstr[20];
		const char *w, *s;
		for (k = 0; k < MAX_STATES; k++)
		{
			for (j = 0; j < 2; j++)
			{
				strcpy (states, "AI3_Default");
				if (k != 0)
					sprintf (states + strlen (states), "%d", k + 1);
				for (i = 0; i < MAX_OPTION; i++)
				{
					if (j == 0)
						strcpy (sstr, "Weapon");
					else
						strcpy (sstr, "Special");
					if (i != 0)
						sprintf (sstr + strlen (sstr), "%d", i + 1);
					w = get_config_string (states, sstr, "");
					option_type[k][j][i] = OPTION_UNKNOWN;
					if (!strcmp (w, "No_Front"))
						option_type[k][j][i] = OPTION_NO_FRONT;
					else if (!strcmp (w, "Front"))
						option_type[k][j][i] = OPTION_FRONT;
					else if (!strcmp (w, "Sides"))
						option_type[k][j][i] = OPTION_SIDES;
					else if (!strcmp (w, "Field"))
						option_type[k][j][i] = OPTION_FEILD;
					else if (!strcmp (w, "Narrow"))
						option_type[k][j][i] = OPTION_NARROW;
					else if (!strcmp (w, "Homing"))
						option_type[k][j][i] = OPTION_HOMING;
					else if (!strcmp (w, "No_range"))
						option_type[k][j][i] = OPTION_NO_RANGE;
					else if (!strcmp (w, "Back"))
						option_type[k][j][i] = OPTION_BACK;
					else if (!strcmp (w, "Precedence"))
						option_type[k][j][i] = OPTION_PRECEDENCE;
					else if (!strcmp (w, "Defense"))
						option_type[k][j][i] = OPTION_DEFENSE;
					else if (!strcmp (w, "Battery"))
						option_type[k][j][i] = OPTION_BATTERY;
					else if (!strcmp (w, "Max_Battery"))
						option_type[k][j][i] = OPTION_MAX_BATT;
					else if (!strcmp (w, "Reserve_Battery"))
						option_type[k][j][i] = OPTION_RESERVE_BATT;
					else if (!strcmp (w, "Cloak"))
						option_type[k][j][i] = OPTION_CLOAK;
					else if (!strcmp (w, "Proximity"))
						option_type[k][j][i] = OPTION_PROXIMITY;
					else if (!strcmp (w, "Plus_Fire"))
						option_type[k][j][i] = OPTION_PLUS_FIRE;
					else if (!strcmp (w, "Plus_Special"))
						option_type[k][j][i] = OPTION_PLUS_SPECIAL;
					else if (!strcmp (w, "Launched"))
						option_type[k][j][i] = OPTION_LAUNCHED;
					else if (!strcmp (w, "Hold"))
						option_type[k][j][i] = OPTION_HOLD;
					else if (!strcmp (w, "No_Proximity"))
						option_type[k][j][i] = OPTION_NO_PROXIMITY;
					else if (!strcmp (w, "Next_State"))
						option_type[k][j][i] = OPTION_NEXT_STATE;
					else if (!strcmp (w, "Reset_State"))
						option_type[k][j][i] = OPTION_RESET_STATE;
					else if (!strcmp (w, "Always_When_Full"))
						option_type[k][j][i] = OPTION_ALWAYS_WHEN_FULL;
					else if (!strcmp (w, "Mine"))
						option_type[k][j][i] = OPTION_MINE;
					else if (!strcmp (w, "None"))
						option_type[k][j][i] = OPTION_NONE;

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
			}
			planet_safe[k] = get_config_int (states, "Planet_Distance", 0);
//  if (planet_safe[k] == 0); //Launchpad, is this supposed to be here?
			planet_safe[k] = 75 * get_config_int ("ship", "Mass", 8);
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
	return;
}
