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

#ifdef DESIGN_VER // resource overrides for 3dswin.rc

	#undef  IDD_ABOUTBOX
	#define	IDD_ABOUTBOX					IDD_ABOUTBOX_VIZ				
	#undef  IDS_ABOUTBOX
	#define	IDS_ABOUTBOX					IDS_ABOUTBOX_VIZ
	#undef	IDS_APPTITLE
	#define	IDS_APPTITLE					IDS_APPTITLE_VIZ
	#undef	IDB_PREVIEW
	#define	IDB_PREVIEW						IDB_PREVIEW_VIZ
	#undef  IDR_MAINFRAME
	#define	IDR_MAINFRAME					IDR_MAINFRAME_VIZ				// icon

#endif	// DESIGN_VER

#endif	// _RESOURCEOVERRIDE_H
