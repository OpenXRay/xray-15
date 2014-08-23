//
//	mtlresOverride.h
//
//
//	This header file contains product specific resource override code.
//	It remaps max resource ID to derivative product alternatives.
//
//

#ifndef _MTLRESOVERRIDE_H
#define _MTLRESOVERRIDE_H


#ifdef WEBVERSION

	// Dialogs
	#undef  IDD_DMTL_SHADER4
	#define IDD_DMTL_SHADER4				IDD_DMTL_SHADER4_PLASMA

	#undef  IDD_DMTL_EXTRA6
	#define IDD_DMTL_EXTRA6					IDD_DMTL_EXTRA6_PLASMA

	#undef  IDD_DMTL_TEXMAP4
	#define IDD_DMTL_TEXMAP4				IDD_DMTL_TEXMAP4_PLASMA

	// Strings

	#undef  IDS_KE_SHADER
	#define IDS_KE_SHADER					IDS_KE_SHADER_PLASMA

	#undef  IDS_KE_BASIC_BLINN
	#define IDS_KE_BASIC_BLINN				IDS_KE_BASIC_BLINN_PLASMA

	

#endif	// WEBVERSION

#ifdef DESIGN_VER

	#ifdef RENDER_VER

		#undef IDD_BMTEX
		#define IDD_BMTEX					IDD_BMTEX_VIZR

		#undef IDD_NOISE
		#define IDD_NOISE					IDD_NOISE_VIZR

	#endif	// RENDER_VER
#endif	// DESIGN_VER

#endif	// _MTLRESOVERRIDE_H
