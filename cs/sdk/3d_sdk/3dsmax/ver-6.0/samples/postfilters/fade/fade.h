//-----------------------------------------------------------------------------
// ---------------------
// File ....: Fade.h
// ---------------------
// Author...: Gus J Grubba
// Date ....: February 1996
// Descr....: Fade Image Filter
//
// History .: Feb, 21 1995 - Started
//            
//-----------------------------------------------------------------------------
        
#ifndef _FADECLASS_
#define _FADECLASS_

#define DLLEXPORT __declspec(dllexport)

//-----------------------------------------------------------------------------
//-- Configuration Block ------------------------------------------------------
//

#define FADEVERSION 100

typedef struct tagFADEDATA {
     DWORD	version;
     int 	type;
} FADEDATA;

//-----------------------------------------------------------------------------
//-- Class Definition ---------------------------------------------------------
//

class ImageFilter_Fade : public ImageFilter {
    
		FADEDATA			data;

     public:
     
        //-- Constructors/Destructors
        
                       ImageFilter_Fade( );
                      ~ImageFilter_Fade( ) {}
               
        //-- Filter Info  ---------------------------------

        const TCHAR   *Description         ( ) ;
        const TCHAR   *AuthorName          ( ) { return _T("Gus J Grubba");}
        const TCHAR   *CopyrightMessage    ( ) { return _T("Copyright 1996, Yost Group");}
        UINT           Version             ( ) { return (FADEVERSION);}

        //-- Filter Capabilities --------------------------
        
        DWORD          Capability          ( ) { return(IMGFLT_FILTER	| 
        																IMGFLT_MASK 	| 
        																IMGFLT_CONTROL); }

        //-- Show DLL's About box -------------------------
        
        void           ShowAbout           ( HWND hWnd );  
        BOOL           ShowControl         ( HWND hWnd );  

        //-- Show Time ------------------------------------
        
        BOOL           Render              ( HWND hWnd );

        //-- Filter Configuration -------------------------
        
        BOOL           LoadConfigure			( void *ptr );
        BOOL           SaveConfigure			( void *ptr );
        DWORD          EvaluateConfigure		( );
        
        //-- Local Methods --------------------------------
        
		BOOL				Control					(HWND ,UINT ,WPARAM ,LPARAM );

};

#endif

//-- EOF: Fade.h ----------------------------------------------------------
