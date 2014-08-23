/**********************************************************************

FILE:           IGlobalDXDisplayManager.h

DESCRIPTION:    Public interface controlling the display of the Dx Effect
in the Viewport

CREATED BY:     Neil Hazzard, Discreet

HISTORY:        Created 21 May 2003

*>	Copyright (c) 2003, All Rights Reserved.
**********************************************************************/
#ifndef	__IGLOBALDXDISPLAYMANAGER_H__
#define __IGLOBALDXDISPLAYMANAGER_H__

#include "iFnPub.h"

#define GLOBAL_DX_DISPLAY_MANAGER_INTERFACE Interface_ID(0x7ebe15d6, 0x2b7b422b)

class IGlobalDXDisplayManager : public FPStaticInterface
{
public:

	// Sets a global overide to turn off display of Dx Shaders in the viewport
	virtual void	SetForceSoftware(BOOL set=TRUE)=0;

	// Sets the force display of Dx Effects when an object is selected.  This only works
	// if SetForceSoftware is set to TRUE
	virtual void    SetForceSelected(BOOL set =TRUE )=0;

	//Gets the state of the Force Software flag
	virtual BOOL	IsForceSoftware()=0;

	//Gets the state of the Force Selected falg
    virtual BOOL	IsForceSelected()=0;

	//Queries whether DX is available - useful for UI handlers
	virtual BOOL	IsDirectXActive()=0;

};

inline IGlobalDXDisplayManager* GetGlobalDXDisplayManager() { return (IGlobalDXDisplayManager*)GetCOREInterface(GLOBAL_DX_DISPLAY_MANAGER_INTERFACE); }



#endif