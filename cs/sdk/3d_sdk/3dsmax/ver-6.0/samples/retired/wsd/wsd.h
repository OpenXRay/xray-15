//-----------------------------------------------------------------------------
// ----------------
// File ....: wsd.h
// ----------------
// Author...: Gus J Grubba
// Date ....: September 1995
// Descr....: WSD Image I/O Module
//
// History .: Sep, 20 1995 - Started
//
//-----------------------------------------------------------------------------
        
#ifndef _WSDCLASS_
#define _WSDCLASS_

#define DLLEXPORT __declspec(dllexport)

#include "fmtspec.h"

//-- Constants ----------------------------------------------------------------

#define WSDCONFIGNAME _T("wsd.ini")
#define WSDSECTION    _T("Default State")
#define WSDHOSTKEY    _T("Hostname")
#define WSDMXFRAMEKEY _T("Max Frames")
#define WSDSYSKEY     _T("System")
#define WSDDEFAULT    _T("accom")
#define WSDDEFSYS     _T("ntsc")

#define MAX_FRAMES		1860
#define LINE_TIMEOUT	5.0f

//-----------------------------------------------------------------------------
//-- Class Definition ---------------------------------------------------------
//

class BitmapIO_WSD : public BitmapIO {
    
     private:
        
        int     width;
        int     height;
        
        WORD    *rgbbuf;
        BYTE    *yuvbuf;

        TCHAR   hostname[MAX_PATH];
        BOOL    ntsc;
		int		maxframes;
        
        WSDDATA data;
        
        BMM_Color_64 *line;
        
        ConnectionInfo ci;
        
        //-- Private Methods
        
        void           RGBtoYUV           ( BMM_Color_64* rgb, BYTE *yuv, int len );
        void           YUVtoRGB           ( BMM_Color_64 *rgb, BYTE *yuv, int len );
        void           CleanUp            ( );
        void           GetCfgFilename     ( TCHAR *filename );
        int            ReadCfg            ( );
        void           WriteCfg           ( );
        int            Where              ( );
        BOOL           ControlConnect     ( HWND hWnd );
        BOOL           ShowSetup          ( HWND hWnd );
        
     public:
     
        //-- Constructors/Destructors
        
                       BitmapIO_WSD       ( );
                      ~BitmapIO_WSD       ( );
               
        //-- Number of extemsions supported
        
        int            ExtCount           ( )       { return 0;}
        
        //-- Extension #n (i.e. "3DS")
        
        const TCHAR   *Ext                ( int n ) {return _T("");}
        
        //-- Descriptions
        
        const TCHAR   *LongDesc           ( );
        const TCHAR   *ShortDesc          ( );

        //-- Miscelaneous Messages
        
        const TCHAR   *AuthorName         ( )       { return _T("Gus J Grubba");}
        const TCHAR   *CopyrightMessage   ( )       { return _T("Copyright 1994, 1996 Gus J Grubba");}
        unsigned int   Version            ( )       { return (WSDVERSION);}

        //-- Driver capabilities
        
        int            Capability         ( )       { return BMMIO_READER			| 
                                                             BMMIO_WRITER			|
                                                             BMMIO_RANDOM_ACCESS	|
                                                             BMMIO_MULTIFRAME		|
                                                             BMMIO_CONTROLREAD		| 
                                                             BMMIO_CONTROLWRITE		| 
                                                             BMMIO_EVALMATCH		| 
		    											 	 BMMIO_NON_CONCURRENT_ACCESS |
                                                             BMMIO_CONTROLGENERIC;}
                                                                   
        //-- Driver Configuration
        
        BOOL           LoadConfigure      ( void *ptr );
        BOOL           SaveConfigure      ( void *ptr );
        DWORD          EvaluateConfigure  ( );
        
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

        //-- Dialog Procs
        
        BOOL           Setup              ( HWND,UINT,WPARAM,LPARAM );
        BOOL           Control            ( HWND,UINT,WPARAM,LPARAM );
        
        void			EvalMatch		   ( TCHAR *matchString );

};

#endif

//-- EOF: wsd.h ---------------------------------------------------------------
