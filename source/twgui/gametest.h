#ifndef __TWGUI_GAMETEST__
#define __TWGUI_GAMETEST__

#include "twgui.h"

class stopbutton : public Button
{
	int exitcode;
public:
	// this handles the specific operations for when this button is selected.
	stopbutton();
	virtual void handle_lpress();
	int stop();
};





class TWgui : public NormalGame
{
	AreaReserve *testmenu, *meleemenu;
	
	Button *zoomout, *zoomin, *abort;

	void calculate();
	void animate(Frame *frame);

	void init(Log *_log);
};



#endif




