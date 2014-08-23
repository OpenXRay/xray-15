//-----------------------------------------------------------------------------
// ----------------
// File ....: ifl.h
// ----------------
// Author...: Gus J Grubba
// Date ....: October 1995
// Descr....: IFL File I/O Module
//
// History .: Oct, 20 1995 - Started
//            
//
//
//-----------------------------------------------------------------------------
        
#ifndef _IFLCLASS_
#define _IFLCLASS_

#define DLLEXPORT __declspec(dllexport)

#define IFLCONFIGNAME _T("ifl.cfg")
#define IFLVERSION 200

//-----------------------------------------------------------------------------
//-- Configuration Block ------------------------------------------------------
//

typedef struct tagIFLDATA {
     DWORD	version;
     TCHAR	editor[MAX_PATH];
} IFLDATA;

//-----------------------------------------------------------------------------
//-- Class Definition ---------------------------------------------------------
//

class BitmapIO_IFL : public BitmapIO {
    
     private:
     
		HANDLE	edtProcess;
		IFLDATA	data;

		Bitmap *loadMap;
		FILE   *inStream;
		FILE   *iflFile;
		int     width;
		int     height;
        
        //-- This handler's private functions
        
		int          	NumberOfFrames     	(TCHAR *file);
		int         	FirstNonBlank      	(TCHAR *s);
		void    	    GetIFLFile       	(const TCHAR *file, int frame, TCHAR *filenameOut);
		void			KillProcess			( );
       BOOL           	EditIFL          	( HWND hWnd, BitmapInfo *bi );
		BOOL			ReadCfg				( );
		void			WriteCfg			( );
		void			GetCfgFilename		( TCHAR *filename );

     public:
     
        //-- Constructors/Destructors
        
                       BitmapIO_IFL       ( );
                       //BitmapIO_IFL       ( BitmapStorage *s,BitmapIO *previous,int frame );
                      ~BitmapIO_IFL       ( );
               
        //-- Number of extemsions supported
        
        int            ExtCount           ( )       { return 1;}
        
        //-- Extension #n (i.e. "3DS")
        
        const TCHAR   *Ext                ( int n ) {return _T("ifl");}
        
        //-- Descriptions
        
        const TCHAR   *LongDesc           ( );
        const TCHAR   *ShortDesc          ( );

        //-- Miscelaneous Messages
        
        const TCHAR   *AuthorName         ( )       { return _T("Gus J Grubba");}
        const TCHAR   *CopyrightMessage   ( )       { return _T("Copyright 1995 Yost Group");}
        const TCHAR   *OtherMessage1      ( )       { return _T("");}
        const TCHAR   *OtherMessage2      ( )       { return _T("");}
        
        unsigned int   Version            ( )       { return (0200);}

        //-- Driver capabilities
        
        int            Capability         ( )       { return	BMMIO_READER    | 
                                                            	BMMIO_EXTENSION |
                                                            	//BMMIO_INFODLG    |
                                                            	//BMMIO_OWN_VIEWER |
															 	//BMMIO_CONTROLREAD |
                                                            	BMMIO_IFL; }
                                                                   
        //-- Driver Configuration
        
        BOOL           LoadConfigure      ( void *ptr );
        BOOL           SaveConfigure      ( void *ptr );
        DWORD          EvaluateConfigure  ( ) { return 0; }
        
        //-- Show DLL's "About..." box
        
        void           ShowAbout          ( HWND hWnd );  

        //-- Show DLL's Control Panel
        
        BOOL           ShowControl        ( HWND hWnd, DWORD flag );

        //-- Return info about image
        
        //BMMRES         GetImageInfoDlg    ( HWND hWnd, BitmapInfo *fbi, const TCHAR *filename);
        BMMRES         GetImageInfo       ( BitmapInfo *bi );        

        //-- I/O Interface
        
        BMMRES         GetImageName       ( BitmapInfo *fbi, TCHAR *filename);
        
        BitmapStorage *Load               ( BitmapInfo *fbi, Bitmap *map, BMMRES *status );
        BMMRES         OpenOutput         ( BitmapInfo *bi,  Bitmap *map );
        BMMRES         Write              ( int frame );
        int            Close              ( int flag );


		BOOL		   Control			  (HWND,UINT,WPARAM,LPARAM);


};

#endif

//-- EOF: ifl.h ---------------------------------------------------------------
