/**********************************************************************
 *<
	FILE:			PFChannels_GlobalFunctions.h

	DESCRIPTION:	Collection of useful functions (declaration).
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 10-22-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFCHANNELS_GLOBALFUNCTIONS_H_
#define _PFCHANNELS_GLOBALFUNCTIONS_H_

#include "max.h"

#include "RandGenerator.h"

namespace PFChannels {

TCHAR* GetString(int id);

ClassDesc* GetParticleChannelNewDesc();
ClassDesc* GetParticleChannelIDDesc();
ClassDesc* GetParticleChannelPTVDesc();
ClassDesc* GetParticleChannelINodeDesc();
ClassDesc* GetParticleChannelBoolDesc();
ClassDesc* GetParticleChannelIntDesc();
ClassDesc* GetParticleChannelFloatDesc();
ClassDesc* GetParticleChannelPoint3Desc();
ClassDesc* GetParticleChannelQuatDesc();
ClassDesc* GetParticleChannelMatrix3Desc();
ClassDesc* GetParticleChannelVoidDesc();
ClassDesc* GetParticleChannelMeshDesc();
ClassDesc* GetParticleChannelAngAxisDesc();
ClassDesc* GetParticleChannelTabUVVertDesc();
ClassDesc* GetParticleChannelTabTVFaceDesc();
ClassDesc* GetParticleChannelMapDesc();
ClassDesc* GetParticleChannelMeshMapDesc();


} // end of namespace PFChannels

#endif // _PFCHANNELS_GLOBALFUNCTIONS_H_