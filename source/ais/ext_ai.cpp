
#include <allegro.h>
#include <string.h>
#include <stdio.h>

#include "melee.h"
REGISTER_FILE
#include "id.h"
#include "frame.h"

#include "melee/mcbodies.h"
#include "melee/mgame.h"
#include "melee/mcontrol.h"
#include "melee/mship.h"
#include "melee/mshot.h"
#include "melee/manim.h"
#include "melee/mview.h"

#include "ext_ai.h"
#include "other/gdialog.h"

ExternalAI::ExternalAI(SpaceObject * so, std::string script)
{
	ASSERT(so!=NULL);
	space_object = so;
	L = lua_open();
	InitConversationModule( L );

	if ( lua_dofile(L, script.c_str()) != 0)
	{
		tw_error("Error in External AI lua script");
	};
}

/*! \brief summoned when somebody want to communicate with this ship 
	\who is this "somebody"
*/
int ExternalAI::Dialog(SpaceLocation* who)
{
	int top = lua_gettop(L);
	lua_pushstring(L, "DIALOG");
	lua_gettable(L, LUA_GLOBALSINDEX);

	if ( !lua_isfunction(L, -1) )
	{
		return 0;
	};
	lua_call(L, 0, 0);
	lua_settop(L, top);

	return 0;
}
