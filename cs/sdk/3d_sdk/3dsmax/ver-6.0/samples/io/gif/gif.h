//-----------------------------------------------------------------------------
// --------------------
// File ....: gif.h
// --------------------
// Author...: Tom Hudson
// Date ....: May 1995
// Descr....: Targa File I/O Module
//
// History .: Dec. 09 1995 - Started file
//            
//-----------------------------------------------------------------------------

#define GIFCLASSID 1234

#define DLLEXPORT __declspec(dllexport)

// Reasons for GIF load failure:
#define GIF_INVALID_SIGNATURE 1
#define GIF_TRUNCATED 2
#define GIF_INVALID_FILE 3
#define GIF_OUT_OF_MEMORY 4
#define GIF_WRITE_ERROR 5

#define MAX_CODES   4095
#define LARGEST_CODE	4095
#define TABLE_SIZE	(8*1024)

// GIF file stuff:

#pragma pack(1)		// Gotta pack these structures

typedef struct gif_header
	{
	char giftype[6];
	short w,h;
	unsigned char colpix;	/* flags */
	unsigned char bgcolor;
	unsigned char reserved;
	} GIFHeader;

typedef struct gif_GCB
	{
	unsigned char flags;
	unsigned short delay;
	unsigned char xparent;
	} GIFGraphicControlBlock;

// GCB flags bits
#define GCB_FLAGS_XPARENT 0x01

#define COLTAB	0x80
#define COLMASK 0x70
#define COLSHIFT 4
#define PIXMASK 7
#define COLPIXVGA13 (COLTAB | (5<<COLSHIFT) | 7)

typedef struct gif_image
	{
	short x,y,w,h;
	unsigned char flags;
	} GIFImage;

#define ITLV_BIT 0x40

#pragma pack()		// No more packing!

/* Various error codes used by decoder
 * and my own routines...   It's okay
 * for you to define whatever you want,
 * as long as it's negative...  It will be
 * returned intact up the various subroutine
 * levels...
 */
#define OUT_OF_MEMORY -10
#define BAD_CODE_SIZE -20
#define IMAGE_BUFFER_FULL -30
#define READ_ERROR -1
#define WRITE_ERROR -2
#define OPEN_ERROR -3
#define CREATE_ERROR -4
#define TOO_HIGH	-5

//-----------------------------------------------------------------------------
//-- Class Definition ---------------------------------------------------------
//

class BitmapIO_GIF : public BitmapIO {
    
     private:
     
        Bitmap *loadMap;
		BitmapStorage *loadStorage;
		BitmapStorage *saveStorage;
        FILE   *inStream;
        FILE   *outStream;

		GIFHeader header;
		GIFImage image;
		GIFGraphicControlBlock GCB;

		int storageType;

		BMM_Color_24 gif_cmap[256];				/* Raw GIF file color map */

		int badGIFReason;
		int bad_code_count;
		int gif_line, gif_colors;
		char iphase;
		int iy;
		WORD curr_size;                     /* The current code size */
		WORD clear;                         /* Value for a clear code */
		WORD ending;                        /* Value for a ending code */
		WORD newcodes;                      /* First available code */
		WORD top_slot;                      /* Highest code for current size */
		WORD slot;                          /* Last read code */

		/* The following static variables are used
		 * for seperating out codes
		 */
		WORD navail_bytes;                  /* # bytes left in block */
		WORD nbits_left;                    /* # bits left in current byte */
		BYTE b1;                            /* Current byte */
		BYTE *pbytes;                       /* Pointer to next byte in block */
		BYTE gif_byte_buff[256+3];          /* Current block */

		// GIF write variables
		BYTE *gif_wpt;
		long gif_wcount;

		jmp_buf recover;

		short *prior_codes;
		short *code_ids;
		BYTE *added_chars;

		short code_size;
		short clear_code;
		short eof_code;
		short bit_offset;
		short max_code;
		short free_code;
		
        //-- This handler's private functions

        BitmapStorage *ReadGIFFile        ( BitmapInfo *fbi, BitmapManager *manager, BMMRES *status);
		int GIFGetByte();
		int GIFOutLine(BYTE *pixels, int linelen);
		WORD InitExp(WORD size);
		WORD GetNextCode();
		int Decoder(WORD linewidth, BYTE *buf, BYTE *stack, BYTE *suffix, USHORT *prefix);
		int GIFDecoder(WORD linewidth);
		int ReadHeader();
		BitmapStorage * LoadGIFStuff(BitmapInfo *fbi, BitmapManager *manager );
		int SaveGIF(Bitmap *map);
		void InitTable(short min_code_size);
		void Flush(short n);
		void WriteCode(short code);
		short CompressData(int min_code_size);
		short GIFCompressData(int min_code_size);

     public:
     
        //-- Constructors/Destructors
        
                       BitmapIO_GIF       ( );
                      ~BitmapIO_GIF       ( ) {}

        //-- Number of extemsions supported
        
        int            ExtCount           ( )       { return 1;}
        
        //-- Extension #n (i.e. "3DS")
        
        const TCHAR   *Ext                ( int n ) { return _T("gif"); }
        
        //-- Descriptions
        
        const TCHAR   *LongDesc           ( );
        const TCHAR   *ShortDesc          ( );

        //-- Miscelaneous Messages
        
        const TCHAR   *AuthorName         ( )       { return _T("Tom Hudson");}
        const TCHAR   *CopyrightMessage   ( )       { return _T("Copyright 1995 Yost Group");}
        UINT           Version            ( )       { return (100);}

        //-- Driver capabilities
        
        int            Capability         ( )       { return BMMIO_READER    | 
        // Un-comment the following line to enable GIF writing:
//                                                             BMMIO_WRITER    | 
                                                             BMMIO_EXTENSION; }
        
        //-- Driver Configuration
        
        BOOL           LoadConfigure      ( void *ptr ) { return NULL; }
        BOOL           SaveConfigure      ( void *ptr ) { return NULL; }
        DWORD          EvaluateConfigure  ( ) { return 0; }
        
        //-- Show DLL's "About..." box
        
        void           ShowAbout          ( HWND hWnd );  

        //-- Return info about image
        
        BMMRES         GetImageInfo       ( BitmapInfo *fbi );        

        //-- Image Input
        
        BitmapStorage *Load               ( BitmapInfo *fbi, Bitmap *map, BMMRES *status);

        //-- Image Output
        
        BMMRES         OpenOutput         ( BitmapInfo *fbi, Bitmap *map );
        BMMRES         Write              ( int frame );
        int            Close              ( int flag );
  
};
