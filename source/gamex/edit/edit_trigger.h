
#ifndef __GAMEX_EDIT_TRIGGER__
#define __GAMEX_EDIT_TRIGGER__


#include "../gameproject.h"
#include "edit_dialogue.h"

#include "../../twgui/twgui.h"



class GameTriggerEdit : public GameBare
{
	BlockStore	dstore;

	virtual void init();
	virtual void quit();
	//virtual bool handle_key(int k);

	virtual void calculate();
	virtual void animate(Frame *frame);

	WindowManager *winman;

};


#endif


