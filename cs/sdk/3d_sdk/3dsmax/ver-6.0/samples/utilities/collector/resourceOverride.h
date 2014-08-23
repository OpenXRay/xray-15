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

#if defined(GAME_VER)

	#undef  IDD_CL_PANEL
	#define IDD_CL_PANEL			IDD_CL_PANEL_GMAX
	#undef  IDS_NO_VALID_FILE_PATH
	#define IDS_NO_VALID_FILE_PATH	IDS_NO_VALID_FILE_PATH_GMAX

#endif

#if defined(RENDER_VER)

	#undef  IDD_CL_PANEL
	#define IDD_CL_PANEL			IDD_CL_PANEL_VIZR
	#undef  IDS_NO_VALID_FILE_PATH
	#define IDS_NO_VALID_FILE_PATH	IDS_NO_VALID_FILE_PATH_VIZR

#endif

#endif	// _RESOURCEOVERRIDE_H