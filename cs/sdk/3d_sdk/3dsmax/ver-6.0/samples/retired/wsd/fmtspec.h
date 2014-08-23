//-----------------------------------------------------------------------------
// --------------------
// File ....: fmtspec.h
// --------------------
// Author...: Gus J Grubba
// Date ....: March 1997
// Descr....: WSD Device Specific Parameters
//
// History .: Mar, 27 1997 - Started
//
//-----------------------------------------------------------------------------
        
#ifndef _WSDFMTSPEC_
#define _WSDFMTSPEC_

//-----------------------------------------------------------------------------
//-- WSD data Structure -------------------------------------------------------
//

#define WSDVERSION    201

typedef struct tagWSDDATA {
	DWORD	version;			//-- Reserved
	int		startframe;			//-- Starting frame (Offset)
	BOOL	chromadither;		//-- Perform Chroma Dithering (TRUE or FALSE)
	char	hostname[MAX_PATH];
	BOOL    ntsc;
	int     height;
} WSDDATA;

#endif

//-- EOF: fmtspec.h -----------------------------------------------------------
