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

#ifdef GAME_VER

	// Dialogs
	#undef  IDD_LIGHT_SHADOW2
	#define IDD_LIGHT_SHADOW2				IDD_LIGHT_SHADOW2_GMAX
	#undef  IDD_FCAMERAPARAM
	#define IDD_FCAMERAPARAM				IDD_FCAMERAPARAM_GMAX
	#undef  IDD_GRIDPARAM2
	#define IDD_GRIDPARAM2					IDD_GRIDPARAM2_GMAX

	// Strings
	#undef  IDS_TH_MAX_TEXT
	#define IDS_TH_MAX_TEXT					IDS_TH_MAX_TEXT_GMAX

#endif	// GAME_VER

#ifdef WEBVERSION

	// Dialogs
	#undef  IDD_SCAMERAPARAM
	#define IDD_SCAMERAPARAM					IDD_SCAMERAPARAM_PLASMA

	#undef  IDD_FCAMERAPARAM
	#define IDD_FCAMERAPARAM					IDD_FCAMERAPARAM_PLASMA

	#undef  IDD_LIGHT_SHADOW2
	#define IDD_LIGHT_SHADOW2					IDD_LIGHT_SHADOW2_PLASMA


	// Strings
	#undef  IDS_TH_MAX_TEXT
	#define IDS_TH_MAX_TEXT					IDS_TH_MAX_TEXT_PLASMA

#endif	// WEBVERSION

#ifdef RENDER_VER

	// Dialog
	#undef  IDD_FCAMERAPARAM
	#define IDD_FCAMERAPARAM					IDD_FCAMERAPARAM_VIZR

#endif // RENDER_VER

#endif	// _RESOURCEOVERRIDE_H
