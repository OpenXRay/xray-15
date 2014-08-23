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


#ifdef DESIGN_VER

	#ifdef RENDER_VER

		#undef IDD_SPECKLE
		#define IDD_SPECKLE					IDD_SPECKLE_VIZR

		#undef IDD_WATER
		#define IDD_WATER					IDD_WATER_VIZR

		#undef IDS_DS_WATER_PARAMS
		#define IDS_DS_WATER_PARAMS			IDS_DS_WATER_PARAMS_VIZR

		#undef IDS_DS_WATER
		#define IDS_DS_WATER				IDS_DS_WATER_VIZR

		#undef IDS_DS_WATER_CDESC
		#define IDS_DS_WATER_CDESC			IDS_DS_WATER_CDESC_VIZR
	#endif	// RENDER_VER

#endif	// DESIGN_VER

#endif
