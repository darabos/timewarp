#include "../ais.h"
REGISTER_FILE

const char *ControlVegetable::getTypeName() {
	return "VegetableBot";
	}
int ControlVegetable::think() {
	return 0;
	}
int ControlVegetable::choose_ship(VideoWindow *window, char * prompt, class Fleet *fleet ) {
	return -1;
	}
ControlVegetable::ControlVegetable (const char *name, int channel) : Control(name, channel) {
	}

const char *ControlMoron::getTypeName() {
	return "MoronBot";
	}
int stupid_bot(Ship *ship) {
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
int ControlMoron::think() {
	return stupid_bot(ship);
	}
ControlMoron::ControlMoron(const char *name, int channel) : Control(name, channel) {}


