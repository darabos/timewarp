#ifndef _MELEE_H

#include "../melee.h"

#endif



#ifndef _MSHPPAN_H

#define _MSHPPAN_H



extern int PANEL_WIDTH;

extern int PANEL_HEIGHT;





extern int CAPTAIN_X;

extern int CAPTAIN_Y;

extern int PANEL_DEATH_FRAMES;

extern int crew_x;

extern int crew_y;

extern int batt_x;

extern int batt_y;





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



#endif

