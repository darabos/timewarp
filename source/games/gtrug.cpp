/* $Id$ */ 
#include <allegro.h>
#include <string.h>

#include "../melee.h"
REGISTER_FILE

#include "../scp.h"
#include "../ais.h"

#include "../melee/mframe.h"
#include "../melee/mgame.h"
#include "../melee/mview.h"
#include "../melee/mcontrol.h"
#include "../melee/mcbodies.h"
#include "../melee/mlog.h"
#include "../melee/mshppan.h"
#include "../melee/mnet1.h"

#include "gtrug.h"

#define truggame ((TrugGame*)game)

TrugGUI *truggy;

TrugControl::TrugControl (const char *name, int channel) 
	: 
	ControlWussie(name, channel)
{
	order.type = ORDER_DEFAULT;
}

void TrugControl::next_order() {
	order.type = ORDER_FREELANCE;
	return;
}

int TrugControl::think()
{
	if (!ship)
		return 0;

	int k = ControlWussie::think();
	switch (order.type) {
		case ORDER_FREELANCE: {
		}
		break;
		case ORDER_OVERRIDE: {
			k = player->manual->keys;
		}
		break;
		case ORDER_PLAYDEAD: {
			k = 0;
		}
		break;
		case ORDER_ATTACK: {
			k &= ~(keyflag::next | keyflag::prev | keyflag::closest);
            //TODO check following 2 lines
			//if (!target || (target->get_serial() == target_serial)) 
			//	break;
			if (game->frame_number & 1) break;
            //TODO check following 4 lines
			//if (!game->find_serial(target_serial)) {
			//	next_order();
            //	break;
			//}
			k |= keyflag::next;
		}
		break;
		case ORDER_MOVE: {
			k = 0;
			Vector2 m = ship->normal_pos();
			m = min_delta(Vector2(order.x,order.y), m);
			if (magnitude_sqr(m) < order.z * order.z) {
				next_order();
				break;
			}
			double ra = min_delta(m.angle(), ship->get_angle());
			if (ra < -PI/64) k |= keyflag::left;
			if (ra >  PI/64) k |= keyflag::right;
			k |= keyflag::thrust;
		}
		break;
	}
	return k;
}

void ViewTrug::init(View *old) {
	View::init(old);
	min = 480;
	max = 4800;
	f = 0;
	return;
}

void ViewTrug::calculate (Game *game) {
	CameraPosition n = camera;
	if (key_pressed(key_zoom_in))  n.z /= 1 + 0.002 * frame_time;
	if (key_pressed(key_zoom_out)) n.z *= 1 + 0.002 * frame_time;
	if (n.z < min) n.z = min;	
	if (n.z > max) n.z = max;
	if (key_pressed(key_alter1)) f += 0.008 * frame_time;
	else f -= 0.008 * frame_time;
	if (f < 0) f = 0;
	if (f > 1.2) f = 1.2;
	Control *poss = truggy->player->manual;
	if (poss && poss->ship) {
		SpaceLocation *c = poss->ship;
		focus ( &n, c );
		n.pos += (f) * view_size * n.z * unit_vector(c->get_angle()) / 4;
	}
	else {
		double spd = 0.002;
		if (key[KEY_LEFT]  || key[KEY_4_PAD]) n.pos.x -= frame_time * spd * n.z;
		if (key[KEY_RIGHT] || key[KEY_6_PAD]) n.pos.x += frame_time * spd * n.z;
		if (key[KEY_UP]    || key[KEY_8_PAD]) n.pos.y -= frame_time * spd * n.z;
		if (key[KEY_DOWN]  || key[KEY_2_PAD]) n.pos.y += frame_time * spd * n.z;
	}
	track(n, frame_time * 2);
	return;
}

TrugPlayer::TrugPlayer(int channel) {
	manual = game->create_control(channel, "Human");
	team = game->new_team();
	overriden = NULL;
	this->channel = channel;
}
void TrugPlayer::calculate() {
}
TrugPlayer::~TrugPlayer() {
}



/*class TrugBase : public SpaceObject {
	public:
	TrugBase (SpaceSprite *sprite, double money, double income, int health);
	double starbucks;
	double income;
};*/


Ship *TrugGame::create_ship(const char *id, TrugPlayer *player, Vector2 pos, double angle) {
	//create AI
	Control * c = new TrugControl("nameless", channel_none);
	if (!c) error("bad Control type!");
	c->load("scp.ini", "Config0");
	c->temporary = true;
	add(c);
	//create ship
	Ship *s = Game::create_ship(id, c, pos, angle, player->team);
	return s;
}

bool TrugGame::handle_key (int k) {
	if (Game::handle_key(k)) return true;
	if (gui && gui->handle_key(k)) return true;
	return false;
}
void TrugGame::prepare() {
	Game::prepare();
	truggy = gui;
}
void TrugGame::preinit() {
	Game::preinit();
	gui = NULL;
}

TrugGame::~TrugGame() {
	unsigned int i;
	for (i = 0; i < players.size(); i += 1) delete players[i];
	if (gui) delete gui;
	gui = NULL;
}

TrugPlayer* TrugGame::add_player ( int channel ) {
	int i = players.size();
	players.push_back( players[i] = new TrugPlayer(channel) );
	return players[i];
}



void TrugGame::calculate() {

	//normal stuff
	Game::calculate();


	//GUI / input stuff
/*	(this->*gui)(gui_event_calculate, frame_time, game_time);
	if ((mouse_b & 1) && !(old_mouse_b & 1)) {
		double x = mouse_x, y = mouse_y;
		if (view->screen2game(&x, &y)) (this->*gui)(gui_event_leftclick_inside, (int)x, (int)y);
		else (this->*gui)(gui_event_leftclick_outside, mouse_x, mouse_y);
		}
	old_mouse_x = mouse_x;
	old_mouse_y = mouse_y;
	old_mouse_b = mouse_b;*/
	return;
	}

void TrugGame::play_sound (SAMPLE *sample, SpaceLocation *source, int vol, int freq) {
	if (source) {
		double d = distance_from(source->normal_pos(), space_center);
		d = (d / size.x);
		int v = (int)floor(256 / (1 + d * 2));
		Physics::play_sound(sample, source, (v * vol) >> 8, freq);
	}
}

void TrugGame::init(Log *_log) {
	Game::init(_log);

	show_mouse(screen);

	normal_turbo /= 1.25;
	size = Vector2(32768, 32768);

	prepare();

	add(new Stars());
	//FIX ME!!!
	change_view ( new ViewTrug() );
	view->window->locate(5,0,5,0,-10,.8,-10,1);
	view_locked = true;

	TrugPlayer *sp = add_player ( channel_server );
	TrugPlayer *cp = add_player ( channel_network[1] );

	add(create_ship("kzedr", sp, random(size), random(PI2)));
	add(create_ship("chebr", cp, random(size), random(PI2)));

	if (p_local != 0)
		add(gui = new TrugGUI( cp ));
	else
		add(gui = new TrugGUI( sp ));

	return;
	}

TrugWidget::TrugWidget ( ) : channel(channel_none) {}
SpaceLocation *TrugWidget::get_focus() {return NULL;}
void TrugWidget::next_focus() {}
void TrugWidget::animate ( Frame *frame ) {}
void TrugWidget::gui_stuff ( Frame *frame, VideoWindow *region, int frame_type, int region_type ) 
{
}

void TrugShipList::gui_stuff ( Frame *frame, VideoWindow *region, int frame_type, int region_type ) 
{

}

TrugGUI::TrugGUI( TrugPlayer *player ) {
	attributes &=~ATTRIB_SYNCHED;
	this->player = player;
	window = new VideoWindow();
	window->preinit();
	prev.x = 0;
	prev.y = 0;
	prev.z = 0;
	prev.buttons = 0;
}

DIALOG TrugStatusScreenDialog[] = {
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
  { d_box_proc,        40,   40,   180,  215,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { d_button_proc,     45,   45,   170,  30,   0x0,  0,    0,    D_EXIT,  0,    0,    (void *)"Ships" , NULL, NULL },
  { d_button_proc,     45,   80,   170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Map" , NULL, NULL },
  { d_button_proc,     45,   115,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Teams" , NULL, NULL },
  { d_button_proc,     45,   150,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Options", NULL, NULL },
  { d_button_proc,     45,   185,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Help", NULL, NULL },
  { d_button_proc,     45,   220,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Exit", NULL, NULL },
  { d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,    0,    160,  480,  255,  0,    0,    0,       1,    0,    NULL, NULL, NULL }
};

void TrugGUI::calculate ( ) {
/*
	Status Screen
			player stats
					# of ships
					starbucks

					value of ships
			arrow keys to scroll camera
			left-click on owned ship to switch to ship view
		S	button to ship list
		P	button to production
		M	button to tactical map

	Ship List
			a list of ships (size 8)
					mouse wheel or scroll buttons to scroll
					left-click or ENTER to ship select
			camera
					? arrow keys to scroll camera ?
					? camera follows currently highlighted ship ?
			left-click on owned ship to switch to ship view
		ESC	button to status
		P	button to production
		M	button to tactical map

	Production
			a list of producers (size 3)
					mouse wheel or scroll buttons to scroll
					click or ENTER to ship selected
			a producers sub-window
			camera
					? arrow keys to scroll camera ?
					? camera follows currently highlighted production center ?
			left-click on owned ship to switch to ship view
		ESC	button to status
		S	button to ship list
		M	button to tactical map

	Ship Selected
			right-click on enemy ship to attack enemy
			right-click on empty space to move
			left-click on owned ship to switch to ship view
			buttons to issue orders
					ORDER_FREELANCE
					ORDER_ATTACK
					ORDER_MOVE
					ORDER_OVERRIDE
			? a list of ships (size 3) ?
		S	button to ship list
		M	button to tactical map

	Tactical Map
			not a GUI-screen type
			replaces the regular camera view
			shows everything on the map, permits arbitrary zooming
			FOG OF WAR
				see 1000-2500 radius around your ships
				see 2500 radius around your production centers
*/
}
void TrugGUI::animate ( Frame *space ) {
}
bool TrugGUI::handle_key ( int key ) {
	return false;
}
void TrugGUI::handle_mouse ( MouseState *now ) {
}

/*
void TrugGame::gui_ship_override(int type, int a, int b) {
	switch (type) {
		case gui_event_selected: {
			if (panel && panel->exists()) {
				panel->die();
				}
			panel = new ShipPanel((Ship*) possession);
			add(panel);
			//panel->locate(video.width-PANEL_WIDTH, 0);
			put_event(generate_event_override(possession->get_serial()));
			acquire_screen();
			textprintf(screen, font, window->w - PANEL_WIDTH - 30, PANEL_HEIGHT + 50, pallete_color[14], 
					"A - Admiral");
			release_screen();
			}
		break;
		case gui_event_deselected: {
			put_event(generate_event_override(0));
			}
		break;
		case gui_event_calculate: {
			if (!possession->exists()) {
				possession = NULL;
				panel = NULL;
				select_gui(&TrugGame::gui_selecting_ship);
				}
			}
		break;
		case gui_event_keypress: {
			switch (b) {
				case KEY_U: {
					select_gui(&TrugGame::gui_ship_selected);
					redraw();
					}
				break;
				}
			}
		break;
		}
	return;
	}

void TrugGame::gui_ship_selected(int type, int a, int b) {
	switch (type) {
		case gui_event_selected: {
			if (panel && panel->exists()) {
				panel->die();
				}
			panel = new ShipPanel((Ship*) possession);
			add(panel);
			//panel->locate(video.width-PANEL_WIDTH, 0);
			acquire_screen();
			textprintf(screen, font, window->w - PANEL_WIDTH - 30, PANEL_HEIGHT + 50, pallete_color[14], 
					"O - Override");
			textprintf(screen, font, window->w - PANEL_WIDTH - 30, PANEL_HEIGHT + 80, pallete_color[14], 
					"D - Deselect");
			release_screen();
			}
		break;
		case gui_event_deselected: {
			acquire_screen();
			rectfill(screen, window->w - PANEL_WIDTH - 30, 0, window->w, window->h, pallete_color[8]);
			release_screen();
			}
		break;
		case gui_event_keypress: {
			switch (b) {
				case KEY_D: {
					possession = NULL;
					select_gui(&TrugGame::gui_selecting_ship);
					redraw();
					}
				break;
				case KEY_O: {
					select_gui(&TrugGame::gui_ship_override);
					}
				break;
				}
			}
		break;
		case gui_event_calculate: {
			if (!possession->exists()) {
				possession = NULL;
				select_gui(&TrugGame::gui_selecting_ship);
				redraw();
				}
			}
		break;
		case gui_event_leftclick_inside: {
			double r = 99999, t;
			SpaceLocation *tmp = NULL;
			Query q;
			for (q.begin(a, b, bit(LAYER_SHIPS), 150); q.current; q.next()) {
				t = distance_from(a, b, q.current->normal_x(), q.current->normal_y());
				if (t < r) {
					tmp = q.current;
					r = t;
					}
				}
			if (tmp) {
				if (tmp->isShip() && (tmp->get_team() == gui_player->team)) {
					possession = tmp;
					select_gui(&TrugGame::gui_ship_selected);
					}
				else if (tmp->get_team() != gui_player->team) {
					put_event(generate_event_attack(possession->get_serial(), tmp->get_serial()));
					}
				}
			}
		break;
		}
	return;
	}

void TrugGame::gui_selecting_ship(int type, int a, int b) {
	switch (type) {
		case gui_event_selected: {
			if (panel && panel->exists()) {
				panel->die();
				panel = NULL;
				}
			possession = NULL;
			}
		break;
		case gui_event_leftclick_inside: {
			double r = 99999, t;
			SpaceLocation *tmp = NULL;
			Query q;
			for (q.begin(a, b, bit(LAYER_SHIPS), 150); q.current; q.next()) {
				t = distance_from(a, b, q.current->normal_x(), q.current->normal_y());
				if (t < r) {
					tmp = q.current;
					r = t;
					}
				}
			if (tmp && tmp->isShip() && (tmp->get_team() == gui_player->team)) {
				possession = tmp;
				select_gui(&TrugGame::gui_ship_selected);
				}
			}
		break;
		}
	return;
	}*/

//REGISTER_GAME (TrugGame, "Trug")
//*/
