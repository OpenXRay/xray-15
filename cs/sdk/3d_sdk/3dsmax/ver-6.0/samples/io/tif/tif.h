//-----------------------------------------------------------------------------
// --------------------
// File ....: tif.h
// --------------------
// Author...: Tom Hudson
// Date ....: Feb 1996
// Descr....: TIF File I/O Module
//
// History .: Feb. 20, 1996 - Started file
//            February, 2001.  Major rewrite for TIFF 6.0 support - GL
//-----------------------------------------------------------------------------

#include "tiffconf.h"//compiler defines for libtiff.
#ifdef LZW_SUPPORT
// Uncomment the following when LZW compression is OK'd
//#define ALLOW_LZW_COMPRESSION
#endif


// Need to bracket the C-based library stuff with an explicit declaration:
#ifdef	__cplusplus
extern "C" {
#endif

#include "tiffio.h"
#include "tiffiop.h"

#ifdef	__cplusplus
	}
#endif

#ifdef GEOREFSYS_UVW_MAPPING 
#include "gtiffio.h"
#endif

// Pack all the tif structs
#pragma pack(1)

#define TIFCLASSID 0xfc12

#define DLLEXPORT __declspec(dllexport)

#define TIFVERSION 103		// 101 = Original release
							// 102 = No-compression fix 7/5/96
							// 103 = Rewrite for tiff 6.0 GL, Feb 2001.

#define TIFCONFIGNAME _T("tif.cfg")

#define BLOCKSIZE       65536      // changed from 16384 DS 2/15/99

#ifdef ALLOW_LZW_COMPRESSION
/* LZW decode parameters */

#define BITS_MIN    9           /* start with 9 bits */
#define BITS_MAX    12          /* max of 12 bit strings */

/* predefined codes for LZW compression */

#define CODE_CLEAR  256         /* code to clear string table */
#define CODE_EOI    257         /* end-of-information code */
#define CODE_FIRST  258         /* first free code entry */
#define	CODE_MAX	MAXCODE(BITS_MAX)
#define HSIZE       11252       /* 80% occupancy */
/* #define HSIZE       9001  */      /* 80% occupancy */

#define CHECK_GAP	10000		/* enc_ratio check interval */

static	uchar	 rmask[9] =
	{ 0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff };
static	uchar lmask[9] =
    { 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };

#undef HSIZE
#define	HSIZE		5003		/* 80% occupancy */
#define	HSHIFT		(8-(16-12))

/* LZW decompression status structure */
#define	LZW_HORDIFF4	0x01		/* hor. diff w/ 4-bit samples */
#define	LZW_HORDIFF8	0x02		/* hor. diff w/ 8-bit samples */
#define	LZW_HORDIFF16	0x04		/* hor. diff w/ 16-bit samples */
#define	LZW_HORDIFF32	0x08		/* hor. diff w/ 32-bit samples */
#define	LZW_RESTART	0x01		/* restart interrupted decode */

struct encode {
	int checkpoint;		/* point at which to clear table */
	long	ratio;			/* current compression ratio */
	long incount;		/* (input) data bytes encoded */
	long outcount;		/* encoded (output) bytes */
	int htab[HSIZE];		/* hash table */
	short	codetab[HSIZE];		/* code table */
   };

typedef struct {
	int  lzw_oldcode;			/* last code encountered */
	BYTE lzw_hordiff;
	WORD lzw_flags;
	WORD lzw_nbits;				/* number of bits/code */
	WORD lzw_stride;		/* horizontal diferencing stride */
	int   lzw_maxcode;			/* maximum code for lzw_nbits */
	long	lzw_bitoff;			 	/* bit offset into data */
	long	lzw_bitsize;			/* size of strip in bits */
	int	lzw_free_ent;			/* next free entry in hash table */
	union {
		struct encode enc;
		} u;
	} LZWState;

#define	enc_checkpoint	u.enc.checkpoint
#define	enc_ratio	u.enc.ratio
#define	enc_incount	u.enc.incount
#define	enc_outcount	u.enc.outcount
#define	enc_htab	u.enc.htab
#define	enc_codetab	u.enc.codetab

#endif//ALLOW_LZW_COMPRESSION

// Stop packing
#pragma pack()

//-----------------------------------------------------------------------------
//-- TIF data Structure -------------------------------------------------------
//

	//Possible storage types from the user interface
enum photometric
	{
	tif_write_mono,
	tif_write_color,
	tif_write_logl,
	tif_write_logluv
	};

//Possible compression types for a tiff image.
enum compression
	{
	tif_compress_none = 0,
	tif_compress_packbits,//usually bilevel images b/w
#ifdef ALLOW_LZW_COMPRESSION
	tif_compress_lzw//works for greyscale and color.
#endif
	};


//#define WRITE_MONO 0
//#define WRITE_COLOR 1

#ifdef GEOREFSYS_UVW_MAPPING //store geoInfo and matrix in .cfg file.
typedef struct _tifuserdata {
     DWORD  version;
	 BOOL	saved;
     BYTE   writeType;
	 BYTE   compressionType;
	 BOOL	writeAlpha;
	 BOOL	disableControl;//disable the standard tiff format popup dialog.
	 BOOL	lightActive;//if the next (or current) frame is being written for the lighting Analysis utility.
					//also force the data to be written using LogL.
	 float	lumStonits;//This is the physically based units factor for luminance-> used to convert data to candelas/meter^2.
	 BOOL	geoInfo;
	 Matrix3 matrix; 
	 BOOL operator==(const _tifuserdata& other);
	 // added a dpi field to structure
	 // David Cunningham, September 7, 2001
	 double dpi;
} TIFUSERDATA;
#else
typedef struct _tifuserdata {
     DWORD  version;
	 BOOL	saved;
     BYTE   writeType;
	 BYTE	compressionType;
	 BOOL	writeAlpha;

	//Specialized options for VIZ Lighting Analysis features using this format.
	 BOOL	disableControl;//disable the standard tiff format popup dialog.
	 BOOL	lightActive;//if the next (or current) frame is being written for the lighting Analysis utility.
					//also force the data to be written using LogL.
	 float	lumStonits;//This is the physically based units factor for luminance -> used to convert data to candelas/meter^2.
	 BOOL operator==(const _tifuserdata& other);
	 double dpi;
} TIFUSERDATA;
#endif  //not DESIGN_VER (max)

#define NO_STONITS	-1.0f

// Reasons for TIF load failure:

#define TIF_SAVE_OK					1
#define TIF_SAVE_WRITE_ERROR		0

#define TIFF_OPTIONS_INTERFACE Interface_ID(0x2a5f32b0, 0x504e42ef)
#define GetTiffInterface(cd) \
	(TiffInterface *)(cd)->GetFPInterface(TIFF_OPTIONS_INTERFACE)



#define GetTiffIOInterface(cd) \
	(BitmapIO_TIF*)(cd)->GetInterface(Interface_ID(TIFCLASSID,0))


//Enumeration of possible methods on the Tif interface.
enum { tif_get_photometric, 
		tif_set_photometric , 
		tif_get_show_control, 
		tif_set_show_control,
		tif_get_compression,
		tif_set_compression,
		tif_get_alpha,
		tif_set_alpha,
		tif_get_dpi,
		tif_set_dpi
 };

//Properties of the same interface.
enum { tif_photo , tif_show_control, tif_compression, tif_alpha, tif_test};


/************************************************************************
/*	
/*	This interface is mainly for the lighting analysis utility, however,
/*	it could be accessed by any client through MAXScript formatting tiff files properly.
/*	
/************************************************************************/

class TiffInterface : public FPStaticInterface
{
	public:

	virtual photometric GetPhotometric()=0;
	virtual void SetPhotometric(int newPhotometric)=0;

	virtual BOOL GetShowControl()=0;
	virtual void SetShowControl(BOOL show)=0;

	virtual compression GetCompression()=0;
	virtual void SetCompression(int newCompress)=0;
	
	virtual BOOL GetAlpha()=0;
	virtual void SetAlpha(BOOL onOff)=0;

	virtual double GetDPI()=0;
	virtual void SetDPI(double newDPI)=0;
};

//Tiff error handlers for this class...
static void MAXWarningHandler(const char* module, const char* fmt, va_list ap);
static void MAXErrorHandler(const char* module, const char* fmt, va_list ap);

//-----------------------------------------------------------------------------
//-- Class Definition ---------------------------------------------------------
//

class BitmapIO_TIF : public BitmapIO{
    
     private:
     
        Bitmap *loadMap;
		BitmapStorage *loadStorage;
		BitmapStorage *saveStorage;
        FILE   *inStream;
        FILE   *outStream;
		
		BOOL load_alpha;
		int nsamp;	// Number of samples per pixel
		int width, height;
		BYTE *loadbuf;
		TIFF *tif;
		TCHAR fileName[MAX_PATH];
		TIFFDirectory *td;

      TIFUSERDATA   UserData;

        //-- This handler's private functions

        BitmapStorage *ReadTIFFile( BitmapInfo *fbi, BitmapManager *manager, BMMRES *status);
		BitmapStorage *LoadTIFStuff(BitmapInfo *fbi, BitmapManager *manager );
     	BitmapStorage *TifReadLineArt(BitmapInfo *fbi, BitmapManager *manager);
     	BitmapStorage *TifReadGrayScale(BitmapInfo *fbi, BitmapManager *manager);
     	BitmapStorage *TifReadGrayScale16(BitmapInfo *fbi, BitmapManager *manager);
     	BitmapStorage *TifReadPlanarRGB(BitmapInfo *fbi, BitmapManager *manager);
     	BitmapStorage *TifReadChunkyRGB(BitmapInfo *fbi, BitmapManager *manager);
		void ScrunchColorMap(BMM_Color_48 *colpal);
    	BitmapStorage *TifReadColPal(BitmapInfo *fbi, BitmapManager *manager);
		BitmapStorage *TifReadLogLUV(BitmapInfo* fbi, BitmapManager* manager);

		// Write stuff:
		unsigned short rps, spp;
		long rawcc;
#ifdef ALLOW_LZW_COMPRESSION
		LZWState lzw_state;
		BYTE *comp_buf;
#endif
		BYTE *shortstrip;
		BMM_Color_64 *scanline, *scanptr;
		BOOL write_alpha;
		void MakeTiffhead();
 		int SaveTIF(/*FILE *stream*/);
		BOOL WriteTIF(FILE *stream);
		int LZWPreEncode(void);
		int LZWEncode(uchar *bp, int cc );
		int LZWPostEncode(void);
		void PutNextCode(int c);
		void ClearBlock(void);
		void ClearHash();

        void           GetCfgFilename     ( TCHAR *filename );
        BOOL           ReadCfg            ( );
        void           WriteCfg           ( );

     public:
     
        //-- Constructors/Destructors
        
                       BitmapIO_TIF       ( );
                      ~BitmapIO_TIF       ( );

        //-- Number of extemsions supported
        
		#ifndef GEOREFSYS_UVW_MAPPING 
        int            ExtCount           ( )       { return 1;}
		#else
		int            ExtCount           ( )       { return 2;}
		#endif 
        
        //-- Extension #n (i.e. "3DS")
        
		#ifndef GEOREFSYS_UVW_MAPPING 
        const TCHAR   *Ext                ( int n ) { return _T("tif"); }
		#else
		const TCHAR   *Ext                ( int n ) { return (n==1) ? _T("tiff") : _T("tif"); }
		#endif 
        
        //-- Descriptions
        
        const TCHAR   *LongDesc           ( );
        const TCHAR   *ShortDesc          ( );

        //-- Miscelaneous Messages
        
        const TCHAR   *AuthorName         ( )       { return _T("Tom Hudson");}
        const TCHAR   *CopyrightMessage   ( )       { return _T("Copyright 1996 Yost Group");}
        UINT           Version            ( )       { return (101);}//Changed from 100 GL March '01

        //-- Driver capabilities
        // If UserData.lightActive is set, then we will do multi-frame
        // tiff files, so make sure the Capability reflects this. CA - 312242
        int            Capability         ( )       { return UserData.lightActive
														? BMMIO_READER    | 
                                                             BMMIO_WRITER    | 
                                                             BMMIO_EXTENSION |
															 BMMIO_CONTROLWRITE |
															 BMMIO_MULTIFRAME
														: BMMIO_READER    | 
                                                             BMMIO_WRITER    | 
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
        
        BMMRES         GetImageInfo       ( BitmapInfo *fbi );        

        //-- Image Input

        /**
         * Note:
         * To extract LUMINANCE INFORMATION from the tif file, the client should first
         * call this Load method.  The TIFUSERDATA structure is stored in the BitmapInfo
         * parameter passed to the method.  
         * Then extract this data using this call:  
         * TIFUSERDATA* tifInfo= (TIFUSERDATA*)fbi->GetPiData();
         * The luminance value is stored in tifInfo.lumStonits.
         */
        
        BitmapStorage *Load               ( BitmapInfo *fbi, Bitmap *map, BMMRES *status);

        //-- Image Output
        
        BMMRES         OpenOutput         ( BitmapInfo *fbi, Bitmap *map );
        BMMRES         Write              ( int frame );
        int            Close              ( int flag );
  
        //-- This handler's specialized functions
        
        BOOL           Control            ( HWND ,UINT ,WPARAM ,LPARAM );

        friend class TiffInterfaceImp;
};
