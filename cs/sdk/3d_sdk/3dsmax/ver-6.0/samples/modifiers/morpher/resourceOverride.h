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

#ifdef GAME_FREE_VER

	#undef  IDD_GLOBAL_PARAMS
	#define IDD_GLOBAL_PARAMS	IDD_GLOBAL_PARAMS_GMAX

#endif	// GAME_VER

#ifdef WEBVERSION

	#undef  IDD_GLOBAL_PARAMS
	#define IDD_GLOBAL_PARAMS	IDD_GLOBAL_PARAMS_PLASMA

#endif	// WEBVERSION

#endif	// _RESOURCEOVERRIDE_H
