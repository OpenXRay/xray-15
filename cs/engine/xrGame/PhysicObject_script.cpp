#include "pch_script.h"
#include "PhysicObject.h"

using namespace luabind;

#pragma optimize("s",on)
void CPhysicObject::script_register(lua_State *L)
{
	module(L)
	[
		class_<CPhysicObject,CGameObject>("CPhysicObject")
			.def(constructor<>())
			.def("run_anim_forward",				&CPhysicObject::run_anim_forward)
			.def("run_anim_back",					&CPhysicObject::run_anim_back)
			.def("stop_anim",						&CPhysicObject::stop_anim)
			.def("anim_time_get",					&CPhysicObject::anim_time_get)
			.def("anim_time_set",					&CPhysicObject::anim_time_set)
			.def("play_bones_sound",				&CPhysicObject::play_bones_sound)
			.def("stop_bones_sound",				&CPhysicObject::stop_bones_sound)
	];
}
