//-----------------------------------------------------------------------------
// ---------------------
// File ....: Contrast.h
// ---------------------
// Author...: Gus J Grubba
// Date ....: September 1995
// Descr....: Contrast Image Filter
//
// History .: Sep, 07 1995 - Started
//            
//
//
//-----------------------------------------------------------------------------
		 
#ifndef _CONCLASS_
#define _CONCLASS_

#define DLLEXPORT __declspec(dllexport)
#define CONTRASTCLASSID NEGATIVECLASSID + 0x128

//-----------------------------------------------------------------------------
//-- Configuration Block ------------------------------------------------------
//

#define CONTRASTVERSION 100

typedef struct tagCONTRASTDATA {
     DWORD	version;
	  BOOL	absolute;
     float	contrast;
	  float	brightness;
} CONTRASTDATA;

//-----------------------------------------------------------------------------
//-- Class Definition ---------------------------------------------------------
//

class ImageFilter_Contrast : public ImageFilter {

		WORD			*lut;
		CONTRASTDATA	data;

		ISpinnerControl	*conspin;
		ISpinnerControl	*brispin;
	
		BOOL			BuildTable			( void );

	  public:
	  
		//-- Constructors/Destructors
		
						ImageFilter_Contrast ( );
						~ImageFilter_Contrast( ) { if (lut) LocalFree(lut); }
			   
		//-- Filter Info  ----------------------------------

		const TCHAR   	*Description        ( ) ;
		const TCHAR   	*AuthorName         ( ) { return _T("Gus J Grubba");}
		const TCHAR   	*CopyrightMessage   ( ) { return _T("Copyright 1993, 1996 Gus J Grubba");}
		UINT           	Version             ( ) { return (100);}

		//-- Filter Capabilities --------------------------
		
		DWORD          	Capability          ( ) { return( IMGFLT_FILTER | IMGFLT_CONTROL); }

		//-- Show DLL's Dialogue boxes --------------------
		
		void           	ShowAbout           ( HWND hWnd );  
		BOOL           	ShowControl         ( HWND hWnd );  

		//-- Show Time ------------------------------------
		
		BOOL           	Render              ( HWND hWnd );

        //-- Filter Configuration -------------------------
        
		BOOL           	LoadConfigure		( void *ptr );
		BOOL           	SaveConfigure		( void *ptr );
		DWORD          	EvaluateConfigure	( );
        
		//-- Local Methods --------------------------------
        
		BOOL  			Control	  			(HWND ,UINT ,WPARAM ,LPARAM );

};

#endif

//-- EOF: Contrast.h ----------------------------------------------------------
