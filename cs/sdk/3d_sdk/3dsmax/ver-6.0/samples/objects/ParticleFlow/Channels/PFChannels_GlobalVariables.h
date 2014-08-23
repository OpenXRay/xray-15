/**********************************************************************
 *<
	FILE:			PFChannels_GlobalVariables.h

	DESCRIPTION:	Collection of global variables and constants (declaration).
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		10-22-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFCHANNELS_GLOBALVARIABLES_H_
#define _PFCHANNELS_GLOBALVARIABLES_H_

#include "max.h"

#include "ParticleChannelDesc.h"

namespace PFChannels {

extern HINSTANCE hInstance;

extern ParticleChannelNewDesc			TheParticleChannelNewDesc;
extern ParticleChannelIDDesc			TheParticleChannelIDDesc;
extern ParticleChannelPTVDesc			TheParticleChannelPTVDesc;
extern ParticleChannelINodeDesc			TheParticleChannelINodeDesc;
extern ParticleChannelBoolDesc			TheParticleChannelBoolDesc;
extern ParticleChannelIntDesc			TheParticleChannelIntDesc;
extern ParticleChannelFloatDesc			TheParticleChannelFloatDesc;
extern ParticleChannelPoint3Desc		TheParticleChannelPoint3Desc;
extern ParticleChannelQuatDesc			TheParticleChannelQuatDesc;
extern ParticleChannelMatrix3Desc		TheParticleChannelMatrix3Desc;
extern ParticleChannelVoidDesc			TheParticleChannelVoidDesc;
extern ParticleChannelMeshDesc			TheParticleChannelMeshDesc;
extern ParticleChannelAngAxisDesc		TheParticleChannelAngAxisDesc;
extern ParticleChannelTabUVVertDesc		TheParticleChannelTabUVVertDesc;
extern ParticleChannelTabTVFaceDesc		TheParticleChannelTabTVFaceDesc;
extern ParticleChannelMapDesc			TheParticleChannelMapDesc;
extern ParticleChannelMeshMapDesc		TheParticleChannelMeshMapDesc;

} // end of namespace EPDElements

#endif // _PFCHANNELS_GLOBALVARIABLES_H_