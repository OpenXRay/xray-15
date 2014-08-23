/**********************************************************************
 *<
	FILE:			DLLMain.cpp

	DESCRIPTION:	Defines the entry point for the DLL application.
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 10-22-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/


#include "Max.h"
#include "resource.h"

#include "PFChannels_GlobalVariables.h"
#include "PFChannels_GlobalFunctions.h"


BOOL APIENTRY DllMain( HINSTANCE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	static BOOL controlsInit = FALSE;
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			// Hang on to this DLL's instance handle.
			PFChannels::hInstance = hModule;
			if ( !controlsInit )
			{
				controlsInit = TRUE;
				// MAX custom controls
				InitCustomControls(PFChannels::hInstance);
				// Initialize Win95 controls
 				InitCommonControls();
			}
			break;
	}

	return(TRUE);
}

// This function returns the number of plug-in classes this DLL implements
__declspec( dllexport ) int LibNumberClasses() 
{
	return 17;
}

// This function return the ith class descriptor
__declspec( dllexport ) ClassDesc* 
LibClassDesc(int i) 
{
	switch(i) 
	{
		case 0:		return PFChannels::GetParticleChannelNewDesc();
		case 1:		return PFChannels::GetParticleChannelIDDesc();
		case 2:		return PFChannels::GetParticleChannelPTVDesc();
		case 3:		return PFChannels::GetParticleChannelBoolDesc();
		case 4:		return PFChannels::GetParticleChannelIntDesc();
		case 5:		return PFChannels::GetParticleChannelFloatDesc();
		case 6:		return PFChannels::GetParticleChannelPoint3Desc();
		case 7:		return PFChannels::GetParticleChannelQuatDesc();
		case 8:		return PFChannels::GetParticleChannelMatrix3Desc();
		case 9:		return PFChannels::GetParticleChannelMeshDesc();
		case 10:	return PFChannels::GetParticleChannelAngAxisDesc();
		case 11:	return PFChannels::GetParticleChannelTabUVVertDesc();
		case 12:	return PFChannels::GetParticleChannelTabTVFaceDesc();
		case 13:	return PFChannels::GetParticleChannelMapDesc();
		case 14:	return PFChannels::GetParticleChannelMeshMapDesc();
		case 15:	return PFChannels::GetParticleChannelINodeDesc();
		case 16:	return PFChannels::GetParticleChannelVoidDesc();

		default:	return 0;
	}
}

__declspec( dllexport ) const TCHAR *
LibDescription() 
{ 
	return _T(PFChannels::GetString(IDS_PLUGINDESCRIPTION)); 
}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG LibVersion() 
{ 
	return VERSION_3DSMAX; 
}

// don't defer plug-in loading since it may interfere with maxscript creation [bayboro 04-07-2003]
// Let the plug-in register itself for deferred loading
//__declspec( dllexport ) ULONG CanAutoDefer()
//{
//	return 1;
//}





