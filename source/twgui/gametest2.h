/* $Id$ */ 
#ifndef __TWGUI_GAMETEST2_H__
#define __TWGUI_GAMETEST2_H__

#include "twgui.h"

#include "../melee/mgame.h"
#include "../melee/mmain.h"

#include "../games/gflmelee.h"

#include "twbutton.h"
#include "twwindow.h"
#include "twmenuexamples.h"



class WindowManager;





//class TWgui : public GameLayer
class TWgui : public NormalGame
{
protected:
	double	solarsystemsize, planetsystemsize;
	Vector2	refpos;
	bool always_random[16];

	ShipPanelBmp *player_panel[16];

	char texted[512];

public:
	// only temp. use !!
//	GameLayer	*prev, *next;
//	double		refscaletime;

	struct option_str
	{
		int healthbar, wedge, captain;
	} option;



	TWindow *meleemenu;
	PopupT	*alertmenu;
	
	Button *zoomout, *zoomin, *abort, *callhelp, *snapshot, *extraplanet,
			*viewmode, *alert_yes, *alert_no;
	TextButton		*alert_text;
	TextEditBox		*text_edit;
	SwitchButton	*screenswitch,
					*healthbarswitch, *wedgeindicatorswitch, *captainpanelswitch, *chatlogswitch;

	PopupList		*popupviewmode;
	PopupTextInfo	*popup2;
	PopupFleetSelection		*fleetsel;

	virtual void calculate();
	virtual bool handle_key(int k);

	virtual void animate(Frame *frame);
	virtual void animate_predict(Frame *frame, int time);
	virtual void animate();

	virtual void init(Log *_log);

	virtual void choose_new_ships();
	//void menu_add_keys();

	void animate_stuff(Frame *frame);

	void handle_zoom_in();
};




#endif // __TWGUI_GAMETEST2_H__




