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

#ifdef RENDER_VER

	//
	// Dialogs
	//
	#undef  IDD_EDTRI_SELECT
	#define IDD_EDTRI_SELECT	IDD_EDTRI_SELECT_VIZR
	#undef  IDD_EDTRI_SURF_FACE
	#define IDD_EDTRI_SURF_FACE	IDD_EDTRI_SURF_FACE_VIZR
	#undef  IDD_EDTRI_GEOM
	#define IDD_EDTRI_GEOM		IDD_EDTRI_GEOM_VIZR

#endif	// RENDER_VER

#endif	// _RESOURCEOVERRIDE_H
