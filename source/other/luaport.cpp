

#include <string.h>

#include "luaport.h"




/** \brief A list of lua-type functions

*/

static const int max_lua_func = 1024;
static int Ndescriptors = 0;



lua_func_registry *descriptors[max_lua_func];



lua_func_registry::lua_func_registry(char *descr, lua_CFunction luafunc)
{
	if (Ndescriptors < max_lua_func)
	{
		// store the lua-function information, needed to initialize lua later on.
		strncpy(id, descr, sizeof(id));
		f = luafunc;

		// add this thing to a global list
		descriptors[Ndescriptors] = this;
		++Ndescriptors;
	}
}

/** \brief Accesses a list of lua-type functions and registers them with lua.
*/

void register_lua_functions(lua_State *L)
{
	int i;
	for ( i = 0; i < Ndescriptors; ++i )
	{
	    lua_register(L, descriptors[i]->id, descriptors[i]->f);
	}
}



lua_State *lua_init()
{
	lua_State *L;
	
	L = lua_open();
	
	luaopen_base(L);
	luaopen_table(L);
	luaopen_io(L);
	luaopen_string(L);
	luaopen_math(L);
	luaopen_debug(L);
	
	// lua_registers functions, that are conveniently stored in a separate listing
	// till now.
	register_lua_functions(L);

	return L;
}
	


