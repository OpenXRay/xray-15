/*******************************************************************
 *
 *    DESCRIPTION:	.PNG file-format I/O DLL Header file
 *
 *    AUTHOR:		Charlie Thaeler
 *    BASED ON RAS.H BY:		Keith Trummel
 *
 *    HISTORY:		09/27/95 Started coding
 *
 *******************************************************************/

// Here's the bitmap I/O class

//extern "C" {
#include "png.h"
//}

#define PNGCLASSID   0x6be260fb

typedef enum png_colors {
	PngPalette,
	PngRGB,
	PngRGBA,
	PngGray,
	PngGrayA
};

typedef struct png_cfg {
	png_colors color_type;
	int bitdepth;
	int interlaced;
	BOOL saved;
} png_cfg;

class BitmapIO_PNG : public BitmapIO {
private:
	png_struct		*png;
	png_info		*info;
	png_bytep		*row_pointers;
	FILE            *istream;
    FILE            *ostream;
    BitmapStorage   *ReadPNGFile (BitmapInfo *fbi, BitmapManager *mgr);

public:
	png_cfg			cfg;
	png_cfg			dlgcfg;
			BitmapIO_PNG();
			~BitmapIO_PNG();
    int				ExtCount() { return 1; }			// Number of extemsions supported
    const TCHAR *	Ext(int n) { return _T("png"); }	// Extension #n (i.e. "3DS")
    const TCHAR *	LongDesc();							// Long ASCII description 
    const TCHAR *	ShortDesc();						// Short ASCII description
    const TCHAR *	AuthorName();						// ASCII Author name
    const TCHAR *	CopyrightMessage();					// ASCII Copyright message
    unsigned int	Version() { return 96; }			// Version number * 100 (i.e. v3.01 = 301)
    int   	        Capability() 						// Returns read/write capability
						{return BMMIO_READER |
								BMMIO_WRITER |
								BMMIO_EXTENSION |
								BMMIO_CONTROLWRITE;}

    BOOL                LoadConfigure (void *ptr);
    BOOL                SaveConfigure (void *ptr);
    DWORD               EvaluateConfigure ();
	void				GetCfgFilename( TCHAR *filename );
	BOOL 				ReadCfg( ); 
	void 				WriteCfg( ); 

    void                ShowAbout (HWND hWnd);
    BOOL                ShowControl (HWND hWnd, DWORD flag);

    BMMRES	        GetImageInfo(BitmapInfo *info);	
    BitmapStorage *	Load (BitmapInfo *fbi, Bitmap *map, BMMRES *status);

    BMMRES	        OpenOutput(BitmapInfo *fbi, Bitmap *map); 
    BMMRES	        Save(const TCHAR *name,Bitmap *map); 
    BMMRES	        Write(int frame);		// Save image
    int		        Close(int option);		// Close file
};