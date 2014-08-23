//
//	resourceOverride.h
//
//
//	This header file contains product specific resource override code.
//	It remaps max resource ID to derivative product alternatives.
//
//

#ifndef _RESOURCEOVERRIDE_H
#define _RESOURCEOVERRIDE_H

#include "buildver.h"

#ifdef GAME_VER

// dialogs
#undef  IDD_EP_MESHSMOOTH
#define IDD_EP_MESHSMOOTH			IDD_EP_MESHSMOOTH_GMAX

#endif	// GAME_VER

#endif	// _RESOURCEOVERRIDE_H
