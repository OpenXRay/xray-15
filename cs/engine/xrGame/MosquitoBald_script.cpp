#include "pch_script.h"
#include "MosquitoBald.h"
#include "ZoneCampfire.h"

using namespace luabind;

#pragma optimize("s",on)
void CMosquitoBald::script_register	(lua_State *L)
{
	module(L)
	[
		class_<CMosquitoBald,CGameObject>("CMosquitoBald")
			.def(constructor<>()),
		class_<CZoneCampfire,CGameObject>("CZoneCampfire")
			.def(constructor<>())
			.def("turn_on",				&CZoneCampfire::turn_on_script)
			.def("turn_off",			&CZoneCampfire::turn_off_script)
			.def("is_on",				&CZoneCampfire::is_on)
	];
}
