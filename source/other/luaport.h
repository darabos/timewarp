#ifndef __LUA_PORT__
#define __LUA_PORT__

extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}


/** \brief Class that is used to initialize a list of lua-type functions during
game initialization. This list just stores some pointers and strings, has nothing
to do with lua, rather, it is temporary storage.

*/

class lua_func_registry
{
public:
	char id[512];
	lua_CFunction f;
	lua_func_registry(char *descr, lua_CFunction luafunc);

};

// Some macros to reduce code/coding complexity :

// declares the default (needed by the arg-macros) function body
// also declares narg, the number of arguments that are to be returned
// also declares the lua-registry class, which is used to build a list of functions available to lua.
#define lua_func(func) \
extern int func(lua_State *L); \
lua_func_registry regme##func (#func, func); \
static int func(lua_State *L) \
{ \
int nret = 0;

// closing bracket, and returns the number of arguments to lua
#define lua_ret \
if (lua_gettop(L) > nret) tw_error("lua: not all arguments were read"); \
return nret; \
}

// declares a double, and initializes with a value from the stack.
#define arg_double(x) \
double x; \
if (!lua_isnumber(L, -1)) tw_error("lua: argument is not a number"); \
x = lua_tonumber(L, -1); \
lua_pop(L, 1);


#define arg_string(x) \
const char *x; \
if (!lua_isstring(L, -1)) tw_error("lua: argument is not a string"); \
x = lua_tostring(L, -1); \
lua_pop(L, 1);




void register_lua_functions(lua_State *L);

lua_State *lua_init();



#endif
