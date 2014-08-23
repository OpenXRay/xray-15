/**********************************************************************
 *<
	FILE:			PFChannels_GlobalFunctions.cpp

	DESCRIPTION:	Collection of useful functions (definition).
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		10-22-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/


#include "PFChannels_GlobalFunctions.h"
#include "PFChannels_GlobalVariables.h"


namespace PFChannels {

TCHAR* GetString(int id)
{
	enum { kBufSize = 1024 };
	static TCHAR buf1[kBufSize], buf2[kBufSize], 
				 buf3[kBufSize], buf4[kBufSize];
	static TCHAR* bp[4] = {buf1, buf2, buf3, buf4};
	static int active = 0;
	TCHAR* result = NULL;
	if (hInstance)
		result = LoadString(hInstance, id, bp[active], sizeof(buf1)) ? bp[active] : NULL;
	active = (active+1)%4; // twiddle between buffers to help multi-getstring users (up to 4)
	return result;
}


ClassDesc* GetParticleChannelNewDesc()
{ return &TheParticleChannelNewDesc; }

ClassDesc* GetParticleChannelIDDesc()
{ return &TheParticleChannelIDDesc; }

ClassDesc* GetParticleChannelPTVDesc()
{ return &TheParticleChannelPTVDesc; }

ClassDesc* GetParticleChannelINodeDesc()
{ return &TheParticleChannelINodeDesc; }

ClassDesc* GetParticleChannelBoolDesc()
{ return &TheParticleChannelBoolDesc; }

ClassDesc* GetParticleChannelIntDesc()
{ return &TheParticleChannelIntDesc; }

ClassDesc* GetParticleChannelFloatDesc()
{ return &TheParticleChannelFloatDesc; }

ClassDesc* GetParticleChannelPoint3Desc()
{ return &TheParticleChannelPoint3Desc; }

ClassDesc* GetParticleChannelQuatDesc()
{ return &TheParticleChannelQuatDesc; }

ClassDesc* GetParticleChannelMatrix3Desc()
{ return &TheParticleChannelMatrix3Desc; }

ClassDesc* GetParticleChannelVoidDesc()
{ return &TheParticleChannelVoidDesc; }

ClassDesc* GetParticleChannelMeshDesc()
{ return &TheParticleChannelMeshDesc; }

ClassDesc* GetParticleChannelAngAxisDesc()
{ return &TheParticleChannelAngAxisDesc; }

ClassDesc* GetParticleChannelTabUVVertDesc()
{ return &TheParticleChannelTabUVVertDesc; }

ClassDesc* GetParticleChannelTabTVFaceDesc()
{ return &TheParticleChannelTabTVFaceDesc; }

ClassDesc* GetParticleChannelMapDesc()
{ return &TheParticleChannelMapDesc; }

ClassDesc* GetParticleChannelMeshMapDesc()
{ return &TheParticleChannelMeshMapDesc; }



} // end of namespace PFChannels