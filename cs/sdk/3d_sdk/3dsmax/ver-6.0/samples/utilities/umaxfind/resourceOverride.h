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

	#undef  IDD_PANEL
	#define IDD_PANEL			IDD_PANEL_GMAX
	#undef  IDS_CLASSNAME
	#define IDS_CLASSNAME		IDS_CLASSNAME_GMAX
	#undef  IDS_TITLE
	#define IDS_TITLE			IDS_TITLE_GMAX
	#undef  IDS_NOEXE
	#define IDS_NOEXE			IDS_NOEXE_GMAX
	#undef  IDS_EXE_FILENAME
	#define IDS_EXE_FILENAME	IDS_EXE_FILENAME_GMAX

#elif defined(DESIGN_VER)

	#undef  IDD_PANEL
	#define IDD_PANEL					IDD_PANEL_VIZ
	#undef  IDS_CLASSNAME
	#define IDS_CLASSNAME			IDS_CLASSNAME_VIZ
	#undef  IDS_TITLE
	#define IDS_TITLE					IDS_TITLE_VIZ
	#undef  IDS_NOEXE
	#define IDS_NOEXE					IDS_NOEXE_VIZ
	#undef  IDS_EXE_FILENAME
	#define IDS_EXE_FILENAME	IDS_EXE_FILENAME_VIZ

#endif

#endif	// _RESOURCEOVERRIDE_H