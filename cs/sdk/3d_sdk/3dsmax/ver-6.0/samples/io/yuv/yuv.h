//-----------------------------------------------------------------------------
// ----------------
// File ....: yuv.h
// ----------------
// Author...: Gus J Grubba
// Date ....: July 1995
// Descr....: YUV File I/O Module
//
// History .: Jul, 27 1995 - Started
//            
//
//
//-----------------------------------------------------------------------------
        
#ifndef _YUVCLASS_
#define _YUVCLASS_

#define DLLEXPORT __declspec(dllexport)

//-- Constants ----------------------------------------------------------------

#define WIDTH   720
#define PHEIGHT 576
#define NHEIGHT 486

//-----------------------------------------------------------------------------
//-- Class Definition ---------------------------------------------------------
//

class BitmapIO_YUV : public BitmapIO {
    
     private:
     
        Bitmap *loadMap;    // Set when loading a bitmap
        FILE   *inStream;   // Set when loading a bitmap
        
     public:
     
        //-- Constructors/Destructors
        
                       BitmapIO_YUV       ( );
                      ~BitmapIO_YUV       ( );
               
        //-- Number of extemsions supported
        
        int            ExtCount           ( )       { return 1;}
        
        //-- Extension #n (i.e. "3DS")
        
        const TCHAR   *Ext                ( int n ) {return _T("yuv");}
        
        //-- Descriptions
        
        const TCHAR   *LongDesc           ( );
        const TCHAR   *ShortDesc          ( );

        //-- Miscelaneous Messages
        
        const TCHAR   *AuthorName         ( )       { return _T("Gus J Grubba");}
        const TCHAR   *CopyrightMessage   ( )       { return _T("Copyright 1993, 1995 Gus J Grubba");}
        const TCHAR   *OtherMessage1      ( )       { return _T("");}
        const TCHAR   *OtherMessage2      ( )       { return _T("");}
        
        unsigned int   Version            ( )       { return (100);}

        //-- Driver capabilities
        
        int            Capability         ( )       { return BMMIO_READER    | 
                                                             BMMIO_EXTENSION |
                                                             BMMIO_CONTROLWRITE; }
        
        //-- Driver Configuration
        
        BOOL           LoadConfigure      ( void *ptr );
        BOOL           SaveConfigure      ( void *ptr );
        DWORD          EvaluateConfigure  ( )       { return 0; }
        
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
        
        void           YUVtoRGB           ( BMM_Color_64 *rgb, BYTE *yuv, int len  );

};

#endif

//-- EOF: yuv.h ---------------------------------------------------------------
