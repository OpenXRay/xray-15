/*	
 *		bitmap_protocol.h - protocol for MAX bitmaps
 *
 *			Copyright © John Wainwright 1996
 *
 */
 
// BitMap

	def_visible_primitive( openBitMap,		"openBitMap" ); 
	def_visible_primitive( selectBitMap,	"selectBitMap" ); 
	def_visible_generic  ( display,			"display" ); 
	def_visible_generic  ( unDisplay,		"unDisplay" ); 
	def_visible_generic  ( save,			"save" ); 
	use_generic          ( close,			"close" ); 
	def_visible_generic  ( gotoFrame,		"gotoFrame" ); 
	def_visible_generic  ( getTracker,		"getTracker" ); 
	def_visible_generic  ( deleteTracker,	"deleteTracker" ); 
//	def_visible_generic  ( perspectiveMatch, "perspectiveMatch" ); 

	use_generic			 ( copy,			"copy" ); 
	def_visible_generic  ( zoom,			"zoom" ); 
	def_visible_generic  ( crop,			"crop" ); 
	def_visible_generic  ( setAsBackground, "setAsBackground" ); 

	def_visible_generic  ( getPixels,		 "getPixels" ); 
	def_visible_generic  ( setPixels,		 "setPixels" ); 
	def_visible_generic  ( getIndexedPixels, "getIndexedPixels" ); 
	def_visible_generic  ( setIndexedPixels, "setIndexedPixels" ); 
	def_visible_generic  ( getChannel,		 "getChannel" ); 
	def_visible_generic  ( getChannelAsMask, "getChannelAsMask" ); 

// BitMapWIndow
	
// MotionTracker
	
	def_visible_generic ( resample,			"resample");
	def_visible_generic ( reset,			"reset");
	def_visible_generic ( clearCacheEntry,  "clearCacheEntry");
	def_visible_generic ( setCacheEntry,    "setCacheEntry");
