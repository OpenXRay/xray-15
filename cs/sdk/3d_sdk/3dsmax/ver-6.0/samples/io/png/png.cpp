/**********************************************************************
 *
 *  FILE: png.cpp
 *
 *  DESCRIPTION:   .PNG file-format I/O DLL
 *
 *  CREATED BY: Charlie Thaeler 
 *  BASED ON ras.cpp BY: Keith Trummel 
 *
 *  (C) Copyright 1995-1996 by Autodesk, Inc.
 *
 *  This program is copyrighted by Autodesk, Inc. and is licensed to you under
 *  the following conditions.  You may not distribute or publish the source
 *  code of this program in any form.  You may incorporate this code in object
 *  form in derivative works provided such derivative works are (i.) are de-
 *  signed and intended to work solely with Autodesk, Inc. products, and (ii.)
 *  contain Autodesk's copyright notice "(C) Copyright 1994 by Autodesk, Inc."
 *
 *  AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS.  AUTODESK SPE-
 *  CIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
 *  A PARTICULAR USE.  AUTODESK, INC.  DOES NOT WARRANT THAT THE OPERATION OF
 *  THE PROGRAM WILL BE UNINTERRUPTED OR ERROR FREE.
 *
 **********************************************************************/
/** include files **/

#define STRICT
#include <stdio.h>
#include "max.h"
#include "IParamb2.h"
#include "bmmlib.h"
#include "pixelbuf.h"
#include "pnginc.h"
#include "resource.h"

#define NO_124OUT

#define MAX_STRING_LENGTH   256
#define PNG_VERSION         "0.95"

HINSTANCE hInstance;
HINSTANCE hResource;
int controlsInit = FALSE;

/** local definitions **/
TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

static TCHAR ThisExtList[] = _T("png");


BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
{
    hInstance = hinstDLL;
    hResource = hinstDLL;

    if ( !controlsInit ) {
        controlsInit = TRUE;
                
        // initialize Chicago controls
        InitCommonControls();
        
        InitCustomControls (hResource);
    }

    switch(fdwReason) {
    case DLL_PROCESS_ATTACH:
		break;
    case DLL_THREAD_ATTACH:
		break;
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
		break;
    }
    return(TRUE);
}


//------------------------------------------------------

class PNGClassDesc:public ClassDesc2 {
public:
    int				IsPublic()					{ return 1; }
    void *			Create(BOOL loading=FALSE)	{ return new BitmapIO_PNG; }
    const TCHAR *	ClassName()					{ return GetString(IDS_CLASS_NAME); }
    SClass_ID		SuperClassID()				{ return BMM_IO_CLASS_ID; }
    Class_ID		ClassID()					{ return Class_ID(PNGCLASSID,0); }
    const TCHAR *	Category()					{ return GetString(IDS_CATEGORY); }
	const TCHAR    *InternalName ( )			{ return _T("pngio"); }
	HINSTANCE		HInstance    ( )			{ return hInstance; }

};

static PNGClassDesc PNGDesc;

//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() 
{
    static int loaded = 0;
    static TCHAR stringBuf[MAX_STRING_LENGTH];

    if (! loaded) {
        LoadString (hResource, IDS_LIB_DESCRIPTION, stringBuf, 
                    MAX_STRING_LENGTH);
        loaded = 1;
    }

    return stringBuf;
}

__declspec( dllexport ) int
LibNumberClasses() 
{ 
    return 1; 
}

__declspec( dllexport ) ClassDesc *
LibClassDesc(int i) 
{
    switch(i) {
    case 0:
		return &PNGDesc;
		break;
    default:
		return 0;
		break;
    }
}

__declspec( dllexport ) ULONG 
LibVersion ( )  
{ 
    return ( VERSION_3DSMAX ); 
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

// Function published July 2000 - CCJ
// FP Interface
class BitmapIO_Png_Imp : public IBitmapIO_Png {	
public:
	int		GetType();
	void	SetType(int type);
	BOOL	GetAlpha();
	void	SetAlpha(BOOL alpha);
	BOOL	GetInterlaced();
	void	SetInterlaced(BOOL interlaced);

	// function IDs 
	enum { pngio_getType, pngio_setType, pngio_getAlpha, pngio_setAlpha, pngio_getInterlaced, pngio_setInterlaced }; 
	// enum IDs 
	enum { pngio_type }; 

	DECLARE_DESCRIPTOR(BitmapIO_Png_Imp) 

	// dispatch map
	BEGIN_FUNCTION_MAP
		FN_0(pngio_getType, TYPE_ENUM, GetType);
		VFN_1(pngio_setType, SetType, TYPE_ENUM);

		FN_0(pngio_getAlpha, TYPE_BOOL, GetAlpha);
		VFN_1(pngio_setAlpha, SetAlpha, TYPE_BOOL);

		FN_0(pngio_getInterlaced, TYPE_BOOL, GetInterlaced);
		VFN_1(pngio_setInterlaced, SetInterlaced, TYPE_BOOL);

	END_FUNCTION_MAP 
	};

static BitmapIO_Png_Imp pngIOInterface(
		BMPIO_INTERFACE, _T("ipngio"), IDS_PNGIO_INTERFACE, &PNGDesc, 0,
			BitmapIO_Png_Imp::pngio_getType, _T("getType"), IDS_PNGIO_GETBITMAPTYPE, TYPE_ENUM, BitmapIO_Png_Imp::pngio_type, 0, 0, 
			BitmapIO_Png_Imp::pngio_setType, _T("setType"), IDS_PNGIO_SETBITMAPTYPE, TYPE_VOID, 0, 1, 
			_T("type"), 0, TYPE_ENUM, BitmapIO_Png_Imp::pngio_type,

			BitmapIO_Png_Imp::pngio_getAlpha, _T("getAlpha"), IDS_PNGIO_GETALPHA, TYPE_BOOL, 0, 0, 
			BitmapIO_Png_Imp::pngio_setAlpha, _T("setAlpha"), IDS_PNGIO_SETALPHA, TYPE_VOID, 0, 1, 
			_T("useAlpha"), 0, TYPE_BOOL,

			BitmapIO_Png_Imp::pngio_getInterlaced, _T("getInterlaced"), IDS_PNGIO_GETINTERLACED, TYPE_BOOL, 0, 0, 
			BitmapIO_Png_Imp::pngio_setInterlaced, _T("setInterlaced"), IDS_PNGIO_SETINTERLACED, TYPE_VOID, 0, 1, 
			_T("Interlaced"), 0, TYPE_BOOL,

		enums,
			BitmapIO_Png_Imp::pngio_type, 5,
				"paletted",	BMM_PALETTED,
				"true24",	BMM_TRUE_24,
				"true48",	BMM_TRUE_48,
				"gray8",	BMM_GRAY_8,
				"gray16",	BMM_GRAY_16,
		end); 

int BitmapIO_Png_Imp::GetType()
	{
	int bmtype = BMM_NO_TYPE;

	BitmapIO_PNG* p = new BitmapIO_PNG;
	if (p) {
		p->ReadCfg();

		switch (p->cfg.color_type) {
			case PngPalette:
				bmtype = BMM_PALETTED;
				break;
			case PngRGB:
			case PngRGBA:
				if (p->cfg.bitdepth == 8) {
					bmtype = BMM_TRUE_24;
					}
				else if (p->cfg.bitdepth == 16) {
					bmtype = BMM_TRUE_48;
					}
				break;
			case PngGray:
			case PngGrayA:
				if (p->cfg.bitdepth == 8) {
					bmtype = BMM_GRAY_8;
					}
				else if (p->cfg.bitdepth == 16) {
					bmtype = BMM_GRAY_16;
					}
				break;
			}

		delete p;
		}
	return bmtype;
	}

void BitmapIO_Png_Imp::SetType(int type)
	{
	BitmapIO_PNG* p = new BitmapIO_PNG;
	if (p) {
		p->ReadCfg();

		bool hasAlpha = false;
		if ((p->cfg.color_type == PngRGBA) || (p->cfg.color_type == PngGrayA)) {
			hasAlpha = true;
			}

		switch (type) {
			case BMM_PALETTED:
				p->cfg.color_type = PngPalette;
				p->cfg.bitdepth = 8;
				break;
			case BMM_TRUE_24:
				p->cfg.color_type = hasAlpha ? PngRGBA : PngRGB;
				p->cfg.bitdepth = 8;
				break;
			case BMM_TRUE_48:
				p->cfg.color_type = hasAlpha ? PngRGBA : PngRGB;
				p->cfg.bitdepth = 16;
				break;
			case BMM_GRAY_8:
				p->cfg.color_type = hasAlpha ? PngGrayA : PngGray;
				p->cfg.bitdepth = 8;
				break;
			case BMM_GRAY_16:
				p->cfg.color_type = hasAlpha ? PngGrayA : PngGray;
				p->cfg.bitdepth = 16;
				break;
			}

		p->WriteCfg();

		delete p;
		}
	}

BOOL BitmapIO_Png_Imp::GetAlpha()
	{
	BOOL alpha = FALSE;

	BitmapIO_PNG* p = new BitmapIO_PNG;
	if (p) {
		p->ReadCfg();

		if ((p->cfg.color_type == PngRGBA) || (p->cfg.color_type == PngGrayA)) {
			alpha = TRUE;
			}

		delete p;
		}
	return alpha;
	}

void BitmapIO_Png_Imp::SetAlpha(BOOL alpha)
	{
	BitmapIO_PNG* p = new BitmapIO_PNG;
	if (p) {
		p->ReadCfg();
		if (alpha) {
			if (p->cfg.color_type == PngRGB) {
				p->cfg.color_type = PngRGBA;
				}
			else if (p->cfg.color_type == PngGray) {
				p->cfg.color_type = PngGrayA;
				}
			}
		else {
			if (p->cfg.color_type == PngRGBA) {
				p->cfg.color_type = PngRGB;
				}
			else if (p->cfg.color_type == PngGrayA) {
				p->cfg.color_type = PngGray;
				}
			}

		p->WriteCfg();

		delete p;
		}
	}

BOOL BitmapIO_Png_Imp::GetInterlaced()
	{
	BOOL interlaced = false;

	BitmapIO_PNG* p = new BitmapIO_PNG;
	if (p) {
		p->ReadCfg();

		interlaced = p->cfg.interlaced;

		delete p;
		}
	return interlaced;
	}

void BitmapIO_Png_Imp::SetInterlaced(BOOL interlaced)
	{
	BitmapIO_PNG* p = new BitmapIO_PNG;
	if (p) {
		p->ReadCfg();
		p->cfg.interlaced = interlaced;
		p->WriteCfg();

		delete p;
		}
	}

//
// Handy subroutines
//

static TCHAR plugin_str[MAX_STRING_LENGTH];
static TCHAR error_str[MAX_STRING_LENGTH];
static TCHAR warning_str[MAX_STRING_LENGTH];

static void
error_func(png_structp png, png_const_charp data)
{
	LoadString(hResource, IDS_PNG_PLUGIN, plugin_str, MAX_STRING_LENGTH);
	LoadString(hResource, IDS_PNG_ERROR, error_str, MAX_STRING_LENGTH);
	MessageBox(NULL, error_str,  plugin_str, MB_OK);
	longjmp(png->jmpbuf, TRUE);
}

static void
warning_func(png_structp png, png_const_charp data)
{
	LoadString(hResource, IDS_PNG_PLUGIN, plugin_str, MAX_STRING_LENGTH);
	LoadString(hResource, IDS_PNG_WARNING, warning_str, MAX_STRING_LENGTH);
	MessageBox(NULL, warning_str,  plugin_str, MB_OK);
}

/* Read an PNG file, returning the storage where it's located */
BitmapStorage *
BitmapIO_PNG::ReadPNGFile(BitmapInfo *fbi, BitmapManager *manager) 
{
    BitmapStorage *storage = NULL;
	unsigned char magic_numbers[8];
    
    if((istream=_tfopen(fbi->Name(), _T("rb")))==NULL)
		return NULL;

	// grab the first 8 bytes for testing
	if (fread(magic_numbers, 1, 8, istream) != 8) {
		fclose(istream);
		return NULL;
	} else 
		rewind(istream);

	// Make sure we're a png file
	if (!png_check_sig(magic_numbers, 8)) {
		fclose(istream);
		return NULL;
	}

    png = png_create_read_struct (PNG_VERSION, (void *) this, error_func, warning_func);
    if (setjmp(png->jmpbuf)) {
        if (info)
		    for (png_uint_32 i = 0; i < info->height; i++)
    			if (row_pointers[i]) free(row_pointers[i]);
		if (row_pointers) {
			free(row_pointers);
			row_pointers = NULL;
		}

		if (storage) {
			delete storage;
			storage = NULL;
		}

        fclose(istream);
        png_destroy_read_struct (&png, &info, NULL);
        return NULL;
    }
    info = png_create_info_struct(png);

    png_init_io(png, istream);
	png_read_info(png, info);

    fbi->SetWidth((WORD)info->width);
    fbi->SetHeight((WORD)info->height);
	if (info->valid & PNG_INFO_gAMA)
		fbi->SetGamma(info->gamma);
//	else
//		fbi->SetGamma (1.0f);
	if (info->valid & PNG_INFO_pHYs)
		fbi->SetAspect((float)info->x_pixels_per_unit / (float)info->y_pixels_per_unit);
	else
		fbi->SetAspect(1.0f);
    fbi->SetFlags(0);

	/* expand grayscale images to the full 8 bits */
	/* expand images with transparency to full alpha channels */
	/* I'm going to ignore lineart and just expand it to 8 bits */
	if ((info->color_type == PNG_COLOR_TYPE_PALETTE && info->bit_depth < 8) ||
		(info->color_type == PNG_COLOR_TYPE_GRAY && info->bit_depth < 8) ||
		(info->valid & PNG_INFO_tRNS))
		png_set_expand(png);

	int number_passes = 1;

	if (info->interlace_type)
		number_passes = png_set_interlace_handling(png);

	if (info->bit_depth == 16)
		png_set_swap(png);

	png_read_update_info(png, info);
   
	int bmtype = BMM_NO_TYPE;

	if (info->bit_depth == 1) {
			bmtype = BMM_LINE_ART;
	} else {
		switch(info->color_type) {
		case PNG_COLOR_TYPE_PALETTE:
			bmtype = BMM_PALETTED;
			break;
		case PNG_COLOR_TYPE_RGB:
		case PNG_COLOR_TYPE_RGB_ALPHA:
			switch(info->bit_depth) {
			case 2:
			case 4:
				// Not allowed
				break;
			case 8:
				bmtype = BMM_TRUE_32;  // zero alpha for those that don't have it
				break;
			case 16:
				bmtype = BMM_TRUE_64;
				break;
			}
			break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
		case PNG_COLOR_TYPE_GRAY:
			switch(info->bit_depth) {
			case 2:
			case 4:
				// we should never get here because of the expand code so drop through
				break;
			case 8:
				bmtype = BMM_GRAY_8;
				break;
			case 16:
				bmtype = BMM_GRAY_16;
				break;
			}
			break;
		}
	}

	if (bmtype == BMM_NO_TYPE) {
		fclose(istream);
        png_destroy_read_struct (&png, &info, NULL);
		return NULL;
	}

    // Create a storage for this bitmap...
	// (we may need to special case GRAY since it has a problem with alpha)
	if(info->color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		if (info->bit_depth == 16) {
			storage = BMMCreateStorage(manager, BMM_TRUE_64);
			fbi->SetType(BMM_TRUE_64);
		} else {
			storage = BMMCreateStorage(manager, BMM_TRUE_32);
			fbi->SetType(BMM_TRUE_32);
		}
	} else {
		storage = BMMCreateStorage(manager, bmtype);
		fbi->SetType(bmtype);
	}

	if (info->channels == 2 || info->channels == 4)
		fbi->SetFlags(MAP_HAS_ALPHA);


    if(storage == NULL) {
		fclose(istream);
        png_destroy_read_struct (&png, &info, NULL);
		return NULL;
	}

    if(storage->Allocate(fbi, manager, BMM_OPEN_R)==0) {
		delete storage;
		storage = NULL;
		fclose(istream);
        png_destroy_read_struct (&png, &info, NULL);
		return NULL;
	}

	row_pointers = (png_bytep *)malloc(info->height * sizeof(png_bytep));

	for (png_uint_32 i = 0; i < info->height; i++)
		row_pointers[i] = (png_bytep)malloc(info->rowbytes);

	// now read the image
	png_read_image(png, row_pointers);

	switch(bmtype) {
	case BMM_LINE_ART: {
		BMM_Color_64 *line64 = (BMM_Color_64 *) calloc(info->width, sizeof(BMM_Color_64));
		for (png_uint_32 iy = 0; iy < info->height; iy++) {
			BMM_Color_64 *l64 = line64;
			for (png_uint_32 ix = 0; ix < info->width; ix++,l64++) {
				png_uint_32 abyte = ix / 8;
				png_uint_32 abit = ix % 8;
				unsigned char tbyte = row_pointers[iy][abyte];
				unsigned char c = tbyte & (0x80 >> abit);
				l64->r = l64->g = l64->b = c ? 0xffff : 0;
				l64->a = 0;
			}
			storage->PutPixels(0, iy, info->width, line64);
		}
		free(line64);
		}
		break;
	case BMM_PALETTED: {
		if (info->bit_depth == 8) {
			for (png_uint_32 iy = 0; iy < info->height; iy++)
				storage->PutIndexPixels(0, iy, info->width, row_pointers[iy]);
		} else {
			unsigned char *pixels = (unsigned char *)calloc(info->width, sizeof(unsigned char));
			for (png_uint_32 iy = 0; iy < info->height; iy++) {
				// now fill a row of pixels
				unsigned char *inbyte = row_pointers[iy];
				for (png_uint_32 ix = 0; ix < info->width; inbyte++) {
					switch(info->bit_depth) {
					case 2:
						pixels[ix] = (*inbyte & 0xc0) >> 6;
						ix++; if (ix >= info->width) break;
						pixels[ix] = (*inbyte & 0x30) >> 4;
						ix++; if (ix >= info->width) break;
						pixels[ix] = (*inbyte & 0x0c) >> 2;
						ix++; if (ix >= info->width) break;
						pixels[ix] = *inbyte & 0x03;
						ix++;
						break;
					case 4:
						pixels[ix] = (*inbyte & 0xf0) >> 4;
						ix++; if (ix >= info->width) break;
						pixels[ix] = *inbyte & 0x0f;
						ix++;
						break;
					}
				}
				storage->PutIndexPixels(0, iy, info->width, pixels);
			}
			free(pixels);
		}
		// Now the palette
		PixelBuf48 palette(256);
		BMM_Color_48 *palout = palette.Ptr();
		for(int i = 0; i < png->num_palette; ++i,++palout) {
			palout->r = (USHORT)png->palette[i].red << 8;
			palout->g = (USHORT)png->palette[i].green << 8;
			palout->b = (USHORT)png->palette[i].blue << 8;
		}
		storage->SetPalette(0, png->num_palette, palette.Ptr());
		}
		break;
	case BMM_TRUE_32: {
		BMM_Color_64 *line64 = (BMM_Color_64 *) calloc(info->width, sizeof(BMM_Color_64));
		for (png_uint_32 iy = 0; iy < info->height; iy++) {
			BMM_Color_64 *l64 = line64;
			for (png_uint_32 ix = 0; ix < info->rowbytes; l64++) {
				l64->r = (unsigned short) (row_pointers[iy][ix++]) << 8;
				l64->g = (unsigned short) (row_pointers[iy][ix++]) << 8;
				l64->b = (unsigned short) (row_pointers[iy][ix++]) << 8;
				if (info->channels == 4) {
					l64->a = (unsigned short) (row_pointers[iy][ix++]) << 8;
				} else
					l64->a = 0;
			}
			storage->PutPixels(0, iy, info->width, line64);
		}
		free(line64);
		}
		break;
	case BMM_TRUE_64: {
		BMM_Color_64 *line64 = (BMM_Color_64 *) calloc(info->width, sizeof(BMM_Color_64));
		for (png_uint_32 iy = 0; iy < info->height; iy++) {
			BMM_Color_64 *l64 = line64;
			unsigned short *row = (unsigned short *) row_pointers[iy];
			for (png_uint_32 ix = 0; ix < info->width; ix++, l64++) {
				l64->r = *row++;
				l64->g = *row++;
				l64->b = *row++;
				if (info->channels == 4) {
					l64->a = *row++;
				} else
					l64->a = 0;
			}
			storage->PutPixels(0, iy, info->width, line64);
		}
		free(line64);
		}
		break;
	case BMM_GRAY_8: {
		BMM_Color_64 *line64 = (BMM_Color_64 *) calloc(info->width, sizeof(BMM_Color_64));
		for (png_uint_32 iy = 0; iy < info->height; iy++) {
			BMM_Color_64 *l64 = line64;
			for (png_uint_32 ix = 0; ix < info->rowbytes; l64++) {
				l64->r = l64->g = l64->b = (unsigned short) (row_pointers[iy][ix++]) << 8;
				if (info->channels == 2)
					l64->a = (unsigned short) (row_pointers[iy][ix++]) << 8;
				else
					l64->a = 0;
			}
			storage->PutPixels(0, iy, info->width, line64);
		}
		free(line64);
		}
		break;
	case BMM_GRAY_16: {
		BMM_Color_64 *line64 = (BMM_Color_64 *) calloc(info->width, sizeof(BMM_Color_64));
		for (png_uint_32 iy = 0; iy < info->height; iy++) {
			BMM_Color_64 *l64 = line64;
			unsigned short *row = (unsigned short *) row_pointers[iy];
			for (png_uint_32 ix = 0; ix < info->width; ix++, l64++) {
				l64->r = l64->g = l64->b = *row++;
				if (info->channels == 2) {
					l64->a = *row++;
				} else
					l64->a = 0;
			}
			storage->PutPixels(0, iy, info->width, line64);
		}
		free(line64);
		}
		break;
	}

	png_read_end(png, info);

	for (i = 0; i < info->height; i++)
		free(row_pointers[i]);

	free(row_pointers);

	fclose(istream);
    png_destroy_read_struct (&png, &info, NULL);

    return storage;
}

//
// BitmapIO_PNG class methods
//
static void InitUserData(png_cfg *cfg) {
	cfg->bitdepth = 16;
	cfg->color_type = PngRGBA;
	cfg->interlaced = FALSE;
	cfg->saved = FALSE;
}

BitmapIO_PNG::BitmapIO_PNG() 
{
	row_pointers = NULL;
    istream = NULL;
    ostream = NULL;
    png     = NULL;
    info    = NULL;
	InitUserData(&cfg);
}

BitmapIO_PNG::~BitmapIO_PNG() 
{
    if (istream)
		fclose (istream);
    if (ostream)
		fclose (ostream);
}


const TCHAR *
BitmapIO_PNG::LongDesc() 
{
	return GetString(IDS_LONGDESC);
}
	
const TCHAR *
BitmapIO_PNG::ShortDesc() 
{
	return GetString(IDS_SHORTDESC);
}

const TCHAR *
BitmapIO_PNG::AuthorName() 
{
    return GetString(IDS_AUTHOR);
}

const TCHAR *
BitmapIO_PNG::CopyrightMessage() 
{
	return GetString(IDS_COPYRIGHT);
}


// Configuration Functions
BOOL 
BitmapIO_PNG::LoadConfigure (void *ptr) 
{
    png_cfg *buf = (png_cfg *) ptr;
    memcpy(&cfg, ptr, sizeof(png_cfg));
	cfg.saved = TRUE;
    return TRUE;
}

BOOL 
BitmapIO_PNG::SaveConfigure (void *ptr) 
{
    if (ptr) {
		cfg.saved = TRUE;
		memcpy(ptr, &cfg, sizeof(png_cfg));
		return TRUE;
    } else
		return FALSE;
}

DWORD
BitmapIO_PNG::EvaluateConfigure () 
{
    return sizeof(png_cfg);
}

// Execution Functions
BMMRES 
BitmapIO_PNG::GetImageInfo(BitmapInfo *fbi) {
    BMMRES status = BMMRES_SUCCESS;
	unsigned char magic_numbers[8];

    // Use info->Name() to get name
    if((istream=_tfopen(fbi->Name(), _T("rb")))==NULL)
		return BMMRES_FILENOTFOUND;

	// grab the first 8 bytes for testing
	if (fread(magic_numbers, 1, 8, istream) != 8) {
		fclose(istream);
		return BMMRES_INVALIDFORMAT;
	} else 
		rewind(istream);

	// Make sure we're a png file
	if (!png_check_sig(magic_numbers, 8)) {
		fclose(istream);
		return BMMRES_INVALIDFORMAT;
	}

    png = png_create_read_struct (PNG_VERSION, (void *) this, error_func, warning_func);
    if (setjmp(png->jmpbuf)) {
        png_destroy_read_struct (&png, &info, NULL);
        fclose(istream);
        return BMMRES_INVALIDFORMAT;
    }
    info = png_create_info_struct(png);

	png_init_io(png, istream);
	png_read_info(png, info);

	// fill in the information.
	fbi->SetWidth((WORD)info->width);
	fbi->SetHeight((WORD)info->height);
	if (info->valid & PNG_INFO_gAMA)
		fbi->SetGamma(info->gamma);
//	else
//		fbi->SetGamma (1.0f);
	if (info->valid & PNG_INFO_pHYs)
		fbi->SetAspect((float)info->x_pixels_per_unit / (float)info->y_pixels_per_unit);
	else
		fbi->SetAspect(1.0f);
	fbi->SetFirstFrame(0);
	fbi->SetLastFrame(0);

	int bmtype = BMM_NO_TYPE;

	if (info->bit_depth == 1) {
		bmtype = BMM_LINE_ART;
	} else {
		switch(info->color_type) {
		case PNG_COLOR_TYPE_PALETTE:
			bmtype = BMM_PALETTED;
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			switch(info->bit_depth) {
			case 2:
			case 4:
				// Not allowed
				break;
			case 8:
				bmtype = BMM_TRUE_32;
				break;
			case 16:
				bmtype = BMM_TRUE_64;
				break;
			}
			break;
		case PNG_COLOR_TYPE_RGB:
			switch(info->bit_depth) {
			case 2:
			case 4:
				// Not allowed
				break;
			case 8:
				bmtype = BMM_TRUE_24;
				break;
			case 16:
				bmtype = BMM_TRUE_48;
				break;
			}
			break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
		case PNG_COLOR_TYPE_GRAY:
			switch(info->bit_depth) {
			case 2:
			case 4:
				bmtype = BMM_GRAY_8;  // call it 8 since there's no smaller
				break;
			case 8:
				bmtype = BMM_GRAY_8;
				break;
			case 16:
				bmtype = BMM_GRAY_16;
				break;
			}
			break;
		}
	}


	if (bmtype == BMM_NO_TYPE)
		status = BMMRES_INVALIDFORMAT;

	fbi->SetType(bmtype);

	fclose(istream);
    png_destroy_read_struct (&png, &info, NULL);
	return status;
}

BitmapStorage *
BitmapIO_PNG::Load(BitmapInfo *fbi, Bitmap *map, BMMRES *status) 
{
    if(openMode != BMM_NOT_OPEN) {
		*status = BMMRES_IOERROR;
		return NULL;
    }
    
    openMode = BMM_OPEN_R;
    
    BitmapStorage *s = ReadPNGFile(fbi, map->Manager());
    
	if(!s) {
	    *status = BMMRES_IOERROR;
	    return NULL;
	}
    
    // Set the storage's name
    
    s->bi.CopyImageInfo (fbi);
    
    *status = BMMRES_SUCCESS;
    return s;
}


BMMRES
BitmapIO_PNG::OpenOutput(BitmapInfo *fbi, Bitmap *map) 
{
    if(openMode != BMM_NOT_OPEN)
		return (ProcessImageIOError(fbi, BMMRES_INTERNALERROR));

    if(!map)
		return (ProcessImageIOError(fbi,BMMRES_INTERNALERROR));

	//-- Check for Default Configuration -----------------
	if (!cfg.saved)
		ReadCfg();

    //-- Save Image Info Data
    bi.CopyImageInfo(fbi);    
	bi.SetUpdateWindow(fbi->GetUpdateWindow());

    this->map   = map;
    openMode    = BMM_OPEN_W;

    return BMMRES_SUCCESS;
}

BMMRES
BitmapIO_PNG::Save(const TCHAR *filename, Bitmap *map) 
{
    if(!map)
		return(ProcessImageIOError(&bi,BMMRES_INTERNALERROR));
    
    openMode = BMM_OPEN_W;
    
    if((ostream = _tfopen(filename,_T("wb"))) == NULL)
		return (ProcessImageIOError(&bi));

	BitmapStorage *palettedStorage = NULL;

    png = png_create_write_struct (PNG_VERSION, (void *) this, error_func, warning_func);
    if (setjmp(png->jmpbuf)) {
		if (info)
            for (png_uint_32 i = 0; i < info->height; i++)
			    if (row_pointers[i]) free(row_pointers[i]);
		if (row_pointers) {
			free(row_pointers);
			row_pointers = NULL;
		}

		if (palettedStorage) delete palettedStorage;
        fclose(ostream);
		_tremove(filename);
        png_destroy_write_struct (&png, &info);
        return BMMRES_IOERROR;
    }
    info = png_create_info_struct(png);

    png_init_io(png, ostream);

	switch(cfg.color_type) {
	case PngPalette:
		info->color_type = PNG_COLOR_TYPE_PALETTE;
		info->pixel_depth = 8;
		info->valid |= PNG_INFO_PLTE;
		info->num_palette = 256;
		break;
	case PngRGB:
		info->color_type = PNG_COLOR_TYPE_RGB;
		break;
	case PngRGBA:
		info->color_type = PNG_COLOR_TYPE_RGB_ALPHA;
		break;
	case PngGray:
		info->color_type = PNG_COLOR_TYPE_GRAY;
		break;
	case PngGrayA:
		info->color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
		break;
	}
	info->width = map->Width();
	info->height = map->Height();
	if (OutputGamma() != 1.0f) {
		info->valid |= PNG_INFO_gAMA;
		info->gamma = OutputGamma();
	} else info->gamma = 1.0f;

	if (map->Aspect() != 1.0f) {
		info->valid |= PNG_INFO_pHYs;
		info->x_pixels_per_unit = (png_uint_32)(1024.0f * map->Aspect());
		info->y_pixels_per_unit = 1024;
		info->phys_unit_type = 0;
	}

	if (cfg.interlaced)
		info->interlace_type = 1;
	else
		info->interlace_type = 0;

	switch( info->color_type) {
	case PNG_COLOR_TYPE_PALETTE:
	case PNG_COLOR_TYPE_GRAY:
		info->channels = 1;
		break;
	case PNG_COLOR_TYPE_GRAY_ALPHA:
		info->channels = 2;
		break;
	case PNG_COLOR_TYPE_RGB:
		info->channels = 3;
		break;
	case PNG_COLOR_TYPE_RGB_ALPHA:
		info->channels = 4;
		break;
	}

	info->bit_depth = cfg.bitdepth;
	info->rowbytes = info->width * info->channels * info->bit_depth / 8;

	row_pointers = (png_bytep *)malloc(info->height * sizeof(png_bytep));
	for (png_uint_32 i = 0; i < info->height; i++)
		row_pointers[i] = (png_bytep)malloc(info->rowbytes);

	switch (info->bit_depth) {
	case 16:  // this is only RGB/RGBA/Gray/GrayA
		switch(info->color_type) {
		case PNG_COLOR_TYPE_RGB:
		case PNG_COLOR_TYPE_RGB_ALPHA:
			{
			BMM_Color_64 *line64 = (BMM_Color_64 *) calloc(info->width,sizeof(BMM_Color_64));
			for (png_uint_32 iy = 0; iy < info->height; ++iy) {
				if (GetOutputPixels(0, iy, info->width, line64) != 1) {
					for (png_uint_32 i = 0; i < info->height; i++)
						if (row_pointers[i]) free(row_pointers[i]);
					if (row_pointers) {
						free(row_pointers);
						row_pointers = NULL;
					}

					fclose(ostream);
					_tremove(filename);
					free(line64);
                    png_destroy_write_struct (&png, &info);
					return BMMRES_IOERROR;
				}
				BMM_Color_64 *l64=line64;
				unsigned short *oshort = (unsigned short *)row_pointers[iy];
				for (png_uint_32 ix = 0; ix < info->width; ++l64, ix++) {
					*oshort = (unsigned short)l64->r; oshort++;
					*oshort = (unsigned short)l64->g; oshort++;
					*oshort = (unsigned short)l64->b; oshort++;
					if (info->channels == 4) {
						*oshort = (unsigned short)l64->a; oshort++;
					}
				}
			}
			free(line64);
			}
			break;
		case PNG_COLOR_TYPE_GRAY:
			{
			for (png_uint_32 iy = 0; iy < info->height; ++iy)
				if (map->Get16Gray(0, iy, info->width, (unsigned short *)row_pointers[iy]) != 1) {
					for (png_uint_32 i = 0; i < info->height; i++)
						if (row_pointers[i]) free(row_pointers[i]);
					if (row_pointers) {
						free(row_pointers);
						row_pointers = NULL;
					}

					fclose(ostream);
					_tremove(filename);
                    png_destroy_write_struct (&png, &info);
					return BMMRES_IOERROR;
				}
			}
			break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			{
			BMM_Color_64 *line64 = (BMM_Color_64 *) calloc(info->width, sizeof(BMM_Color_64));
			unsigned short *line = (unsigned short *) calloc(info->width, sizeof(unsigned short));
			for (png_uint_32 iy = 0; iy < info->height; ++iy) {
				if (GetOutputPixels(0, iy, info->width, line64) != 1 ||
					map->Get16Gray(0, iy, info->width, line) != 1) {
					for (png_uint_32 i = 0; i < info->height; i++)
						if (row_pointers[i]) free(row_pointers[i]);
					if (row_pointers) {
						free(row_pointers);
						row_pointers = NULL;
					}
					free(line64);
					free(line);
					fclose(ostream);
					_tremove(filename);
                    png_destroy_write_struct (&png, &info);
					return BMMRES_IOERROR;
				}
				BMM_Color_64 *l64 = line64;
				unsigned short *l=line;
				unsigned short *oshort = (unsigned short *)row_pointers[iy];
				for (png_uint_32 ix = 0; ix < info->width; ix++, l64++) {
					*oshort++ = *l++;
					*oshort++ = (unsigned short)l64->a;
				}
			}
			free(line64);
			free(line);
			}
			break;
		}
		break;

	case 8: // this can be any type
		switch(info->color_type) {
		case PNG_COLOR_TYPE_PALETTE: {
			// Set up a palette buffer
			PixelBuf48 palettebuf(info->num_palette);
			BMM_Color_48 *pal = palettebuf.Ptr();
			// Must compute a color palette, and reduce the image to 256 colors!
			// this calculates a palette based on the gamma corrected values, which
			// corresponds to what GetOutputPixels returns.
			if(CalcOutputPalette(256, pal) == 0) {
				for (png_uint_32 i = 0; i < info->height; i++)
					if (row_pointers[i]) free(row_pointers[i]);
				if (row_pointers) {
					free(row_pointers);
					row_pointers = NULL;
				}
				fclose(ostream);
				_tremove(filename);
                png_destroy_write_struct (&png, &info);
				return BMMRES_IOERROR;
			}
			info->palette = (png_color *)malloc(info->num_palette * sizeof(png_color));
			for (int i = 0; i < info->num_palette; i++) {
				info->palette[i].red = (unsigned char)(pal[i].r >> 8);
				info->palette[i].green = (unsigned char)(pal[i].g >> 8);
				info->palette[i].blue = (unsigned char)(pal[i].b >> 8);
			}
			PixelBuf64 line(info->width);
			ColorPacker* cpack = BMMNewColorPacker(info->width, pal, info->num_palette);
			for (png_uint_32 iy=0; iy < info->height; ++iy) {
				if(!GetOutputPixels(0, iy, info->width, line.Ptr())) {
					for (png_uint_32 i = 0; i < info->height; i++)
						if (row_pointers[i]) free(row_pointers[i]);
					if (row_pointers) {
						free(row_pointers);
						row_pointers = NULL;
					}
					fclose(ostream);
					_tremove(filename);
                    png_destroy_write_struct (&png, &info);
					return BMMRES_IOERROR;
				}
				cpack->PackLine(line.Ptr(), row_pointers[iy], info->width);
			}
			cpack->DeleteThis();
			}
			break;
		case PNG_COLOR_TYPE_RGB:
		case PNG_COLOR_TYPE_RGB_ALPHA:
			{
			BMM_Color_32 *line32 = (BMM_Color_32 *) calloc(info->width,sizeof(BMM_Color_32));
			for (png_uint_32 iy = 0; iy < info->height; ++iy) {
				if (GetDitheredOutputPixels(0, iy, info->width, line32) != 1) {
					for (png_uint_32 i = 0; i < info->height; i++)
						if (row_pointers[i]) free(row_pointers[i]);
					if (row_pointers) {
						free(row_pointers);
						row_pointers = NULL;
					}

					fclose(ostream);
					_tremove(filename);
					free(line32);
                    png_destroy_write_struct (&png, &info);
					return BMMRES_IOERROR;
				}
				BMM_Color_32 *l32=line32;
				unsigned char *obyte = (unsigned char *)row_pointers[iy];
				for (png_uint_32 ix = 0; ix < info->width; ++l32, ix++) {
					*obyte = (unsigned char)l32->r; obyte++;
					*obyte = (unsigned char)l32->g; obyte++;
					*obyte = (unsigned char)l32->b; obyte++;
					if (info->channels == 4) {
						*obyte = (unsigned char)l32->a; obyte++;
					}
				}
			}
			free(line32);
			}
			break;
		case PNG_COLOR_TYPE_GRAY:
			{
			unsigned short *line = (unsigned short *) calloc(info->width * info->channels, sizeof(unsigned short));
			for (png_uint_32 iy = 0; iy < info->height; ++iy) {
				if (map->Get16Gray(0, iy, info->width, line) != 1) {
					for (png_uint_32 i = 0; i < info->height; i++)
						if (row_pointers[i]) free(row_pointers[i]);
					if (row_pointers) {
						free(row_pointers);
						row_pointers = NULL;
					}

					fclose(ostream);
					_tremove(filename);
					free(line);
                    png_destroy_write_struct (&png, &info);
					return BMMRES_IOERROR;
				}
				unsigned short *l=line;
				unsigned char *obyte = (unsigned char *)row_pointers[iy];
				for (png_uint_32 ix = 0; ix < info->width; ix++) {
					*obyte++ = (unsigned char)(*l >> 8); l++;
				}
			}
			free(line);
			}
			break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			{
			BMM_Color_64 *line64 = (BMM_Color_64 *) calloc(info->width,sizeof(BMM_Color_64));
			unsigned short *line = (unsigned short *) calloc(info->width, sizeof(unsigned short));
			for (png_uint_32 iy = 0; iy < info->height; ++iy) {
				if (GetOutputPixels(0, iy, info->width, line64) != 1 ||
					map->Get16Gray(0, iy, info->width, line) != 1) {
					for (png_uint_32 i = 0; i < info->height; i++)
						if (row_pointers[i]) free(row_pointers[i]);
					if (row_pointers) {
						free(row_pointers);
						row_pointers = NULL;
					}

					fclose(ostream);
					_tremove(filename);
					free(line);
					free(line64);
                    png_destroy_write_struct (&png, &info);
					return BMMRES_IOERROR;
				}
				unsigned short *l=line;
				BMM_Color_64 *l64 = line64;
				unsigned char *obyte = (unsigned char *)row_pointers[iy];
				for (png_uint_32 ix = 0; ix < info->width; ix++, l64++) {
					*obyte++ = (unsigned char)(*l >> 8); l++;
					*obyte++ = (unsigned char)(l64->a >> 8);
				}
			}
			free(line);
			free(line64);
			}
			break;
		}
		break;
#ifdef OUTPUT_1_2_4
	case 4: { // Paletted only
		}
		break;
	case 2: { // Paletted only
		}
		break;
	case 1: { // Paletted only
		}
#endif
		break;
	}

	png_write_info(png, info);

	png_set_swap(png);

	png_write_image(png, row_pointers);

	png_write_end(png, info);
	fclose(ostream);

 	for (i = 0; i < info->height; i++)
		free(row_pointers[i]);

	free(row_pointers);

    png_destroy_write_struct (&png, &info);

    return BMMRES_SUCCESS;
}


BMMRES
BitmapIO_PNG::Write(int frame) 
{
    //-- If we haven't gone through an OpenOutput(), leave
    if (openMode != BMM_OPEN_W)
		return (ProcessImageIOError(&bi,BMMRES_INTERNALERROR));

    //-- Resolve Filename --------------------------------
    TCHAR filename[MAX_PATH];

    if (frame == BMM_SINGLEFRAME) {
        _tcscpy(filename,bi.Name());
    } else {
        if (!BMMCreateNumberedFilename(bi.Name(),frame,filename)) {
			return (ProcessImageIOError(&bi,BMMRES_NUMBEREDFILENAMEERROR));
        }
    }

    return Save(filename, map);
}

int
BitmapIO_PNG::Close(int option)
{
	if(openMode != BMM_OPEN_W)
		return 0;
	return 1;
}


INT_PTR CALLBACK 
ConfigCtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) 
{
	BitmapIO_PNG *th = (BitmapIO_PNG *)GetWindowLongPtr( hWnd, GWLP_USERDATA );

	if ( !th && message != WM_INITDIALOG ) return FALSE;

    switch (message) {
    case WM_INITDIALOG:
		th = (BitmapIO_PNG *)lParam;
		SetWindowLongPtr( hWnd, GWLP_USERDATA, (LONG_PTR)th );

		if (!th->cfg.saved)
			th->ReadCfg();

		th->dlgcfg.color_type = th->cfg.color_type;
		th->dlgcfg.bitdepth = th->cfg.bitdepth;
		th->dlgcfg.interlaced = th->cfg.interlaced;

		// first interlaced
		CheckDlgButton( hWnd, IDC_PNG_INTERLACE, th->dlgcfg.interlaced);

		// now alpha
		switch (th->dlgcfg.color_type) {
		case PngPalette:
			EnableWindow(GetDlgItem(hWnd,IDC_PNG_ALPHA), FALSE);
			break;
		case PngRGB:
		case PngGray:
			EnableWindow(GetDlgItem(hWnd,IDC_PNG_ALPHA), TRUE);
			CheckDlgButton( hWnd, IDC_PNG_ALPHA, FALSE);
			break;
		case PngRGBA:
		case PngGrayA:
			EnableWindow(GetDlgItem(hWnd,IDC_PNG_ALPHA), TRUE);
			CheckDlgButton( hWnd, IDC_PNG_ALPHA, TRUE);
			break;
		}

		switch (th->dlgcfg.color_type) {
		case PngPalette:
			CheckDlgButton( hWnd, IDC_PNG_PALETTE, TRUE );
			break;
		case PngRGB:
		case PngRGBA:
			if (th->dlgcfg.bitdepth == 8)
				CheckDlgButton( hWnd, IDC_PNG_RGB24, TRUE );
			else
				CheckDlgButton( hWnd, IDC_PNG_RGB48, TRUE );
			break;
		case PngGray:
		case PngGrayA:
			if (th->dlgcfg.bitdepth == 8)
				CheckDlgButton( hWnd, IDC_PNG_GRAY8, TRUE );
			else
				CheckDlgButton( hWnd, IDC_PNG_GRAY16, TRUE );
			break;
		}

    case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:              
			th->cfg.color_type = th->dlgcfg.color_type;
			th->cfg.bitdepth = th->dlgcfg.bitdepth;
			th->cfg.interlaced = th->dlgcfg.interlaced;
			th->WriteCfg();
			EndDialog(hWnd,1);
			break;
		case IDCANCEL:
			EndDialog(hWnd,0);
			break;

		case IDC_PNG_PALETTE:
			th->dlgcfg.color_type = PngPalette;
			th->dlgcfg.bitdepth = 8;
			EnableWindow(GetDlgItem(hWnd,IDC_PNG_ALPHA), FALSE);
			break;

		case IDC_PNG_RGB24:
			if (IsDlgButtonChecked(hWnd, IDC_PNG_ALPHA))
				th->dlgcfg.color_type = PngRGBA;
			else
				th->dlgcfg.color_type = PngRGB;
			th->dlgcfg.bitdepth = 8;
			EnableWindow(GetDlgItem(hWnd,IDC_PNG_ALPHA), TRUE);
			break;

		case IDC_PNG_RGB48:
			if (IsDlgButtonChecked(hWnd, IDC_PNG_ALPHA))
				th->dlgcfg.color_type = PngRGBA;
			else
				th->dlgcfg.color_type = PngRGB;
			th->dlgcfg.bitdepth = 16;
			EnableWindow(GetDlgItem(hWnd,IDC_PNG_ALPHA), TRUE);
			break;

		case IDC_PNG_GRAY8:
			if (IsDlgButtonChecked(hWnd, IDC_PNG_ALPHA))
				th->dlgcfg.color_type = PngGrayA;
			else
				th->dlgcfg.color_type = PngGray;
			th->dlgcfg.bitdepth = 8;
			EnableWindow(GetDlgItem(hWnd,IDC_PNG_ALPHA), TRUE);
			break;

		case IDC_PNG_GRAY16:
			if (IsDlgButtonChecked(hWnd, IDC_PNG_ALPHA))
				th->dlgcfg.color_type = PngGrayA;
			else
				th->dlgcfg.color_type = PngGray;
			th->dlgcfg.bitdepth = 16;
			EnableWindow(GetDlgItem(hWnd,IDC_PNG_ALPHA), TRUE);
			break;

		case IDC_PNG_INTERLACE:
			th->dlgcfg.interlaced = IsDlgButtonChecked(hWnd, IDC_PNG_INTERLACE);
			break;

		case IDC_PNG_ALPHA:
			switch(th->dlgcfg.color_type) {
			case PngRGB:
			case PngRGBA:
				if (IsDlgButtonChecked(hWnd, IDC_PNG_ALPHA))
					th->dlgcfg.color_type = PngRGBA;
				else
					th->dlgcfg.color_type = PngRGB;
				break;
			case PngGray:
			case PngGrayA:
				if (IsDlgButtonChecked(hWnd, IDC_PNG_ALPHA))
					th->dlgcfg.color_type = PngGrayA;
				else
					th->dlgcfg.color_type = PngGray;
				break;
			}
			break;
		}
		return 1;
    }
    return 0;
}

BOOL
BitmapIO_PNG::ShowControl(HWND hWnd, DWORD flag ) 
{
    return DialogBoxParam(hResource, MAKEINTRESOURCE (IDD_PNG_CONFIG),
		    hWnd, (DLGPROC) ConfigCtrlDlgProc, (LPARAM) this);
}


INT_PTR CALLBACK 
AboutCtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) 
{
    switch (message) {
	
    case WM_INITDIALOG: {
		// more stuff here !
		return 1;
		}

    case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:              
			EndDialog(hWnd,1);
			break;
		case IDCANCEL:
			EndDialog(hWnd,0);
			break;
		}
		return 1;
    }
    return 0;
}

void
BitmapIO_PNG::ShowAbout(HWND hWnd) 
{
    DialogBoxParam (hResource, MAKEINTRESOURCE (IDD_PNG_ABOUT),
		    hWnd, (DLGPROC) AboutCtrlDlgProc, (LPARAM) 0);
}


//-----------------------------------------------------------------------------
// #> BitmapIO_PNG::GetCfgFilename()
//

void BitmapIO_PNG::GetCfgFilename( TCHAR *filename ) {
	_tcscpy(filename,TheManager->GetDir(APP_PLUGCFG_DIR));
	int len = _tcslen(filename);
	if (len) {
		if (_tcscmp(&filename[len-1],_T("\\")))
			_tcscat(filename,_T("\\"));
		}   
	_tcscat(filename,_T("png.cfg"));   
	}


//-----------------------------------------------------------------------------
//-- File Class

class File {
	public:
		FILE  *stream;
		File  ( const TCHAR *name, const TCHAR *mode) { stream = _tfopen(name,mode); }
		~File ( ) { Close(); }
		void Close() { if(stream) fclose(stream); stream = NULL; }
};

//-----------------------------------------------------------------------------
// #> BitmapIO_PNG::ReadCfg()
//

BOOL BitmapIO_PNG::ReadCfg() {
	
	TCHAR filename[MAX_PATH];
	GetCfgFilename(filename);

	//-- Open Configuration File
	
	File file(filename, _T("rb"));
	
	if (!file.stream)
		return (FALSE);
	
	fseek(file.stream,0,SEEK_END);
	DWORD size = (DWORD)ftell(file.stream);
	
	if (size) {

		fseek(file.stream,0,SEEK_SET);
		
		//-- Allocate Temp Buffer
		
		BYTE *buf = (BYTE *)LocalAlloc(LPTR,size);
		
		if (!buf)
			return (FALSE);
		
		//-- Read Data Block and Set it
		
		BOOL res = FALSE;
		
		if (fread(buf,1,size,file.stream) == size)
			res = LoadConfigure(buf);

		if(!res)
			InitUserData(&cfg);
					
		LocalFree(buf);
	
		return (res);
	
	}
	
	return (FALSE);
}
	
//-----------------------------------------------------------------------------
// #> BitmapIO_PNG::WriteCfg()
//

void BitmapIO_PNG::WriteCfg() {
 
	TCHAR filename[MAX_PATH];
	GetCfgFilename(filename);
	
	//-- Find out buffer size
	
	DWORD size = EvaluateConfigure();
	
	if (!size)
		return;
	
	//-- Allocate Temp Buffer
	
	BYTE *buf = (BYTE *)LocalAlloc(LPTR,size);
	
	if (!buf)
		return;
	
	//-- Get Data Block and Write it
	
	if (SaveConfigure(buf)) {   
		File file(filename, _T("wb"));
		if (file.stream) {
			fwrite(buf,1,size,file.stream);
		}
	}
	
	LocalFree(buf);
	
}
