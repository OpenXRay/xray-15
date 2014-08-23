////////////////////////////////////////////////////////////////////////////
//	Module 		: interfaces.h
//	Created 	: 15.06.2005
//  Modified 	: 23.04.2008
//	Author		: Dmitriy Iassenev
//	Description : script interfaces
////////////////////////////////////////////////////////////////////////////

#ifndef CS_LUA_DEBUGGER_INTERFACES_H_INCLUDED
#define CS_LUA_DEBUGGER_INTERFACES_H_INCLUDED

#include <cs/config.h>

#define CS_LUA_DEBUGGER_CALL		CS_CALL

#ifndef CS_LUA_DEBUGGER_API
#	define CS_LUA_DEBUGGER_API		CS_API
#endif // #ifndef CS_LUA_DEBUGGER_API

#define CS_LUA_DEBUGGER_FILE_NAME	CS_LIBRARY_NAME( lua_debugger, dll )

#include <cs/lua_debugger/backend.h>
#include <cs/lua_debugger/engine.h>
#include <cs/lua_debugger/world.h>

#endif // #ifndef CS_LUA_DEBUGGER_INTERFACES_H_INCLUDE