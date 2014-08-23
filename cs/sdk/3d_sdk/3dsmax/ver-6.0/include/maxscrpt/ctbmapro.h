/*	
 *		ctbmpro.h - protocol for MAX camera tracker bitmaps
 *
 *			Copyright © Autodesk, Inc 1998, John Wainwright
 *
 */
 //CTBitMap
	def_visible_primitive  ( openCTBitMap,		"openCTBitMap" ); 
	def_visible_primitive  ( selectCTBitMap,	"selectCTBitMap" ); 
	use_generic		  	   ( display,			"display" ); 
	use_generic			   ( unDisplay,			"unDisplay" ); 
	use_generic			   ( save,				"save" ); 
	use_generic            ( close,				"close" ); 
	use_generic            ( gotoFrame,			"gotoFrame" ); 
	use_generic            ( getTracker,		"getTracker" ); 
	use_generic            ( deleteTracker,		"deleteTracker" ); 
	def_visible_generic    ( perspectiveMatch,	 "perspectiveMatch" ); 
	def_visible_generic    ( loadFrames,		"loadFrames" ); 

	use_generic			   ( copy,				"copy" ); 
	use_generic            ( zoom,				"zoom" ); 
	def_visible_generic    ( resetZoom,			"resetZoom" ); 
	def_visible_generic    ( setFade,			"setFade" ); 

// CTMotionTracker
	use_generic            ( resample,			"resample");
	use_generic            ( reset,				"reset");
	def_visible_generic    ( setStruct,			"setStruct");
	def_visible_generic    ( showTrack,			"showTrack");
	def_visible_generic    ( invalTrack,		"invalTrack");
