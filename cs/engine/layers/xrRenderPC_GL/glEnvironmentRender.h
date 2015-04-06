#pragma once

#include "..\..\include\xrRender\EnvironmentRender.h"

class glEnvironmentRender :
	public IEnvironmentRender
{
public:
	glEnvironmentRender();

	virtual void Copy(IEnvironmentRender &_in) { };
	virtual void OnFrame(CEnvironment &env) { };
	virtual void OnLoad() { };
	virtual void OnUnload() { };
	virtual void RenderSky(CEnvironment &env) { };
	virtual void RenderClouds(CEnvironment &env) { };
	virtual void OnDeviceCreate() { };
	virtual void OnDeviceDestroy() { };
	virtual particles_systems::library_interface const& particles_systems_library() { return (RImplementation.PSLibrary); };
};
