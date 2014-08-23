//-----------------------------------------------------------------------------
// --------------------
// File ....: fmtspec.h
// --------------------
// Author...: Gus J Grubba
// Date ....: March 1997
// Descr....: JPEG File Format Specific Parameters
//
// History .: Mar, 27 1997 - Started
//
//-----------------------------------------------------------------------------
        
#ifndef _JPEGFMTSPEC_
#define _JPEGFMTSPEC_

//-----------------------------------------------------------------------------
//-- Jpeg data Structure ------------------------------------------------------
//

#define JPEGVERSION 200

typedef struct _jpeguserdata {
	DWORD  version;				//-- Reserved
	BYTE   qFactor;				//-- Quality Factor		( Min: 1 -> Max: 100)
	BYTE   Smooth;				//-- Smoothing Factor	( Min: 0 -> Max: 100)
	BOOL   userDataSaved;		//-- Reserved
} JPEGUSERDATA;

#endif

//-- EOF: fmtspec.h -----------------------------------------------------------
