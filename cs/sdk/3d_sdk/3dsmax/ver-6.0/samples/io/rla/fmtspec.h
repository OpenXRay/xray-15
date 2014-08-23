//-----------------------------------------------------------------------------
// --------------------
// File ....: fmtspec.h
// --------------------
// Author...: Gus J Grubba
// Date ....: March 1997
// Descr....: RLA File Format Specific Parameters
//
// History .: Mar, 27 1997 - Started
//
//-----------------------------------------------------------------------------
        
#ifndef _RLAFMTSPEC_
#define _RLAFMTSPEC_

//-----------------------------------------------------------------------------
//-- RLA data Structure -------------------------------------------------------
//
//	Channels are defined in gbuf.h:
//
//	BMM_CHAN_Z
//	BMM_CHAN_MTL_ID
//	BMM_CHAN_NODE_ID
//	BMM_CHAN_UV
//	BMM_CHAN_NORMAL
//	BMM_CHAN_REALPIX
//    etc..

#define RLAVERSION 301

struct RLAUSERDATA {
	DWORD	version;		//-- Reserved
	DWORD	channels;		//-- Bitmap with channels to save
	BOOL	usealpha;		//-- Save Alpha (if present)
	BYTE 	rgb;			//-- 0=8b, 1=16b, 2=32b
	BOOL 	defaultcfg;		//-- Reserved
	char 	desc[128];		//-- Description (ASCII)
	char 	user[32];		//-- User Name (ASCII)
	BOOL	premultAlpha;   //-- Premultiply Alpha

}; 

#endif

//-- EOF: fmtspec.h -----------------------------------------------------------
