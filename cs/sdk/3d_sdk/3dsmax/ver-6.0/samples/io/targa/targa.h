//-----------------------------------------------------------------------------
// --------------------
// File ....: targa.cpp
// --------------------
// Author...: Tom Hudson
// Date ....: May 1995
// Descr....: Targa File I/O Module
//
// History .: May, 31 1995 - Started
//            Oct, 23 1995 - Continued work (GG)
//            
//-----------------------------------------------------------------------------

#define DLLEXPORT __declspec(dllexport)

#include "fmtspec.h"

/* Defines for flags in TGA header */

#define UPPER_LEFT 0x20
#define LOWER_LEFT 0x00

#define TWO_INTERLEAVE 0x40
#define FOUR_INTERLEAVE 0x80

#define BASE            0
#define RUN             1
#define LITERAL         2

// .TGA file header

#pragma pack(1)     // Gotta pack these structures!
typedef struct
     {
     unsigned char idlen;
     unsigned char cmtype;
     unsigned char imgtype;

     unsigned short cmorg;
     unsigned short cmlen;
     unsigned char cmes;
     
     short xorg;
     short yorg;
     short width;
     short height;
     unsigned char pixsize;
     unsigned char desc;
     } TGAHeader;

typedef struct
     {
     short extsize;
     char authorname[41];
     char comments[324];
     short td_month;
     short td_day;
     short td_year;
     short td_hour;
     short td_minute;
     short td_second;
     char jobname[41];
     short jt_hours;
     short jt_minutes;
     short jt_seconds;
     char softwareID[41];
     short sw_version;
     char version_letter;
     char key_a,key_r,key_g,key_b;
     short aspect_w;
     short aspect_h;
     short gamma_numerator;
     short gamma_denominator;
     long color_corr_table;
     long postage_stamp;
     long scan_line;
     char alpha_attributes;
     } TGAExtra;

typedef struct
     {
     long ext_area;
     long dev_area;
     char signature[18];
     } TGAFooter;

#pragma pack()

//-- Constants ----------------------------------------------------------------

#define TGACONFIGNAME _T("targa.cfg")

//-----------------------------------------------------------------------------
// Targa save result codes

#define TGA_SAVE_OK					1
#define TGA_SAVE_UNSUPPORTED_TYPE		0
#define TGA_SAVE_WRITE_ERROR			-1

//-----------------------------------------------------------------------------
//-- Class Definition ---------------------------------------------------------
//

class BitmapIO_TGA : public BitmapIO {
    
     private:
     
        Bitmap *loadMap;
		BitmapStorage *saveStorage;
        FILE   *inStream;
        TGAHeader hdr;
        TGAFooter foot;
        TGAExtra extra;
        BOOL hflip, vflip;
		BOOL gotGamma;
		BitmapInfo *infoBI;		// Used ONLY by our custom Bitmap Info dialog

        float gamma;    // Gamma
        float aspect;   // Pixel aspect ratio
        
        //-- This handler's private functions

        int            ReadHeader         ( );
        int            ReadFooter         ( );
        
        BitmapStorage *ReadTGAFile        ( BitmapInfo *fbi, BitmapManager *manager, BMMRES *status);
        BitmapStorage *Load8BitPalTGA     ( BitmapInfo *fbi, FILE *stream, BitmapManager *manager);
        BitmapStorage *Load8BitGrayTGA    ( BitmapInfo *fbi, FILE *stream, BitmapManager *manager);
        BitmapStorage *Load16BitTGA       ( BitmapInfo *fbi, FILE *stream, BitmapManager *manager);
        BitmapStorage *Load24BitTGA       ( BitmapInfo *fbi, FILE *stream, BitmapManager *manager);
        BitmapStorage *Load32BitTGA       ( BitmapInfo *fbi, FILE *stream, BitmapManager *manager);
 		int            SaveAlpha          ( FILE *stream );	// Saves 8-bit of alpha chan
 		int            Save16BitTGA       ( FILE *stream );
 		int            Save24BitTGA       ( FILE *stream );
 		int            Save32BitTGA       ( FILE *stream );
		int            WriteTargaExtension ( FILE *stream, BOOL alphaUsed, BOOL preMultAlpha );
 
        void           GetCfgFilename     ( TCHAR *filename );
    public:

        TGAUSERDATA    UserData;
        BOOL           ReadCfg            ( );
        void           WriteCfg           ( );
     
        //-- Constructors/Destructors
        
                       BitmapIO_TGA       ( );
                      ~BitmapIO_TGA       ( ) {}

        //-- Number of extemsions supported
        
        int            ExtCount           ( )       { return 4;}
        
        //-- Extension #n (i.e. "3DS")
        
        const TCHAR   *Ext                ( int n );
        
        //-- Descriptions
        
        const TCHAR   *LongDesc           ( );
        const TCHAR   *ShortDesc          ( );

        //-- Miscelaneous Messages
        
        const TCHAR   *AuthorName         ( )       { return _T("Tom Hudson");}
        const TCHAR   *CopyrightMessage   ( )       { return _T("Copyright 1995 Yost Group");}
        UINT           Version            ( )       { return (100);}

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
        
        BMMRES         GetImageInfoDlg    ( HWND hWnd, BitmapInfo *fbi, const TCHAR *filename);
        BMMRES         GetImageInfo       ( BitmapInfo *fbi );        

        //-- Image Input
        
        BitmapStorage *Load               ( BitmapInfo *fbi, Bitmap *map, BMMRES *status);

        //-- Image Output
        
        BMMRES         OpenOutput         ( BitmapInfo *fbi, Bitmap *map );
        BMMRES         Write              ( int frame );
        int            Close              ( int flag );

        //-- This handler's specialized functions
        
        BOOL           ImageInfoDlg       ( HWND, UINT, WPARAM, LPARAM );
		BOOL           Control            ( HWND ,UINT ,WPARAM ,LPARAM );
        
};


// Handy-dandy pixel buffer classes:

template <class T> class PixelBufT {
private:
     T *buf;
     int width;
public:
     inline               PixelBufT(int width) { buf = (T *)calloc(width,sizeof(T)); this->width=width; };
     inline               ~PixelBufT() { if(buf) free(buf); };
     inline   T*          Ptr() { return buf; };
	 inline   T&          operator[](int i) { return buf[i]; }
           int            Fill(int start, int count, T color) {
                          int ix,jx=start+count;
                          if(jx >= width)
                             return 0;
                          for(ix=start; ix<jx; buf[ix++]=color);
                          return 1;
                          };
     };

typedef PixelBufT<UBYTE> PixelBuf8;
typedef PixelBufT<USHORT> PixelBuf16;
typedef PixelBufT<BMM_Color_24> PixelBuf24;
typedef PixelBufT<BMM_Color_32> PixelBuf32;
typedef PixelBufT<BMM_Color_48> PixelBuf48;
typedef PixelBufT<BMM_Color_64> PixelBuf64;

