#include <allegro.h>

#include "../melee.h"
#include "../frame.h"

#include "../melee/mframe.h"
#include "../melee/mgame.h"
#include "../melee/mmain.h"
#include "../melee/mview.h"
#include "../melee/mcbodies.h"
#include "../melee/mshot.h"

void HyperspaceColorEffects (RGB *c) {
	int alpha = (c->filler ^ 255) + 1;
	c->r = (c->r << 8) / alpha;
	c->r = (c->r * 2 + 256) / 3;
	c->r = (c->r * alpha) >> 8;
	gamma_color_effects (c);
	return;
}

class HyperMelee : public NormalGame {
	public:
	double friction;
	virtual void calculate(int time);
	virtual void init(Log *_log);
	virtual void set_resolution (int screen_x, int screen_y);
	virtual void init_objects();
	~HyperMelee();
	};

void HyperMelee::init_objects() {
	STACKTRACE

	Planet *p;
	int i, m;
	add(new Stars());
	log_file("server.ini");
	m = get_config_int("Hyperspace", "Pholes", 2);
	for (i = 0; i < m; i += 1) {
		p = new Planet (random(size), meleedata.hotspotSprite, random(meleedata.hotspotSprite->frames()));
		p->collide_flag_anyone = 0;
		p->gravity_force *= 0.5;
		p->accelerate(NULL, random(PI2), 0.1 + (random()%36)/100., MAX_SPEED);
		add (p);
		}
	m = get_config_int("Hyperspace", "Nholes", 2);
	for (i = 0; i < m; i += 1) {
		p = new Planet (random(size), meleedata.hotspotSprite, random(meleedata.hotspotSprite->frames()));
		p->collide_flag_anyone = 0;
		p->gravity_force *= -1;
		p->accelerate(NULL, random(PI2), 0.1 + (random()%36)/100., MAX_SPEED);
		add (p);
		}
	add(new Asteroid());
	}

void HyperMelee::init(Log *_log) {
	STACKTRACE

	NormalGame::init(_log);
	normal_turbo *= 1.414;
	shot_relativity /= 2;
	log_file("server.ini");
	friction = get_config_float("Hyperspace", "Friction", 0.0006);
	//units on friction are fraction of velocity lost per millisecond
	videosystem.color_effects = HyperspaceColorEffects;
	videosystem.update_colors();
	unload_all_ship_data();
	return;
	}

HyperMelee::~HyperMelee() {
	videosystem.color_effects = gamma_color_effects;
	unload_all_ship_data();
}

void HyperMelee::set_resolution (int screen_x, int screen_y) {
	NormalGame::set_resolution(screen_x, screen_y);
	view->frame->set_background ( 256/3, 0, 0 );
	return;
	}

void HyperMelee::calculate(int time) {
	STACKTRACE

	int i;
	for (i = 0; i < num_items; i += 1) {
		if (item[i]->exists() && !item[i]->isPlanet()) {
			item[i]->vel *= 1 - friction * time;
			if (item[i]->isShot()) {
				Shot *s = (Shot*)item[i];
				s->v *= 1 - friction * time;
				s->range = s->d + (s->range - s->d) * (1 - friction * time);
				}
			}
		}
	NormalGame::calculate();
	}

REGISTER_GAME ( HyperMelee, "Melee in Hyperspace");
