//-----------------------------------------------------------------------------
// -----------------
// File ....: Wipe.h
// -----------------
// Author...: Gus J Grubba
// Date ....: February 1996
// Descr....: Wipe Transition
//
// History .: Feb, 18 1996 - Started
//            
//-----------------------------------------------------------------------------
        
#ifndef _WIPECLASS_
#define _WIPECLASS_

#define DLLEXPORT __declspec(dllexport)

//-----------------------------------------------------------------------------
//-- Configuration Block ------------------------------------------------------
//

#define WIPEVERSION 100

typedef struct tagWIPEDATA {
     DWORD	version;
     int	type;
	BOOL	reverse;
	BOOL	overlap;
} WIPEDATA;

//-----------------------------------------------------------------------------
//-- Class Definition ---------------------------------------------------------
//

class ImageFilter_Wipe : public ImageFilter {
    
     public:

		WIPEDATA			data;
     
        //-- Constructors/Destructors
        
                       ImageFilter_Wipe( );
                      ~ImageFilter_Wipe( ) {}
               
        //-- Filter Info  ---------------------------------

        const TCHAR   *Description         ( ) ;
        const TCHAR   *AuthorName          ( ) { return _T("Gus J Grubba");}
        const TCHAR   *CopyrightMessage    ( ) { return _T("Copyright 1996, Yost Group");}
        UINT           Version             ( ) { return (WIPEVERSION);}

        //-- Filter Capabilities --------------------------
        
        DWORD          Capability          ( ) { return(IMGFLT_COMPOSITOR | 
        																IMGFLT_FILTER 		| 
        																IMGFLT_CONTROL); }

        //-- Show DLL's About box -------------------------
        
        void           ShowAbout				( HWND hWnd );  
        BOOL           ShowControl				( HWND hWnd );

        //-- Show Time ------------------------------------
        
        BOOL           Render					( HWND hWnd );

        //-- Filter Configuration -------------------------
        
        BOOL           LoadConfigure			( void *ptr );
        BOOL           SaveConfigure			( void *ptr );
        DWORD          EvaluateConfigure		( );
        
        //-- Local Methods --------------------------------
        
		BOOL				Control					(HWND ,UINT ,WPARAM ,LPARAM );

};

#endif

//-- EOF: Wipe.h --------------------------------------------------------------
