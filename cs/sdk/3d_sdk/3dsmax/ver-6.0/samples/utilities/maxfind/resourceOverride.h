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

	#undef  IDD_ABOUTBOX
	#define IDD_ABOUTBOX			IDD_ABOUTBOX_GMAX
	#undef  IDD_ABOUTBOX_TINY
	#define IDD_ABOUTBOX_TINY		IDD_ABOUTBOX_TINY_GMAX
	#undef  IDS_APPTITLE
	#define IDS_APPTITLE			IDS_APPTITLE_GMAX

	#undef	IDI_MAINWND
	#define IDI_MAINWND				IDI_MAINWND_GMAX

#elif defined(DESIGN_VER)

	#undef  IDD_ABOUTBOX
	#define IDD_ABOUTBOX				IDD_ABOUTBOX_VIZ
	#undef  IDD_ABOUTBOX_TINY
	#define IDD_ABOUTBOX_TINY		IDD_ABOUTBOX_TINY_VIZ
	#undef  IDS_APPTITLE
	#define IDS_APPTITLE				IDS_APPTITLE_VIZ
	#undef  IDI_MAINWND	
	#define IDI_MAINWND				IDI_MAINWND_VIZ
#endif

#endif	// _RESOURCEOVERRIDE_H