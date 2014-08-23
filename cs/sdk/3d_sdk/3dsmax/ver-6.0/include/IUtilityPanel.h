/**********************************************************************
 *<
	FILE: IUtilityPanel.h

	DESCRIPTION: Function-published interface to access the utility panel

	CREATED BY: David Cunningham

	HISTORY: August 31, 2001 file created

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef __IUTILPANEL__
#define __IUTILPANEL__

#include "iFnPub.h"
#define IUTIL_FO_INTERFACE Interface_ID(0x7476dc, 0x4364dc)


//==============================================================================
// class IUtilityPanel
//  
// The interface to the utility panel manager.
//
// This class defines an interface for opening and closing utility plugins, and
// provides maxscript access to the same.
//
// An instance of this interface can be retrieved using the following line of
// code:
//
//   static_cast<IUtilityPanel*>(GetCOREInterface(IUTIL_FO_INTERFACE));
//
// Or, from maxscript (examples):
//
//   UtilityPanel.OpenUtility Resource_Collector
//   UtilityPanel.OpenUtility Bitmap_Photometric_Path_Editor
//   UtilityPanel.CloseUtility
//
// The last example closes the currently open utility, if any.
//
//==============================================================================

// maxscript method enum
enum { util_open, util_close };

class IUtilityPanel : public FPStaticInterface	{
public:

	virtual BOOL OpenUtility(ClassDesc*) = 0;
	virtual void CloseUtility() = 0;

};


#endif