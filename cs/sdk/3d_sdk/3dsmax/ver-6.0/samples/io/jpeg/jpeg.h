//-----------------------------------------------------------------------------
// ----------------
// File ....: jpeg.h
// ----------------
// Author...: Gus J Grubba
// Date ....: July 1995
// Descr....: JPEG File I/O Module
//
// History .: Jul, 27 1995 - Started
//            
//
//
//-----------------------------------------------------------------------------
        
#ifndef _JPEGCLASS_
#define _JPEGCLASS_

#include <buildver.h>
#include "fmtspec.h"

#define DLLEXPORT __declspec(dllexport)

//-- Constants ----------------------------------------------------------------

#define JPEGCONFIGNAME _T("jpeg.cfg")
#define JPEGSECTION    _T("Default State")
#define JPEGQFACTORKEY _T("QFactor")
#define JPEGSMOOTHKEY  _T("Smoothing")
#define JPEGCODEKEY    _T("Coding")
#define JPEGDEFQ       _T("75")
#define JPEGDEFS       _T("0")
#define JPEGDEFC       _T("huffman")

//-----------------------------------------------------------------------------
//-- Class Definition ---------------------------------------------------------
//

class BitmapIO_JPEG : public BitmapIO {

     private:

        #define ARITH_CODING   1
        #define HUFFMAN_CODING 0
    
        JPEGUSERDATA   UserData;
        
     public:
     
        //-- Constructors/Destructors
        
                       BitmapIO_JPEG       ( );
                      ~BitmapIO_JPEG       ( );
               
        //-- Number of extemsions supported
		int            ExtCount() { return 3; }
		const TCHAR   *Ext(int n)
		{
			switch(n)
			{
			default:
			case 0:
				return _T("jpg");
			case 1:
				return _T("jpe");
			case 2:
				return _T("jpeg");
			}
		}
        
        //-- Descriptions
        
        const TCHAR   *LongDesc           ( );
        const TCHAR   *ShortDesc          ( );

        //-- Miscelaneous Messages
        
        const TCHAR   *AuthorName         ( )       { return _T("Gus J Grubba");}
        const TCHAR   *CopyrightMessage   ( )       { return _T("Copyright 1995, Yost Group");}
        const TCHAR   *OtherMessage1      ( )       { return _T("");}
        const TCHAR   *OtherMessage2      ( )       { return _T("");}
        
        unsigned int   Version            ( )       { return (JPEGVERSION);}

        //-- Driver capabilities
        
        int            Capability         ( )       { return BMMIO_READER    | 
                                                             BMMIO_WRITER    | 
                                                             BMMIO_EXTENSION |
                                                             BMMIO_CONTROLWRITE; }
        
        //-- Driver Configuration
        
        BOOL           LoadConfigure		( void *ptr );
        BOOL           SaveConfigure		( void *ptr );
        DWORD          EvaluateConfigure	( );

		void			GetCfgFilename		( TCHAR *filename );
		BOOL 			ReadCfg				( ); 
		void 			WriteCfg			( ); 
        
        //-- Show DLL's "About..." box
        
        void           ShowAbout          ( HWND hWnd );  

        //-- Show DLL's Control Panel
        
        BOOL           ShowControl        ( HWND hWnd, DWORD flag );

        //-- Return info about image
        
        BMMRES         GetImageInfo       ( BitmapInfo *fbi );        

        //-- Image Input
        
        BitmapStorage *Load               ( BitmapInfo *fbi, Bitmap *map, BMMRES *status);

        //-- Image Output
        
        BMMRES         OpenOutput         ( BitmapInfo *fbi, Bitmap *map );
        BMMRES         Write              ( int frame );
        int            Close              ( int flag );
        

        //-- This handler's specialized functions
        
        BOOL           Control            ( HWND ,UINT ,WPARAM ,LPARAM );

		JPEGUSERDATA*  GetConfiguration   ( )       { return &UserData; }
};


typedef struct _jpegheader {
     BYTE SOI[2];
     BYTE APP0[2];
     BYTE Length[2];
     BYTE Identifier[5];
     BYTE Version[2];
     BYTE Units;
     BYTE Xdensity[2];
     BYTE Ydensity[2];
     BYTE XThumbnail;
     BYTE YThumbnail;
} JPEGHEADER;     

#endif

//-- EOF: jpeg.h ---------------------------------------------------------------
