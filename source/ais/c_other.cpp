/* $Id$ */ 
#include "../ais.h"
REGISTER_FILE

/*! \brief Get control name */
const char *ControlVegetable::getTypeName() {
	return "VegetableBot";
	}
/*! \brief This AI do nothing */
int ControlVegetable::think()
{
	return 0;
}
/*! \brief This function do nothing
  \return -1
*/

int ControlVegetable::choose_ship(VideoWindow *window, char * prompt, class Fleet *fleet ) {
	return -1;
	}

ControlVegetable::ControlVegetable (const char *name, int channel)
:
Control(name, channel)
{
	// this is "nothing".
	auto_update = false;
}

const char *ControlMoron::getTypeName()
{
	return "MoronBot";
}

/*! \brief Simple AI 
  \param ship with this stupid AI
*/
int stupid_bot(Ship *ship) {
	STACKTRACE

	int r = 0;
	double a;
	if (!ship->target) return 0;
	if (!ship->target->exists()) {
		ship->target = NULL;
		return 0;
		}
	a = ship->trajectory_angle(ship->target) - ship->get_angle();
	a = fmod(a + PI2, PI2);
  if (a < PI) {
    r |= keyflag::right;
    }
  else {
    r |= keyflag::left;
    return r;
    }
  a = int(ship->distance(ship->target));
  if (a > 2000) {
    r |= keyflag::thrust;
    }
  else {
    r |= keyflag::thrust;
    r |= keyflag::fire;
    }
  return r;
	}

/*! \brief Summon stupid_bot() */
int ControlMoron::think()
{
	if (ship)
		return stupid_bot(ship);
	else
		return 0;
}

ControlMoron::ControlMoron(const char *name, int channel) : Control(name, channel) {}


