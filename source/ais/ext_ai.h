#ifndef __EXT_AI_H__
#define __EXT_AI_H__

#include <string>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

class SpaceObject;
class SpaceLocation;


/*! \brief responsible for ship external script AI. Including Dialog Support. */
class ExternalAI
{
	lua_State * L;
	SpaceObject * space_object;
public:
	ExternalAI(SpaceObject * ship, std::string script);
	int Dialog(SpaceLocation* who);
};

#endif

