//-----------------------------------------------------------------------------
// ---------------------
// File ....: Negative.h
// ---------------------
// Author...: Gus J Grubba
// Date ....: September 1995
// Descr....: Negative Image Filter
//
// History .: Sep, 07 1995 - Started
//            
//
//
//-----------------------------------------------------------------------------
		 
#ifndef _NEGCLASS_
#define _NEGCLASS_

#define DLLEXPORT __declspec(dllexport)

//-----------------------------------------------------------------------------
//-- Class Definition ---------------------------------------------------------
//

class ImageFilter_Negative : public ImageFilter {

#define BLEND_CLASS_ID			Class_ID(0x123,0x456)
#define DEFAULT_BLEND_VALUE 	0.0f;

		Control *blendcontrol;

	  public:
	  
		//-- Constructors/Destructors
		
					   ImageFilter_Negative( ) {}
					  ~ImageFilter_Negative( ) {}
			   
		//-- Filter Info  ----------------------------------

		const TCHAR   	*Description        ( ) ;
		const TCHAR   	*AuthorName         ( ) { return _T("Gus J Grubba");}
		const TCHAR   	*CopyrightMessage   ( ) { return _T("Copyright 1996, Yost Group");}
		UINT           	Version             ( ) { return (200);}

		//-- Filter Capabilities ---------------------------
		
		DWORD          	Capability          ( ) { return( IMGFLT_FILTER | IMGFLT_MASK | IMGFLT_CONTROL ); }

		//-- Show DLL's About box --------------------------
		
		void           	ShowAbout           ( HWND hWnd );  
		BOOL           	ShowControl         ( HWND hWnd );  

		//-- Show Time -------------------------------------
		
		BOOL           	Render              ( HWND hWnd );

		//-- Node Tracking ---------------------------------

		void			FilterUpdate		( );
		
		//-- Local Methods --------------------------------
        
		BOOL  			Control	  			(HWND ,UINT ,WPARAM ,LPARAM );

};

#endif

//-- EOF: Negative.h ----------------------------------------------------------
