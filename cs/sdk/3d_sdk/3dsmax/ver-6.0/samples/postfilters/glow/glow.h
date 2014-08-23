//-----------------------------------------------------------------------------
// ---------------------
// File ....: Glow.h
// ---------------------
// Author...: Gus J Grubba
// Date ....: September 1995
// Descr....: Glow Image Filter
//
// History .: Feb, 17 1996 - Started
//            
//-----------------------------------------------------------------------------
        
#ifndef _GLOWCLASS_
#define _GLOWCLASS_

#define DLLEXPORT __declspec(dllexport)

//-----------------------------------------------------------------------------
//-- Configuration Block ------------------------------------------------------
//

#define GLOWVERSION 101

typedef struct tagGLOWDATA {
	DWORD version;
	int	type;
	int	node;
	int	mtl;
	int	size;	
	BMM_Color_64 color;	
	int	colorsrc;	
} GLOWDATA;

//-----------------------------------------------------------------------------
//-- Class Definition ---------------------------------------------------------
//

class ImageFilter_Glow : public ImageFilter {
    
		GLOWDATA       data;

	public:
     
        //-- Constructors/Destructors
        
                       ImageFilter_Glow( );
                      ~ImageFilter_Glow( ) {}
               
        //-- Filter Info  ---------------------------------

        const TCHAR   *Description         ( );
        const TCHAR   *AuthorName          ( ) { return _T("Gus J Grubba, David C Thompson");}
        const TCHAR   *CopyrightMessage    ( ) { return _T("Copyright 1996, Yost Group");}
        UINT           Version             ( ) { return (GLOWVERSION);}

        //-- Filter Capabilities --------------------------
        
        DWORD          Capability          ( ) { return(IMGFLT_FILTER | IMGFLT_CONTROL); }

        //-- Show DLL's About & Control box ---------------
        
        void           ShowAbout           ( HWND hWnd );  
        BOOL           ShowControl         ( HWND hWnd );  

        //-- Showtime -------------------------------------
        
        BOOL           Render              ( HWND hWnd );

        //-- Filter Configuration -------------------------
        
        BOOL           LoadConfigure			( void *ptr );
        BOOL           SaveConfigure			( void *ptr );
        DWORD          EvaluateConfigure		( );
        DWORD          ChannelsRequired		( );

        //-- Local Methods --------------------------------
        
		BOOL				Control					(HWND ,UINT ,WPARAM ,LPARAM );
		void				HandleInputs			( HWND hWnd );
		void				GlowPixels				(int sx, int sy, BMM_Color_64 *curPix);

};

#endif

//-- EOF: Glow.h ----------------------------------------------------------
