
#ifdef _DEBUG

#include <allegro.h>

#include "../melee.h"

#include "../melee/mframe.h"
#include "../melee/mgame.h"
#include "../melee/mmain.h"
#include "../melee/mcbodies.h"
#include "../melee/mview.h"

static bool animtoggle = 0;


class GdebugOnly : public NormalGame
{
	virtual void animate(Frame *frame);
	virtual bool handle_key(int k);
	virtual void init(Log *_log);
};


void GdebugOnly::init(Log *_log)
{
	NormalGame::init(_log);

	//turbo = 100;
	normal_turbo = 1000.0;
	turbo = normal_turbo;

	msecs_per_render = 1000;	// draw 1 frame/second.

	prepare();

	// create a list of all the active ships ?
}


void GdebugOnly::animate(Frame *frame)
{
	STACKTRACE

	if (animtoggle)
		NormalGame::animate(frame);

	message.print(1, 15, "t = %i s", int(game_time * 1E-3));
}

bool GdebugOnly::handle_key(int k)
{
	STACKTRACE

	switch (k >> 8)
	{
	case KEY_A:
		animtoggle = !animtoggle;
		break;
	}

	return NormalGame::handle_key(k);
}


REGISTER_GAME ( GdebugOnly, "Debug!");


#endif
