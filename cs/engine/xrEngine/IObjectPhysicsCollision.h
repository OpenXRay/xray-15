#pragma once

xr_pure_interface IPhysicsShell;
xr_pure_interface IPhysicsElement;
xr_pure_interface IObjectPhysicsCollision
{
public:
	virtual	const IPhysicsShell		*physics_shell()const		= 0;
	virtual const IPhysicsElement	*physics_character()const	= 0;//depricated
};