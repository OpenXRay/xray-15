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

#undef	IDD_EDPATCH_SURF_OBJ
#define IDD_EDPATCH_SURF_OBJ		IDD_EDPATCH_SURF_OBJ_GMAX

#undef	IDD_EDPATCH_OPS
#define IDD_EDPATCH_OPS				IDD_EDPATCH_OPS_GMAX

#endif	// GAME_VER

#ifdef RENDER_VER

// dialogs
#undef	IDD_EDSPLINE_OPS
#define IDD_EDSPLINE_OPS		IDD_EDSPLINE_OPS_VIZR

#undef	IDD_EDSPLINE_SELECT
#define IDD_EDSPLINE_SELECT		IDD_EDSPLINE_SELECT_VIZR

#endif	// RENDER_VER

#endif	// _RESOURCEOVERRIDE_H
