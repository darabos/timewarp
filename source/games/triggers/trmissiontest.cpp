


#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../../melee.h"
REGISTER_FILE

#include "../../melee/mview.h"
#include "../../frame.h"

#include "tredit.h"


#include "trtest.h"
#include "trcontrol.h"
#include "trvarsetting.h"
#include "trmath.h"
#include "trgamesetting.h"
#include "trships.h"

#include "../../melee/mgame.h"
#include "../../twgui/twgui.h"









class GameTrMissionTest : public Game
{
protected:

	action_startfunction *trmain;

public:

	int Nx, Ny, dx, dy;
	int	changelocation;
	action_func	*changefunc_ptr;

	virtual void init(Log *_log);

	virtual void calculate();

};



void GameTrMissionTest::init(Log *_log)
{
	Game::init(_log);

	scare_mouse();

	double H = 2000;
	size = Vector2(H, H);
	prepare();


	// initialize the existing action types :

	action_id.init();

	// create the main program action:
	trmain = new action_startfunction();
	trmain->define();
	//trmain->define_treat();

	action_vars.read_vars("gamedata/trvariables.bin");
	trmain->read("gamedata/tractions.bin");

	// execute 1 iteration
	// for testing initialization functions ?
	trmain->calculate();

}



void GameTrMissionTest::calculate()
{
	Game::calculate();
}




REGISTER_GAME(GameTrMissionTest, "Mission test")

