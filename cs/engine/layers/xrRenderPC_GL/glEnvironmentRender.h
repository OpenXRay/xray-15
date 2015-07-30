#pragma once

#include "..\..\include\xrRender\EnvironmentRender.h"

class glEnvironmentRender :
	public IEnvironmentRender
{
public:
	glEnvironmentRender();

	virtual void Copy(IEnvironmentRender &_in) { VERIFY(!"glEnvironmentRender::Copy not implemented."); };
	virtual void OnFrame(CEnvironment &env) { VERIFY(!"glEnvironmentRender::OnFrame not implemented."); };
	virtual void OnLoad() { VERIFY(!"glEnvironmentRender::OnLoad not implemented."); };
	virtual void OnUnload() { VERIFY(!"glEnvironmentRender::OnUnload not implemented."); };
	virtual void RenderSky(CEnvironment &env) { VERIFY(!"glEnvironmentRender::RenderSky not implemented."); };
	virtual void RenderClouds(CEnvironment &env) { VERIFY(!"glEnvironmentRender::RenderClouds not implemented."); };
	virtual void OnDeviceCreate() { VERIFY(!"glEnvironmentRender::OnDeviceCreate not implemented."); };
	virtual void OnDeviceDestroy() { VERIFY(!"glEnvironmentRender::OnDeviceDestroy not implemented."); };
	virtual particles_systems::library_interface const& particles_systems_library() { return (RImplementation.PSLibrary); };
};
