/* $Id$ */ 
#ifndef __MSHPPAN_H__
#define __MSHPPAN_H__

#include "../melee.h"

extern const int PANEL_WIDTH;
extern const int PANEL_HEIGHT;


extern const int CAPTAIN_X;
extern const int CAPTAIN_Y;
extern const int PANEL_DEATH_FRAMES;
extern const int crew_x;
extern const int crew_y;
extern const int batt_x;
extern const int batt_y;


class ShipPanel : public Presence {
	protected:
	BITMAP *panel;
	BITMAP *captain;

	int crew_old;
	int batt_old;
	char batt_light;

	char panel_needs_update;
	char captain_needs_update;

	char old_thrust;         // previous keypresses for ship
	char old_turn_left;
	char old_turn_right;
	char old_fire_weapon;
	char old_fire_special;
	char last_turn_type;

	int deathframe;

	public:
	Ship *ship;

	int always_redraw;

	ShipPanel(Ship *oship);
	virtual ~ShipPanel();

	virtual void draw_stuff(int x, int y, int w, int h, int dx, int dy, int m, int value, int max, int color, int bcolor);
	//used for drawing crew & battery bars

	virtual void refresh();
	//causes redraw sometime

	//virtual void hide();
	//remove the panel from the screen
	//virtual void locate(int x, int y, int w = PANEL_WIDTH, int h = PANEL_HEIGHT);
	//move the panel to x, y //add width & height later?
	VideoWindow *window;

	virtual void calculate();
	//called every frame
	virtual void animate(Frame *space); 
	//the parameter "space" is not used
	};

#endif  // __MSHPPAN_H__
