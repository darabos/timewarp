
#include <allegro.h>

#include "../melee.h"
REGISTER_FILE
#include "mgame.h"
#include "mshppan.h"
#include "mship.h"
#include "mview.h"
#include "../id.h"
#include "../util/aastr.h"

int PANEL_WIDTH = 64;
int PANEL_HEIGHT = 100;

int CAPTAIN_X = 4;
int CAPTAIN_Y = 65;
int PANEL_DEATH_FRAMES = 2500;
int crew_x = 8;
int crew_y = 53;
int batt_x = 56;
int batt_y = 53;


ShipPanel::ShipPanel(Ship *_ship) {
	id |= ID_SHIP_PANEL;

	panel   = create_bitmap(64, 100);
	captain = create_bitmap(CAPTAIN_WIDTH, CAPTAIN_HEIGHT);

	crew_old   = -1;
	batt_old   = -1;
	batt_light = FALSE;

	panel_needs_update = false;
	captain_needs_update = false;

	always_redraw = false;

	old_thrust       = FALSE;
	old_turn_left    = FALSE;
	old_turn_right   = FALSE;
	old_fire_weapon  = FALSE;
	old_fire_special = FALSE;

	deathframe = 0;
	ship = _ship;

	ship->spritePanel->draw(0, 0, 0, panel);
	ship->spritePanel->draw(0, 0, 1, captain);
	draw_stuff (crew_x, crew_y, 2, 1, -3, -2, 2, round_up(ship->crew), round_up(ship->crew_max), tw_color(ship->crewPanelColor()), 0);
	draw_stuff (batt_x, batt_y, 2, 1, -3, -2, 2, round_up(ship->batt), round_up(ship->batt_max), tw_color(ship->battPanelColor()), 0);

	window = new VideoWindow();
	window->preinit();
	}

ShipPanel::~ShipPanel()
{
  destroy_bitmap(captain);
  destroy_bitmap(panel);
  delete window;
}

void ShipPanel::refresh() {
	panel_needs_update = 1;
	captain_needs_update = 1;
	return;
	}

void ShipPanel::calculate() {
	if (!ship) {
		deathframe -= frame_time;
		if (deathframe <= 0) this->die();
		return;
		}
	if (!ship->exists()) {//ship is dieing
		ship->spritePanel->draw(0, 0, 0, panel);
		panel_needs_update = true;
		ship = NULL;	  
		deathframe = PANEL_DEATH_FRAMES;
		return;
		} 
	if(ship->update_panel) {
		ship->update_panel = false;
		panel_needs_update = true;
		captain_needs_update = true;
		}
	if (
			(ship->thrust != old_thrust) ||
			(ship->turn_left != old_turn_left) ||
			(ship->turn_right != old_turn_right) ||
			(ship->fire_weapon != old_fire_weapon) ||
			(ship->fire_special != old_fire_special))
		captain_needs_update = true;
	return;
	}

void ShipPanel::animate(Frame *space) {
	BITMAP *screen = window->surface;
	if (!screen) return;
	int panel_x, panel_y, panel_width, panel_height;
	panel_x = window->x;
	panel_y = window->y;
	panel_width = window->w;
	panel_height = window->h;
	if (always_redraw || FULL_REDRAW) {
		panel_needs_update = true;
		captain_needs_update = true;
		}
	if (!ship) {
		double w, h;
		if (panel_needs_update) {
			blit (captain, panel, 0, 0, CAPTAIN_X, CAPTAIN_Y, captain->w, captain->h);
			acquire_screen();
//			blit(panel, screen, 0, 0, panel_x, panel_y, 64, 100);
//			blit(captain, screen, 0, 0, captain_x, captain_y, CAPTAIN_WIDTH, CAPTAIN_HEIGHT);
			aa_stretch_blit(panel, screen, 0, 0, panel->w, panel->h, panel_x, panel_y, panel_width, panel_height);
			release_screen();
			}
		if (deathframe < 0) return;
		rectfill(captain, 0, 0, CAPTAIN_WIDTH, CAPTAIN_HEIGHT, 0);
		w = CAPTAIN_WIDTH * deathframe / (double)PANEL_DEATH_FRAMES;
		h = CAPTAIN_HEIGHT * deathframe / (double)PANEL_DEATH_FRAMES;
		if(w < 1.0) w = 1.0;
		if(h < 1.0) h = 1.0;
		rectfill(captain,
				(int)(((double)(CAPTAIN_WIDTH) - w) / 2.0),
				(int)(((double)(CAPTAIN_HEIGHT) - h) / 2.0),
				(int)((double)(CAPTAIN_WIDTH) - (((double)(CAPTAIN_WIDTH) - w) / 2.0)),
				(int)((double)(CAPTAIN_HEIGHT) - (((double)(CAPTAIN_HEIGHT) - h) / 2.0)),
				pallete_color[128 - (7 * deathframe / PANEL_DEATH_FRAMES)]);
		blit (captain, panel, 0, 0, CAPTAIN_X, CAPTAIN_Y, captain->w, captain->h);
		acquire_screen();
//		blit(captain, screen, 0, 0, captain_x, captain_y, CAPTAIN_WIDTH, CAPTAIN_HEIGHT);
		aa_set_mode(0);
		aa_stretch_blit(panel, screen, 0, 0, panel->w, panel->h, panel_x, panel_y, panel_width, panel_height);
		release_screen();
		return;
		}

	if (panel_needs_update) {
		ship->spritePanel->draw(0, 0, 0, panel);
		crew_old = -1;
		batt_old = -1;
		}

	if (captain_needs_update) {
		captain_needs_update = false;
		old_thrust       = ship->thrust;
		old_turn_left    = ship->turn_left;
		old_turn_right   = ship->turn_right;
		old_fire_weapon  = ship->fire_weapon;
		old_fire_special = ship->fire_special;

		ship->spritePanel->draw(0, 0, 1, captain);
		if(ship->thrust)       ship->spritePanel->overlay(1, 2, captain);
		if(ship->turn_right)   ship->spritePanel->overlay(1, 3, captain);
		if(ship->turn_left)    ship->spritePanel->overlay(1, 4, captain);
		if(ship->fire_weapon)  ship->spritePanel->overlay(1, 5, captain);
		if(ship->fire_special) ship->spritePanel->overlay(1, 6, captain);
		blit (captain, panel, 0, 0, CAPTAIN_X, CAPTAIN_Y, captain->w, captain->h);
		panel_needs_update = true;
		}

	if (round_up(ship->crew) != crew_old) {		
		crew_old = round_up(ship->crew);
		draw_stuff (crew_x, crew_y, 2, 1, -3, -2, 2, crew_old, round_up(ship->crew_max), tw_color(ship->crewPanelColor()), 0);
		panel_needs_update = true;
		}

	if (round_up(ship->batt) != batt_old) {
		batt_old = round_up(ship->batt);
		draw_stuff (batt_x, batt_y, 2, 1, -3, -2, 2, batt_old, round_up(ship->batt_max), tw_color(ship->battPanelColor()), 0);
		panel_needs_update = true;
		}

	if((!batt_light) && (ship->weapon_low || ship->special_low)) {
		game->panelSprite->draw(40, 58, 2, panel);
		batt_light = TRUE;
		panel_needs_update = true;
		}
	else if(batt_light && (!ship->weapon_low) && (!ship->special_low)) {
		game->panelSprite->draw(40, 58, 1, panel);
		batt_light = FALSE;
		panel_needs_update = true;
		}

	if (panel_needs_update) {
		panel_needs_update = false;
		window->lock();
//		blit(panel, screen, 0, 0, panel_x, panel_y, 64, 100);
//		blit(captain, screen, 0, 0, captain_x, captain_y, CAPTAIN_WIDTH, CAPTAIN_HEIGHT);
		aa_set_mode(0);
		aa_stretch_blit(panel, window->surface, 0, 0, panel->w, panel->h, panel_x, panel_y, panel_width, panel_height);
		window->unlock();
		}

	return;
	}

void ShipPanel::draw_stuff (int x, int y, int w, int h, int dx, int dy, int m, int value, int max, int color, int bcolor) {
	int i;
	w -= 1;
	h -= 1;
	if (value > max) value = max;
	for (i = 0; i < value; i += 1) {
		int _x = x + dx * (i % m);
		int _y = y + dy * (i / m);
		rectfill(panel, _x, _y, _x+w, _y+h, color);
		}
	for (i = value; i < max; i += 1) {
		int _x = x + dx * (i % m);
		int _y = y + dy * (i / m);
		rectfill(panel, _x, _y, _x+w, _y+h, bcolor);
		}
	return;
	}