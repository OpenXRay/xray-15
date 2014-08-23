////////////////////////////////////////////////////////////////////////////
//	Module 		: world.h
//	Created 	: 10.04.2008
//  Modified 	: 10.04.2008
//	Author		: Dmitriy Iassenev
//	Description : script debugger world class
////////////////////////////////////////////////////////////////////////////

#ifndef CS_LUA_DEBUGGER_WORLD_H_INCLUDED
#define CS_LUA_DEBUGGER_WORLD_H_INCLUDED

struct lua_State;

namespace cs {
namespace lua_debugger {

struct engine;

struct DECLSPEC_NOVTABLE world {
	virtual	void CS_LUA_DEBUGGER_CALL	add					(lua_State *state) = 0;
	virtual	void CS_LUA_DEBUGGER_CALL	remove				(lua_State *state) = 0;
	virtual	int  CS_LUA_DEBUGGER_CALL	on_error			(lua_State *state) = 0;
	virtual	void CS_LUA_DEBUGGER_CALL	add_log_line		(const char *log_line) = 0;
	virtual	bool CS_LUA_DEBUGGER_CALL	evaluating_watch	() = 0;
}; // struct DECLSPEC_NOVTABLE world

typedef world*	(CS_LUA_DEBUGGER_CALL *create_world_function_type)	(engine& engine);
typedef void	(CS_LUA_DEBUGGER_CALL *destroy_world_function_type)	(world*& world);

} // namespace lua_debugger
} // namespace cs

extern "C" {
	CS_LUA_DEBUGGER_API	cs::lua_debugger::world*	CS_LUA_DEBUGGER_CALL	cs_script_debugger_create_world		(cs::lua_debugger::engine& engine);
	CS_LUA_DEBUGGER_API	void						CS_LUA_DEBUGGER_CALL	cs_script_debugger_destroy_world	(cs::lua_debugger::world*& world);
}

#endif // #ifndef CS_LUA_DEBUGGER_WORLD_H_INCLUDED