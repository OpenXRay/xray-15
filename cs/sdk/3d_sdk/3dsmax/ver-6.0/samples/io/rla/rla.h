//-----------------------------------------------------------------------------
// ----------------
// File ....: rla.h
// ----------------
// Author...: Gus J Grubba
// Date ....: October 1995
// Descr....: RLA File I/O Module
//
// History .: Oct, 26 1995 - Started
//            
//
//
//-----------------------------------------------------------------------------
        
#ifndef _RLACLASS_
#define _RLACLASS_

#define DLLEXPORT __declspec(dllexport)

#define rlaCONFIGNAME	_T("rla.ini")
#define rpfCONFIGNAME	_T("rpf.ini")
#define rlaSECTION		_T("Default State")
#define rlaCHANNELS		_T("Channels")
#define rlaUSERGB		_T("Use RGB")
#define rlaUSEALPHA		_T("Use Alpha")
#define rlaPREMULTALPHA		_T("PreMult Alpha")
#define rlaRGB		_T("RGB16Bit")
#define rlaDESC		_T("Desc")
#define rlaUSER		_T("User")



#pragma pack(1)     // Gotta pack these structures!

struct RLASWindow {
	short left;
	short right;
	short bottom;
	short top;
	};



const unsigned short RLA_MAGIC_OLD = 0xFFFE;
const unsigned short RLA_MAGIC  = 0xFFFD; // started using this with R3.1 
const int RLA_Y_PAGE_SIZE = 32;


struct RLAHeader {
	RLASWindow window;
	RLASWindow active_window;
	short          frame;
	short          storage_type;
	short          num_chan;
	short          num_matte;
	short          num_aux;
	short          revision;
	char           gamma[16];
	char           red_pri[24];
	char           green_pri[24];
	char           blue_pri[24];
	char           white_pt[24];
	long           job_num;
	char           name[128];
	char           desc[128];
	char           program[64];
	char           machine[32];
	char           user[32];
	char           date[20];
	char           aspect[24];
	char           aspect_ratio[8];
	char           chan[32];
	short          field;
	char           time[12];
	char           filter[32];
	short          chan_bits;
	short          matte_type;
	short          matte_bits;
	short          aux_type;
	short          aux_bits;
	char           aux[32];
	char           space[36];
	long           next;
	};
#pragma pack()  

#include "fmtspec.h"

//-----------------------------------------------------------------------------
//-- Class Definition ---------------------------------------------------------
//

class BitmapIO_RLA : public BitmapIO {
    
     public:
		BOOL 				isRPF;
		RLAHeader			hdr;     
		RLAUSERDATA			UserData;
		FILE*				inStream;
		float 				gamma;
		float 				aspect;
		ULONG 				gbChannels;
		RenderInfo 			*rendInfo;
        
		void				GetCfgFilename 	( TCHAR *filename );
		BOOL 				ReadCfg		   	( );
		void 				WriteCfg	   	( );
		int  				ReadHeader	   	( );
		void 				InitHeader		(RLAHeader &h, int width, int height, float aspect, 
								BOOL doAlpha, ULONG gbChannels, RenderInfo *ri, BOOL saveLayerData, BOOL saveNameTab);
		BOOL				GetHDRData(BMM_Color_fl* in, BMM_Color_fl* out, int y, int width);


     public:
     
        //-- Constructors/Destructors
        
                       BitmapIO_RLA       (BOOL rlf=0);
                      ~BitmapIO_RLA       ( );
               
		BOOL 				Control				 ( HWND ,UINT ,WPARAM ,LPARAM );

        //-- Number of extemsions supported
        
        int            ExtCount           ( )       { return 1;}
        
        //-- Extension #n (i.e. "3DS")
        
        const TCHAR   *Ext                ( int n ) {return isRPF ? _T("rpf"):_T("rla"); }
        
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
                                                             BMMIO_INFODLG   |
                                                             BMMIO_EXTENSION |
                                                             BMMIO_CONTROLWRITE; }
        
        //-- Driver Configuration
        
        BOOL           LoadConfigure      ( void *ptr );
        BOOL           SaveConfigure      ( void *ptr );
        DWORD          EvaluateConfigure  ( );
        
        //-- Show DLL's "About..." box
        
        void           ShowAbout          ( HWND hWnd );  

        //-- Show DLL's Control Panel
        
        BOOL           ShowControl        ( HWND hWnd, DWORD flag );

        //-- Return info about image
        
        BMMRES   		GetImageInfo	( BitmapInfo *fbi );        
		BOOL 			ImageInfoDlg	(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
		BMMRES 			GetImageInfoDlg	(HWND hWnd, BitmapInfo *fbi, const TCHAR *filename);

        //-- Image Input
        
        BitmapStorage *Load               ( BitmapInfo *fbi, Bitmap *map, BMMRES *status);

        //-- Image Output
        
        BMMRES         OpenOutput         ( BitmapInfo *fbi, Bitmap *map );
        BMMRES         Write              ( int frame );
        int            Close              ( int flag );
        
        //-- Channels
        
        DWORD          ChannelsRequired   ( );

};


#endif

//-- EOF: rla.h ---------------------------------------------------------------
