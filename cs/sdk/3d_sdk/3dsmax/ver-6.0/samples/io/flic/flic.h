//-----------------------------------------------------------------------------
// -----------------
// File	....: flic.h
// -----------------
// Author...: Tom Hudson
// Date	....: November 1994
// Descr....: YUV File I/O Module
//
// History .: Nov, 22 1994 - Started
//			  Oct, 22 1995 - Taken over	(Gus J Grubba)
//
//-----------------------------------------------------------------------------
		
#ifndef	_FLICCLASS_
#define	_FLICCLASS_

#define	DLLEXPORT __declspec(dllexport)

#pragma	pack(1)
#include "pjbasics.h"
#pragma	pack()


// Old Autodesk	Animator 1.0 non-animated .CEL files stuff:

#define	CEL_MAGIC 0x09119

// .CEL	file header

#pragma	pack(1)		// Gotta pack this structure!

typedef	struct {
	WORD magic;
	WORD width;
	WORD height;
	WORD offx;
	WORD offy;
	BYTE bits;
	BYTE compression;
	LONG size;
	BYTE filler[16];
	} CelHead;


#define	PAL_LOW	    0
#define	PAL_MED	    1
#define PAL_HI		2
#define	PAL_CUSTOM  3
#define	PAL_UNIFORM 4

#define PAL_HI_EXT	_T("TGA")	// default of FlicConfigData::hi_ext

#define	FLIC_VERSION 	5
#define FLICCONFIGNAME _T("flic.cfg")

#define MAXPALNAME	256

class FlicConfigData {
	public:
	int	version;
	int saved;
	int	palType;
	int	nPalCols;	
	TCHAR palFileName[MAXPALNAME];
	BMM_Color_48 custPal[256];
	void Init();
	FlicConfigData() { Init(); }
	};

#pragma	pack()


// Error codes returned	by ReadCelHeader:
#define	CELERR_READ_ERROR	0
#define	CELERR_BAD_MAGIC	-1
#define	CELERR_BAD_FORMAT	-2

//-----------------------------------------------------------------------------
//-- Class Definition ---------------------------------------------------------
//

class BitmapIO_FLIC	: public BitmapIO {
	
	 private:
	 
		//-- Input fields
	 
		Flic		   inflic;
		FlicRaster	  *prast;
		FILE		  *inStream;	// Used	for	CELs only
		CelHead		   CELhdr;
		BMM_Color_48   CELpalette[256];
		BOOL		   isOldCEL;
		TCHAR		   name[MAX_PATH];
		ISpinnerControl *iPalCols;

		FlicConfigData	config;

		//-- Pointer to	file position for this input instance

		long		   fileptr;	  
	 
		//-- Output	fields
	 
		Flic		   outflic;
		int			   rasterBytes;
		FlicRaster	  *thisRast;
		FlicRaster	  *lastRast;

		BMM_Color_48   cutpal[256];	 //-- Color cut palette: colors start in slot 0
		BMM_Color_48   outpal[256];	 //-- Output palette: arrange for windows
		BYTE 		   fixmap[256];  //-- maps cutpal to outpal
		BYTE 		   *remap; 		 // either  NULL or = fixmap.
		BMM_Color_64   bgColor;
		int			   palSlots;

		BOOL		   hiMakeName;   // Make default temporary filenames for PAL_HI output
		TSTR		   hiTempFile;   // Temporary filename for PAL_HI output
	 
		//-- Input & Output	fields
 
		AnimInfo	   finfo;
		int			   frame;
		
	 public:
	 
		//-- Constructors/Destructors
		
					   BitmapIO_FLIC	  (	);
					  ~BitmapIO_FLIC	  (	);
			   
		//-- Number	of extemsions supported
		
		int			   ExtCount			  (	);
		
		//-- Extension #n (i.e.	"3DS")
		
		const TCHAR	  *Ext				  (	int	n );
		
		//-- Descriptions
		
		const TCHAR	  *LongDesc			  (	);
		const TCHAR	  *ShortDesc		  (	);

		//-- Miscelaneous Messages
		
		const TCHAR	  *AuthorName		  (	);
		const TCHAR	  *CopyrightMessage	  (	);
		unsigned int   Version			  (	);

		//-- Driver	capabilities
		
		int				Capability		  (	);
		
		//-- Driver	Configuration
		
		void			GetCfgFilename		( TCHAR *filename );
		BOOL	 		LoadConfigure	   ( void *ptr );
		BOOL			SaveConfigure	   ( void *ptr );
		DWORD			EvaluateConfigure  ( )		 { 
			return sizeof(FlicConfigData); 
			}
		BOOL 			ReadCfg();
		void 			WriteCfg();
		
		//-- Show DLL's	"About..." box
		
		void			ShowAbout		  (	HWND hWnd );  

		//-- Return	info about image
		
		BMMRES			GetImageInfo		  (	BitmapInfo *fbi	);		  

		//-- Play AVI File
		
		BOOL		   ShowImage		  (	HWND hWnd, BitmapInfo *bi );

		//-- Show DLL's	Control	Panel
		
		BOOL		   ShowControl		  (	HWND hWnd, DWORD flag );

		//-- Image Input
		
		BitmapStorage *Load				  (	BitmapInfo *fbi, Bitmap	*map, BMMRES *status);

		//-- Image Output
		
		BMMRES		   OpenOutput		  (	BitmapInfo *fbi, Bitmap	*map );
		BMMRES		   Write			  (	int	frame );
		int			   Close			  (	int	flag );
		BOOL 		   GetPaletteFromFile (HWND hwnd, TCHAR *name, BOOL testOnly=FALSE);
		int 		   PostMed			  ( );
		int 		   PostHi			  ( );
		int			   TrueColorToPalette (const TCHAR* path, const TCHAR* basename, const TCHAR* ext, int nimages);
		int 		   InitOutputRasters  ( int width, int height);
		void 		   FreeOutputRasters  ( );
		
		//-- This handler's	specialized	functions
		
		Flic		  *FlicPtr			  (	) {	return &inflic;	};
		AnimInfo	  *AnimInfoPtr		  (	) {	return &finfo;	};
		FlicRaster	  *RasterPtr		  (	) {	return prast;	};
		int			   StoreFrame		  (	int	frame, BitmapStorage *s);
		long		   FilePtr			  (	) {	return fileptr;	};
		Errcode		   CueFlic			  (	int	frame);
		int			   ReadCELHeader	  (	);		
		BOOL		   ControlDlg		  (	HWND,UINT,WPARAM,LPARAM	);
};

//-----------------------------------------------------------------------------
//-- File Class

class File {
	 public:
		FILE *stream;
		File(const TCHAR *name,	const TCHAR	*mode) { stream	= _tfopen(name,mode); }
		~File()	{ Close(); }
		void Close() { if(stream) fclose(stream); stream = NULL; }
	 };


//-- Misc flic stuff

typedef	unsigned char *PLANEPTR;

typedef	struct bmap	{
	 SHORT segment;		  /* this shouldn't	be here	but	makes
						 primitives	easier to work on MCGA & RAM...	*/
	 SHORT num_planes;		/* number of bplanes at	least 1	*/
	 LONG bpr;					/* bytes per row */
	 ULONG psize;		  /* size of a plane in	bytes (saves code) */
	 PLANEPTR bp[1];		/* at least	one	plane, the pixelated data */
} Bmap;

#endif
