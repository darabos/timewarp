/* Star control - timewarp */

#ifndef _MAINMENU
#define _MAINMENU

#include "../scp.h"

class MainMenu : public BaseClass {
	public:
	virtual void _event(Event * e);
	virtual void preinit();
	virtual void deinit();
	virtual void init(VideoWindow *parent);
	virtual void doit();
	virtual void enable();
	virtual void disable();
	int state;
	VideoWindow *window;
};

void prepareTitleScreenAssets();


extern MainMenu mainmenu;

#endif
