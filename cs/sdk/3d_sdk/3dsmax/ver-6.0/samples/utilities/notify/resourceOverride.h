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

#if defined(DESIGN_VER)

	#undef	IDR_MAINFRAME
	#define	IDR_MAINFRAME				IDR_MAINFRAME_VIZ
	#undef	IDS_APPTITLE
	#define	IDS_APPTITLE				IDS_APPTITLE_VIZ
	#undef	IDS_ABOUTBOX_LN1
	#define IDS_ABOUTBOX_LN1		IDS_ABOUTBOX_LN1_VIZ

#endif

#endif	// _RESOURCEOVERRIDE_H