#ifndef __GFLMELEE_H__
#define __GFLMELEE_H__


class ShipPanelBmp : public Presence {
	public:
	BITMAP *panel;		// this is the whole panel, including the captain pic.
	BITMAP *captain;

	int crew_old;
	int batt_old;
	char batt_light;

	char old_thrust;         // previous keypresses for ship
	char old_turn_left;
	char old_turn_right;
	char old_fire_weapon;
	char old_fire_special;

	int deathframe;

	Ship *ship;

	ShipPanelBmp(Ship *oship);
	virtual ~ShipPanelBmp();

	virtual void draw_stuff(int x, int y, int w, int h, int dx, int dy, int m, int value, int max, int color, int bcolor);
	//used for drawing crew & battery bars

	virtual void animate_panel(); 
	//the parameter "space" is not used
	};

#endif //__GFLMELEE_H__

