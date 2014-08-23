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

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"

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
			PFActions::hInstance = hModule;
			if ( !controlsInit )
			{
				controlsInit = TRUE;
				// MAX custom controls
				InitCustomControls(PFActions::hInstance);
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
	return 45;
}

// This function return the ith class descriptor
__declspec( dllexport ) ClassDesc* 
LibClassDesc(int i) 
{
	switch(i) 
	{
			// operators
		case 0:		return PFActions::GetPFOperatorSimpleBirthDesc();
		case 1:		return PFActions::GetPFOperatorSimplePositionDesc();	
		case 2:		return PFActions::GetPFOperatorPositionOnObjectDesc();
		case 3:		return PFActions::GetPFOperatorSimpleSpeedDesc();
		case 4:		return PFActions::GetPFOperatorSpeedSurfaceNormalsDesc();
		case 5:		return PFActions::GetPFOperatorSpeedCopyDesc();
		case 6:		return PFActions::GetPFOperatorSpeedKeepApartDesc();
		case 7:		return PFActions::GetPFOperatorSimpleShapeDesc();
		case 8:		return PFActions::GetPFOperatorInstanceShapeDesc();
		case 9:		return PFActions::GetPFOperatorFacingShapeDesc();
		case 10:	return PFActions::GetPFOperatorMarkShapeDesc();
		case 11:	return PFActions::GetPFOperatorSimpleScaleDesc();
		case 12:	return PFActions::GetPFOperatorSimpleOrientationDesc();
		case 13:	return PFActions::GetPFOperatorSimpleSpinDesc();
		case 14:	return PFActions::GetPFOperatorSimpleMappingDesc();
		case 15:	return PFActions::GetPFOperatorForceSpaceWarpDesc();
		case 16:	return PFActions::GetPFOperatorMaterialStaticDesc();
		case 17:	return PFActions::GetPFOperatorMaterialDynamicDesc();
		case 18:	return PFActions::GetPFOperatorMaterialFrequencyDesc();
		case 19:	return PFActions::GetPFOperatorExitDesc();
		case 20:	return PFActions::GetPFOperatorDisplayDesc();	
		case 21:	return PFActions::GetPFOperatorRenderDesc();	
		case 22:	return PFActions::GetPFOperatorCommentsDesc();	

			// tests
		case 23:	return PFActions::GetPFTestDurationDesc();
		case 24:	return PFActions::GetPFTestSpeedDesc();
		case 25:	return PFActions::GetPFTestScaleDesc();
		case 26:	return PFActions::GetPFTestSpawnDesc();
		case 27:	return PFActions::GetPFTestCollisionSpaceWarpDesc();
		case 28:	return PFActions::GetPFTestSpawnOnCollisionDesc();
		case 29:	return PFActions::GetPFTestSpeedGoToTargetDesc();
		case 30:	return PFActions::GetPFTestGoToNextEventDesc();
		case 31:	return PFActions::GetPFTestSplitByAmountDesc();
		case 32:	return PFActions::GetPFTestSplitBySourceDesc();
		case 33:	return PFActions::GetPFTestSplitSelectedDesc();
		case 34:	return PFActions::GetPFTestGoToRotationDesc();

			// custom action states
		case 35:	return PFActions::GetPFOperatorSimpleBirthStateDesc();
		case 36:	return PFActions::GetPFOperatorSimplePositionStateDesc();
		case 37:	return PFActions::GetPFOperatorInstanceShapeStateDesc();
		case 38:	return PFActions::GetPFOperatorFacingShapeStateDesc();
		case 39:	return PFActions::GetPFOperatorMarkShapeStateDesc();
		case 40:	return PFActions::GetPFOperatorMaterialStaticStateDesc();
		case 41:	return PFActions::GetPFOperatorMaterialDynamicStateDesc();
		case 42:	return PFActions::GetPFOperatorMaterialFrequencyStateDesc();
		case 43:	return PFActions::GetPFOperatorPositionOnObjectStateDesc();
		case 44:	return PFActions::GetPFTestSplitByAmountStateDesc();

		default:	return 0;
	}
}

__declspec( dllexport ) const TCHAR *
LibDescription() 
{ 
	return _T(PFActions::GetString(IDS_PLUGINDESCRIPTION)); 
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





