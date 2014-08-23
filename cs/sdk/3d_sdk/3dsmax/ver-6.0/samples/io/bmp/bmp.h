//-----------------------------------------------------------------------------
// ----------------
// File ....: bmp.h
// ----------------
// Author...: Gus J Grubba
// Date ....: October 1995
// Descr....: BMP File I/O Module
//
// History .: Oct, 26 1995 - Started
//            
//
//
//-----------------------------------------------------------------------------
        
#ifndef _BMPCLASS_
#define _BMPCLASS_

#define DLLEXPORT __declspec(dllexport)

#define BMPCONFIGNAME _T("bmp.cfg")

//-----------------------------------------------------------------------------
//-- Class Definition ---------------------------------------------------------
//


typedef struct {
	int		outDepth;		// Output depth
	bool	saved;
}BMPParams;

class BitmapIO_BMP : public BitmapIO {
    
     private:
     
        BITMAPFILEHEADER	hdr;
        BITMAPINFOHEADER	bmi;
        
        void           GetCfgFilename     ( TCHAR *filename );

     public:
     
		BMPParams		mParams;
        BOOL			ReadCfg				( );
        void			WriteCfg			( );

        //-- Constructors/Destructors
        
                       BitmapIO_BMP       ( );
                      ~BitmapIO_BMP       ( );
               
        //-- Number of extemsions supported
        
        int            ExtCount           ( )       { return 1;}
        
        //-- Extension #n (i.e. "3DS")
        
        const TCHAR   *Ext                ( int n ) {return _T("bmp");}
        
        //-- Descriptions
        
        const TCHAR   *LongDesc           ( );
        const TCHAR   *ShortDesc          ( );

        //-- Miscelaneous Messages
        
        const TCHAR   *AuthorName         ( )       { return _T("Gus J Grubba");}
        const TCHAR   *CopyrightMessage   ( )       { return _T("Copyright 1995, Yost Group");}
        const TCHAR   *OtherMessage1      ( )       { return _T("");}
        const TCHAR   *OtherMessage2      ( )       { return _T("");}
        
        unsigned int   Version            ( )       { return (100);}

        //-- Driver capabilities
        
        int            Capability         ( )       { return BMMIO_READER    | 
                                                             BMMIO_WRITER    | 
                                                             BMMIO_EXTENSION |
															 BMMIO_CONTROLWRITE;}
        
        //-- Driver Configuration
        
        BOOL           LoadConfigure      ( void *ptr );
        BOOL           SaveConfigure      ( void *ptr );
        DWORD          EvaluateConfigure  ( ) { return sizeof(BMPParams); }
        
        //-- Show DLL's "About..." box
        
        void           ShowAbout          ( HWND hWnd );  

		//-- Show Image's control Dlg Box
		BOOL           ShowControl      ( HWND hWnd, DWORD flag );

        //-- Return info about image
        
        BMMRES         GetImageInfo       ( BitmapInfo *fbi );        

        //-- Image Input
        
        BitmapStorage *Load               ( BitmapInfo *fbi, Bitmap *map, BMMRES *status);

        //-- Image Output
        
        BMMRES         OpenOutput         ( BitmapInfo *fbi, Bitmap *map);
        BMMRES         Write              ( int frame );
        int            Close              ( int flag );
        
        //-- This handler's specialized functions
        
        int            ReadBimpHeader     ( FILE *stream );

		//-- Dialog Proc for the Image control Dlg box

		BOOL ConfigCtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) ;

};

#endif

//-- EOF: bmp.h ---------------------------------------------------------------
