//-----------------------------------------------------------------------------
// --------------------
// File ....: targa.cpp
// --------------------
// Author...: Tom Hudson, conversion from Kyle Peacock's 3DS writer
// Date ....: May 1995
// Descr....: Targa File I/O Module
//
// History .: May, 31 1995 - Started
//            Oct, 23 1995 - Continued work (GG)
//            Dec. 07 1995 - Tom picked it up again
//            
//-----------------------------------------------------------------------------


//-- Includes -----------------------------------------------------------------

#include <Max.h>
#include <bmmlib.h>
#include <iparamb2.h>
#include "targa.h"
#include "targarc.h"


//-- Handy macros -------------------------------------------------------------

#define TGAREAD(ptr,sz) if (fread(ptr, sz, 1, stream)!=1) goto corrupt;
#define TGAWRITE(ptr,sz) ((fwrite((void *)ptr, sz, 1, stream)!=1) ? 0:1)

//#include <stdarg.h>

//-----------------------------------------------------------------------------
//-- File Class

class File {
	public:
		FILE *stream;
		File(const TCHAR *name, const TCHAR *mode) { stream = _tfopen(name,mode); }
		~File() { Close(); }
		void Close() { if(stream) fclose(stream); stream = NULL; }
	};

//-- Globals ------------------------------------------------------------------

HINSTANCE hInst = NULL;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-- DLL Declaration

BOOL WINAPI DllMain(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID lpvReserved) {
	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:
			if (hInst)
				return(FALSE);
			hInst = hDLLInst;
			break;
		case DLL_PROCESS_DETACH:
			hInst  = NULL;
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
	}
	return TRUE;
}

TCHAR *GetString(int id)
	{
	static TCHAR buf[256];

	if (hInst)
		return LoadString(hInst, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// TGA Class Description

class TGAClassDesc:public ClassDesc2 {
	public:
		int           IsPublic     ( ) { return 1; }
		void         *Create       ( BOOL loading=FALSE) { return new BitmapIO_TGA; }
		const TCHAR  *ClassName    ( ) { return GetString(IDS_TARGA); }
		SClass_ID     SuperClassID ( ) { return BMM_IO_CLASS_ID; }
		Class_ID      ClassID      ( ) { return Class_ID(TARGACLASSID,0); }
		const TCHAR  *Category     ( ) { return GetString(IDS_BITMAP_IO);  }
		const TCHAR  *InternalName ( ) { return _T("tgaio"); }
		HINSTANCE     HInstance    ( ) { return hInst; }
};

static TGAClassDesc TGADesc;

//-----------------------------------------------------------------------------
// Initialize user data to reasonable values

static void InitUserData(TGAUSERDATA *ptr) {
	// Init user data
	ptr->version = TGAVERSION;
	ptr->saved = FALSE;
	ptr->writeType = TGA_32_BITS;
	ptr->compressed = TRUE;
	ptr->alphaSplit = FALSE;
	ptr->preMultAlpha = TRUE;
	ptr->author[0] = 0;
	ptr->jobname[0] = 0;
	ptr->comments1[0] = 0;
	ptr->comments2[0] = 0;
	ptr->comments3[0] = 0;
	ptr->comments4[0] = 0;
	}

//-----------------------------------------------------------------------------
// Interface

DLLEXPORT const TCHAR * LibDescription ( )  { 
	return GetString(IDS_TARGA_DESC); 
}

DLLEXPORT int LibNumberClasses ( ) { 
	return 1; 
}

DLLEXPORT ClassDesc *LibClassDesc(int i) {
	switch(i) {
		case  0: return &TGADesc; break;
		default: return 0;        break;
	}
}


static DWORD randomseed = 9431459;
static int  arand() {
	randomseed = randomseed * 1103515245 + 12345;
	return (int)((randomseed>>16)&0x7FFF);
	}

__declspec( dllexport ) ULONG LibVersion ( )  { 
	return ( VERSION_3DSMAX ); 
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

// Function published July 2000 - CCJ
// FP Interface
class BitmapIO_Tga_Imp : public IBitmapIO_Tga {	
public:
	int		GetColorDepth();
	void	SetColorDepth(int bpp);
	BOOL	GetCompressed();
	void	SetCompressed(BOOL compressed);
	BOOL	GetAlphaSplit();
	void	SetAlphaSplit(BOOL alphaSplit);
	BOOL	GetPreMultAlpha();
	void	SetPreMultAlpha(BOOL preMult);

	TSTR	GetAuthor();
	void	SetAuthor(TSTR author);
	TSTR	GetJobName();
	void	SetJobName(TSTR jobname);
	TSTR	GetComment1();
	void	SetComment1(TSTR comment);
	TSTR	GetComment2();
	void	SetComment2(TSTR comment);
	TSTR	GetComment3();
	void	SetComment3(TSTR comment);
	TSTR	GetComment4();
	void	SetComment4(TSTR comment);

	// function IDs 
	enum { tgaio_getColorDepth, tgaio_setColorDepth, tgaio_getCompressed, tgaio_setCompressed, 
		tgaio_getAlphaSplit, tgaio_setAlphaSplit, tgaio_getPreMultAlpha, tgaio_setPreMultAlpha,
		tgaio_getAuthor, tgaio_setAuthor, tgaio_getJobName, tgaio_setJobName,
		tgaio_getComment1, tgaio_setComment1, tgaio_getComment2, tgaio_setComment2,
		tgaio_getComment3, tgaio_setComment3, tgaio_getComment4, tgaio_setComment4,
	};

	DECLARE_DESCRIPTOR(BitmapIO_Tga_Imp) 

	// dispatch map
	BEGIN_FUNCTION_MAP
		FN_0(tgaio_getColorDepth, TYPE_INT, GetColorDepth);
		VFN_1(tgaio_setColorDepth, SetColorDepth, TYPE_INT);

		FN_0(tgaio_getCompressed, TYPE_INT, GetCompressed);
		VFN_1(tgaio_setCompressed, SetCompressed, TYPE_INT);

		FN_0(tgaio_getAlphaSplit, TYPE_BOOL, GetAlphaSplit);
		VFN_1(tgaio_setAlphaSplit, SetAlphaSplit, TYPE_BOOL);

		FN_0(tgaio_getPreMultAlpha, TYPE_BOOL, GetPreMultAlpha);
		VFN_1(tgaio_setPreMultAlpha, SetPreMultAlpha, TYPE_BOOL);

		FN_0(tgaio_getAuthor, TYPE_TSTR_BV, GetAuthor);
		VFN_1(tgaio_setAuthor, SetAuthor, TYPE_TSTR_BV);

		FN_0(tgaio_getJobName, TYPE_TSTR_BV, GetJobName);
		VFN_1(tgaio_setJobName, SetJobName, TYPE_TSTR_BV);

		FN_0(tgaio_getComment1, TYPE_TSTR_BV, GetComment1);
		VFN_1(tgaio_setComment1, SetComment1, TYPE_TSTR_BV);
		FN_0(tgaio_getComment2, TYPE_TSTR_BV, GetComment2);
		VFN_1(tgaio_setComment2, SetComment2, TYPE_TSTR_BV);
		FN_0(tgaio_getComment3, TYPE_TSTR_BV, GetComment3);
		VFN_1(tgaio_setComment3, SetComment3, TYPE_TSTR_BV);
		FN_0(tgaio_getComment4, TYPE_TSTR_BV, GetComment4);
		VFN_1(tgaio_setComment4, SetComment4, TYPE_TSTR_BV);
	END_FUNCTION_MAP 
	};

static BitmapIO_Tga_Imp tgaIOInterface(
		BMPIO_INTERFACE, _T("itgaio"), IDS_TGAIO_INTERFACE, &TGADesc, 0,
			BitmapIO_Tga_Imp::tgaio_getColorDepth, _T("getColorDepth"), IDS_TGAIO_GETCOLORDEPTH, TYPE_INT, 0, 0, 
			BitmapIO_Tga_Imp::tgaio_setColorDepth, _T("setColorDepth"), IDS_TGAIO_SETCOLORDEPTH, TYPE_VOID, 0, 1, 
			_T("colorDepth"), 0, TYPE_INT, 

			BitmapIO_Tga_Imp::tgaio_getCompressed, _T("getCompressed"), IDS_TGAIO_GETCOMPRESSED, TYPE_BOOL, 0, 0, 
			BitmapIO_Tga_Imp::tgaio_setCompressed, _T("setCompressed"), IDS_TGAIO_SETCOMPRESSED, TYPE_VOID, 0, 1, 
			_T("compression"), 0, TYPE_BOOL, 

			BitmapIO_Tga_Imp::tgaio_getAlphaSplit, _T("getAlphaSplit"), IDS_TGAIO_GETALPHASPLIT, TYPE_BOOL, 0, 0, 
			BitmapIO_Tga_Imp::tgaio_setAlphaSplit, _T("setAlphaSplit"), IDS_TGAIO_SETALPHASPLIT, TYPE_VOID, 0, 1, 
			_T("alphaSplit"), 0, TYPE_BOOL, 

			BitmapIO_Tga_Imp::tgaio_getPreMultAlpha, _T("getPreMultAlpha"), IDS_TGAIO_GETPREMULTALPHA, TYPE_BOOL, 0, 0, 
			BitmapIO_Tga_Imp::tgaio_setPreMultAlpha, _T("setPreMultAlpha"), IDS_TGAIO_SETPREMULTALPHA, TYPE_VOID, 0, 1, 
			_T("preMultAlpha"), 0, TYPE_BOOL, 

			BitmapIO_Tga_Imp::tgaio_getAuthor, _T("getAuthor"), IDS_TGAIO_GETAUTHOR, TYPE_TSTR_BV, 0, 0, 
			BitmapIO_Tga_Imp::tgaio_setAuthor, _T("setAuthor"), IDS_TGAIO_SETAUTHOR, TYPE_VOID, 0, 1, 
			_T("author"), 0, TYPE_TSTR_BV, 

			BitmapIO_Tga_Imp::tgaio_getJobName, _T("getJobName"), IDS_TGAIO_GETJOBNAME, TYPE_TSTR_BV, 0, 0, 
			BitmapIO_Tga_Imp::tgaio_setJobName, _T("setJobName"), IDS_TGAIO_SETJOBNAME, TYPE_VOID, 0, 1, 
			_T("jobname"), 0, TYPE_TSTR_BV, 

			BitmapIO_Tga_Imp::tgaio_getComment1, _T("getComment1"), IDS_TGAIO_GETCOMMENT1, TYPE_TSTR_BV, 0, 0, 
			BitmapIO_Tga_Imp::tgaio_setComment1, _T("setComment1"), IDS_TGAIO_SETCOMMENT1, TYPE_VOID, 0, 1, 
			_T("comment"), 0, TYPE_TSTR_BV, 

			BitmapIO_Tga_Imp::tgaio_getComment2, _T("getComment2"), IDS_TGAIO_GETCOMMENT2, TYPE_TSTR_BV, 0, 0, 
			BitmapIO_Tga_Imp::tgaio_setComment2, _T("setComment2"), IDS_TGAIO_SETCOMMENT2, TYPE_VOID, 0, 1, 
			_T("comment"), 0, TYPE_TSTR_BV, 

			BitmapIO_Tga_Imp::tgaio_getComment3, _T("getComment3"), IDS_TGAIO_GETCOMMENT3, TYPE_TSTR_BV, 0, 0, 
			BitmapIO_Tga_Imp::tgaio_setComment3, _T("setComment3"), IDS_TGAIO_SETCOMMENT3, TYPE_VOID, 0, 1, 
			_T("comment"), 0, TYPE_TSTR_BV, 

			BitmapIO_Tga_Imp::tgaio_getComment4, _T("getComment4"), IDS_TGAIO_GETCOMMENT4, TYPE_TSTR_BV, 0, 0, 
			BitmapIO_Tga_Imp::tgaio_setComment4, _T("setComment4"), IDS_TGAIO_SETCOMMENT4, TYPE_VOID, 0, 1, 
			_T("comment"), 0, TYPE_TSTR_BV, 

		end);

int BitmapIO_Tga_Imp::GetColorDepth()
	{
	int depth = TGA_32_BITS;

	BitmapIO_TGA* p = new BitmapIO_TGA;
	if (p) {
		p->ReadCfg();
		depth = p->UserData.writeType;
		delete p;
		}

	int retVal = 32;
	switch (depth) {
		case TGA_16_BITS: retVal = 16; break;
		case TGA_24_BITS: retVal = 24; break;
		case TGA_32_BITS: retVal = 32; break;
		}
	return retVal;
	}

void BitmapIO_Tga_Imp::SetColorDepth(int depth)
	{
	BitmapIO_TGA* p = new BitmapIO_TGA;
	if (p) {
		p->ReadCfg();

		switch (depth) {
			case 16 : p->UserData.writeType = TGA_16_BITS; break;
			case 24 : p->UserData.writeType = TGA_24_BITS; break;
			case 32 : p->UserData.writeType = TGA_32_BITS; break;
			}

		p->WriteCfg();

		delete p;
		}
	}

BOOL BitmapIO_Tga_Imp::GetCompressed()
	{
	BOOL compressed = TRUE;

	BitmapIO_TGA* p = new BitmapIO_TGA;
	if (p) {
		p->ReadCfg();
		compressed = p->UserData.compressed;
		delete p;
		}
	return compressed;
	}

void BitmapIO_Tga_Imp::SetCompressed(BOOL compressed)
	{
	BitmapIO_TGA* p = new BitmapIO_TGA;
	if (p) {
		p->ReadCfg();
		p->UserData.compressed = compressed;
		p->WriteCfg();
		delete p;
		}
	}

BOOL BitmapIO_Tga_Imp::GetAlphaSplit()
	{
	BOOL alphaSplit = FALSE;

	BitmapIO_TGA* p = new BitmapIO_TGA;
	if (p) {
		p->ReadCfg();
		alphaSplit = p->UserData.alphaSplit;
		delete p;
		}
	return alphaSplit;
	}

void BitmapIO_Tga_Imp::SetAlphaSplit(BOOL alphaSplit)
	{
	BitmapIO_TGA* p = new BitmapIO_TGA;
	if (p) {
		p->ReadCfg();
		p->UserData.alphaSplit = alphaSplit;
		p->WriteCfg();
		delete p;
		}
	}

BOOL BitmapIO_Tga_Imp::GetPreMultAlpha()
	{
	BOOL preMultAlpha = TRUE;

	BitmapIO_TGA* p = new BitmapIO_TGA;
	if (p) {
		p->ReadCfg();
		preMultAlpha = p->UserData.preMultAlpha;
		delete p;
		}
	return preMultAlpha;
	}

void BitmapIO_Tga_Imp::SetPreMultAlpha(BOOL preMultAlpha)
	{
	BitmapIO_TGA* p = new BitmapIO_TGA;
	if (p) {
		p->ReadCfg();
		p->UserData.preMultAlpha = preMultAlpha;
		p->WriteCfg();
		delete p;
		}
	}


TSTR BitmapIO_Tga_Imp::GetAuthor()
	{
	TSTR author;

	BitmapIO_TGA* p = new BitmapIO_TGA;
	if (p) {
		p->ReadCfg();
		author = p->UserData.author;
		delete p;
		}
	return author;
	}

void BitmapIO_Tga_Imp::SetAuthor(TSTR author)
	{
	BitmapIO_TGA* p = new BitmapIO_TGA;
	if (p) {
		author.remove(40);
		p->ReadCfg();
		_tcscpy(p->UserData.author,author.data());
		p->WriteCfg();
		delete p;
		}
	}

TSTR BitmapIO_Tga_Imp::GetJobName()
	{
	TSTR jobname;

	BitmapIO_TGA* p = new BitmapIO_TGA;
	if (p) {
		p->ReadCfg();
		jobname = p->UserData.jobname;
		delete p;
		}
	return jobname;
	}

void BitmapIO_Tga_Imp::SetJobName(TSTR jobname)
	{
	BitmapIO_TGA* p = new BitmapIO_TGA;
	if (p) {
		jobname.remove(40);
		p->ReadCfg();
		_tcscpy(p->UserData.jobname,jobname.data());
		p->WriteCfg();
		delete p;
		}
	}

TSTR BitmapIO_Tga_Imp::GetComment1()
	{
	TSTR comment;

	BitmapIO_TGA* p = new BitmapIO_TGA;
	if (p) {
		p->ReadCfg();
		comment = p->UserData.comments1;
		delete p;
		}
	return comment;
	}

void BitmapIO_Tga_Imp::SetComment1(TSTR comment)
	{
	BitmapIO_TGA* p = new BitmapIO_TGA;
	if (p) {
		comment.remove(80);
		p->ReadCfg();
		_tcscpy(p->UserData.comments1,comment.data());
		p->WriteCfg();
		delete p;
		}
	}

TSTR BitmapIO_Tga_Imp::GetComment2()
	{
	TSTR comment;

	BitmapIO_TGA* p = new BitmapIO_TGA;
	if (p) {
		p->ReadCfg();
		comment = p->UserData.comments2;
		delete p;
		}
	return comment;
	}

void BitmapIO_Tga_Imp::SetComment2(TSTR comment)
	{
	BitmapIO_TGA* p = new BitmapIO_TGA;
	if (p) {
		comment.remove(80);
		p->ReadCfg();
		_tcscpy(p->UserData.comments2,comment.data());
		p->WriteCfg();
		delete p;
		}
	}

TSTR BitmapIO_Tga_Imp::GetComment3()
	{
	TSTR comment;

	BitmapIO_TGA* p = new BitmapIO_TGA;
	if (p) {
		p->ReadCfg();
		comment = p->UserData.comments3;
		delete p;
		}
	return comment;
	}

void BitmapIO_Tga_Imp::SetComment3(TSTR comment)
	{
	BitmapIO_TGA* p = new BitmapIO_TGA;
	if (p) {
		comment.remove(80);
		p->ReadCfg();
		_tcscpy(p->UserData.comments3,comment.data());
		p->WriteCfg();
		delete p;
		}
	}

TSTR BitmapIO_Tga_Imp::GetComment4()
	{
	TSTR comment;

	BitmapIO_TGA* p = new BitmapIO_TGA;
	if (p) {
		p->ReadCfg();
		comment = p->UserData.comments4;
		delete p;
		}
	return comment;
	}

void BitmapIO_Tga_Imp::SetComment4(TSTR comment)
	{
	BitmapIO_TGA* p = new BitmapIO_TGA;
	if (p) {
		comment.remove(80);
		p->ReadCfg();
		_tcscpy(p->UserData.comments4,comment.data());
		p->WriteCfg();
		delete p;
		}
	}


//-----------------------------------------------------------------------------
// *> we_did_abort(BitmapInfo* pbi)
//
//    
static bool we_did_abort(BitmapInfo* pbi)
{
    if(pbi)
    {
        HWND hwnd = pbi->GetUpdateWindow();
        if(IsWindow(hwnd))
        {
            BOOL abort = FALSE;
            SendMessage(hwnd, BMM_CHECKABORT, 0, reinterpret_cast<LPARAM>(&abort));
            return (abort == TRUE);
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
// *> BitmapIO_TGA::ReadHeader()
//
//    Read a .TGA file header

int  BitmapIO_TGA::ReadHeader() {
	if(fread(&hdr,sizeof(TGAHeader),1,inStream)!=1)
		return 0;
	return 1;
}

//-----------------------------------------------------------------------------
// *> BitmapIO_TGA::ReadFooter()
//
//    Read a .TGA file footer and the extra data in a Targa 2.0 file, if present

#define TGA_ALPHA_NONE 0
#define TGA_ALPHA_IGNORE 1
#define TGA_ALPHA_RETAIN 2
#define TGA_ALPHA_HASALPHA 3
#define TGA_ALPHA_PREMULT 4

int  BitmapIO_TGA::ReadFooter() {
	// Initialize the extra area fields we're interested in
	memset(extra.authorname, 0, sizeof(extra.authorname));
	memset(extra.comments, 0, sizeof(extra.comments));
	extra.td_month = 1;
	extra.td_day = 1;
	extra.td_year = 1980;
	extra.td_hour = 0;
	extra.td_minute = 0;
	extra.td_second = 0;
	memset(extra.jobname, 0, sizeof(extra.jobname));
	extra.jt_hours = 0;
	extra.jt_minutes = 0;
	extra.jt_seconds = 0;
	memset(extra.softwareID, 0, sizeof(extra.softwareID));
	extra.sw_version = 0;
	extra.version_letter = ' ';
	extra.key_a = 0;
	extra.key_r = 0;
	extra.key_g = 0;
	extra.key_b = 0;
	extra.aspect_w = 1;
	extra.aspect_h = 1;
	extra.gamma_numerator = 1;
	extra.gamma_denominator = 1;
	extra.color_corr_table = 0;
	extra.postage_stamp = 0;
	extra.scan_line = 0;
	extra.alpha_attributes = TGA_ALPHA_PREMULT;
	
	// Now grab it!
	fseek(inStream, -26L, SEEK_END);
	if(fread(&foot,sizeof(TGAFooter),1,inStream)!=1)
		return 0;
	// Check the signature
	if(strncmp(foot.signature,"TRUEVISION-XFILE",16)!=0)
		return 1;
	// Got the signature, let's grab the footer (if any)
	if(!foot.ext_area)
		return 1;		// No extension area, let's blow!
	fseek(inStream, foot.ext_area, SEEK_SET);
	if(!fread(&extra, sizeof(TGAExtra), 1, inStream))
		return 0;
	if(extra.gamma_denominator) {
		gotGamma = TRUE;
		gamma = (float)extra.gamma_numerator / (float)extra.gamma_denominator;
		if(gamma < MINGAMMA)
			gamma = (float)MINGAMMA;
		else
		if(gamma > MAXGAMMA)
			gamma = (float)MAXGAMMA;
		}
	if(extra.aspect_h)
		aspect = (float)extra.aspect_w / (float)extra.aspect_h;
	return 1;
}

//-----------------------------------------------------------------------------
// *> BitmapIO_TGA::Load8BitPalTGA()
//
//    Load an 8-bit paletted TGA file, returning the storage location

BitmapStorage *BitmapIO_TGA::Load8BitPalTGA(BitmapInfo *fbi, FILE *stream, BitmapManager *manager) {


	//-- Create a storage for this bitmap ------------------------------------

	BitmapStorage *s = BMMCreateStorage(manager,BMM_PALETTED);

	if(!s)
		return NULL;

	if (s->Allocate(fbi,manager,BMM_OPEN_R)==0) {
		bail_out:
		if(s) {
			delete s;
			s = NULL;
		}
		return NULL;
	}

	//-- Bypass the image identification field, if present -------------------

	if (hdr.idlen) {
		if (fseek(stream,(long)hdr.idlen,SEEK_CUR)!=0) {
			corrupt:
			return NULL;
		}
	}

	//-- Must read the color map, if present ------------------------------------

	PixelBuf48 palette(hdr.cmlen+1);	// Allocate a buffer for the palette entries
	// Zero out the palette
	BMM_Color_48 black = {0,0,0};
	palette.Fill(0,hdr.cmlen+1,black);

	if(!hdr.cmlen)
		goto bail_out;

	// Load up the 64-bit palette
	int ix;
	switch(hdr.cmes) {
		case 15:
		case 16: {
			PixelBuf16 buf(hdr.cmlen);
			TGAREAD(buf.Ptr(), sizeof(USHORT)*hdr.cmlen);
			BMM_Color_48 *pal = palette.Ptr();
			USHORT *l16 = buf.Ptr();
			for(ix=0; ix<hdr.cmlen; ++ix,++pal,++l16) {
				pal->r = (WORD)(*l16 & 0x7c00) << 1;
				pal->g = (WORD)(*l16 & 0x03e0) << 6;
				pal->b = (WORD)(*l16 & 0x001f) << 11;
				}
			}
			break;
		case 24: {
			PixelBuf24 buf(hdr.cmlen);
			TGAREAD(buf.Ptr(), sizeof(BMM_Color_24)*hdr.cmlen);
			BMM_Color_48 *pal = palette.Ptr();
			BMM_Color_24 *l24 = buf.Ptr();
			for(ix=0; ix<hdr.cmlen; ++ix,++pal,++l24) {
				pal->r = (WORD)l24->b << 8;
				pal->g = (WORD)l24->g << 8;
				pal->b = (WORD)l24->r << 8;
				}
			}
			break;
		case 32: {
			PixelBuf32 buf(hdr.cmlen);
			TGAREAD(buf.Ptr(), sizeof(BMM_Color_32)*hdr.cmlen);
			BMM_Color_48 *pal = palette.Ptr();
			BMM_Color_32 *l32 = buf.Ptr();
			for(ix=0; ix<hdr.cmlen; ++ix,++pal,++l32) {
				pal->r = (WORD)l32->b << 8;
				pal->g = (WORD)l32->g << 8;
				pal->b = (WORD)l32->r << 8;
				}
			}
			break;
		}

	// Stuff the palette into the storage
	s->SetPalette(hdr.cmorg, hdr.cmlen, palette.Ptr());

	PixelBuf8  line8(hdr.width);

	//-- Read Image File -----------------------------------------------------

	switch (hdr.imgtype) {

		//-- Uncompressed RGB -----------------------------

		case 1:  {
			for(int iy = hdr.height - 1; iy >= 0; --iy) {
				UBYTE *l8=line8.Ptr();
				TGAREAD(l8,sizeof(UBYTE)*hdr.width);
                if(we_did_abort(fbi))
                    goto bail_out;
				if(s->PutIndexPixels(0,iy,hdr.width,l8)!=1)
					goto bail_out;
			}
			}
			break;
		
		//-- Compressed RGB -------------------------------

		case 9: {
			
			int x = 0;
			int y = hdr.height - 1;
			BYTE rle;
			UBYTE pixel;
			UBYTE *l8=line8.Ptr();
			
			while(1) {
				
				TGAREAD(&rle,1);
				
				//-- Compressed Block
				
				if(rle>127) {
					rle-=127;
					TGAREAD(&pixel,1);
					for(int ix=0; ix<rle; ++ix) {
						l8[x++] = pixel;
						if(x>=hdr.width) {
                        if(we_did_abort(fbi))
                            goto bail_out;
						if(s->PutIndexPixels(0,y,hdr.width,line8.Ptr())!=1)
							goto bail_out;
						x=0;
						y--;
						if(y<0)
							goto done;
						}
					}

				//-- Uncompressed Block
				
				} else {

					rle++;
					for(int ix=0; ix<rle; ++ix) {
						TGAREAD(&pixel,1);
						l8[x++] = pixel;
						if(x>=hdr.width) {
                        if(we_did_abort(fbi))
                            goto bail_out;
						if(s->PutIndexPixels(0,y,hdr.width,line8.Ptr())!=1)
							goto bail_out;
						x=0;
						y--;
						if(y<0)
							goto done;
						}
					}
				}
			}
			}
			break;
		}

	done:
	return s;

}

//-----------------------------------------------------------------------------
// *> BitmapIO_TGA::Load8BitGrayTGA()
//
//    Load an 8-bit grayscale TGA file, returning the storage location

BitmapStorage *BitmapIO_TGA::Load8BitGrayTGA(BitmapInfo *fbi, FILE *stream, BitmapManager *manager) {

	//-- Create a storage for this bitmap ------------------------------------

	BitmapStorage *s = BMMCreateStorage(manager,BMM_GRAY_8);

	if(!s)
		return NULL;

	if (s->Allocate(fbi,manager,BMM_OPEN_R)==0) {
		bail_out:
		if(s) {
			delete s;
			s = NULL;
		}
		return NULL;
	}

	//-- Bypass the image identification field, if present -------------------

	if (hdr.idlen) {
		if (fseek(stream,(long)hdr.idlen,SEEK_CUR)!=0) {
			corrupt:
			return NULL;
		}
	}

	//-- Bypass the color map, if present ------------------------------------

	if (hdr.cmlen) {
		if (fseek(stream,(long)hdr.cmlen*(long)((hdr.cmes+7)/8),SEEK_CUR)!=0)
			goto corrupt;
	}

	PixelBuf   line64(hdr.width);
	PixelBuf8  line8(hdr.width);

	//-- Read Image File -----------------------------------------------------

	switch (hdr.imgtype) {

		//-- Uncompressed RGB -----------------------------

		case 3:  {
			for(int iy = hdr.height - 1; iy >= 0; --iy) {
				UBYTE *l8=line8.Ptr();
				BMM_Color_64 *l64=line64.Ptr();
				TGAREAD(l8,sizeof(UBYTE)*hdr.width);
				for(int ix=0; ix<hdr.width; ++ix, ++l8, ++l64) {
					l64->r = l64->g = l64->b = (WORD)*l8 << 8;
					l64->a = 0;
				}
                if(we_did_abort(fbi))
                    goto bail_out;
				if(s->PutPixels(0,iy,hdr.width,line64.Ptr())!=1)
					goto bail_out;
			}
			}
			break;
		
		//-- Compressed RGB -------------------------------

		case 11: {
			
			int x = 0;
			int y = hdr.height - 1;
			BYTE rle;
			UBYTE pixel;
			BMM_Color_64 pixel64;
			BMM_Color_64 *l64=line64.Ptr();
			
			while(1) {
				
				TGAREAD(&rle,1);
				
				//-- Compressed Block
				
				if(rle>127) {
					rle-=127;
					TGAREAD(&pixel,1);
					pixel64.r = pixel64.g = pixel64.b = (WORD)pixel << 8;
					pixel64.a = 0;
					for(int ix=0; ix<rle; ++ix) {
						l64[x++] = pixel64;
						if(x>=hdr.width) {
                        if(we_did_abort(fbi))
                            goto bail_out;
						if(s->PutPixels(0,y,hdr.width,line64.Ptr())!=1)
							goto bail_out;
						x=0;
						y--;
						if(y<0)
							goto done;
						}
					}

				//-- Uncompressed Block
				
				} else {

					rle++;
					for(int ix=0; ix<rle; ++ix) {
						TGAREAD(&pixel,1);
						pixel64.r = pixel64.g = pixel64.b = (WORD)pixel << 8;
						pixel64.a = 0;
						l64[x++] = pixel64;
						if(x>=hdr.width) {
                        if(we_did_abort(fbi))
                            goto bail_out;
						if(s->PutPixels(0,y,hdr.width,line64.Ptr())!=1)
							goto bail_out;
						x=0;
						y--;
						if(y<0)
							goto done;
						}
					}
				}
			}
			}
			break;
		}

	done:
	return s;

}

//-----------------------------------------------------------------------------
// *> BitmapIO_TGA::Load16BitTGA()
//
//    Load an 16-bit TGA file, returning the storage location

BitmapStorage *BitmapIO_TGA::Load16BitTGA(BitmapInfo *fbi, FILE *stream, BitmapManager *manager) {


	//-- Create a storage for this bitmap ------------------------------------

	BitmapStorage *s = BMMCreateStorage(manager,BMM_TRUE_16);

	if(!s)
		return NULL;

	if (s->Allocate(fbi,manager,BMM_OPEN_R)==0) {
		bail_out:
		if(s) {
			delete s;
			s = NULL;
		}
		return NULL;
	}

	//-- Bypass the image identification field, if present -------------------

	if (hdr.idlen) {
		if (fseek(stream,(long)hdr.idlen,SEEK_CUR)!=0) {
			corrupt:
			return NULL;
		}
	}

	//-- Bypass the color map, if present ------------------------------------

	if (hdr.cmlen) {
		if (fseek(stream,(long)hdr.cmlen*(long)((hdr.cmes+7)/8),SEEK_CUR)!=0)
			goto corrupt;
	}

	PixelBuf   line64(hdr.width);
	PixelBuf16 line16(hdr.width);

	//-- Read Image File -----------------------------------------------------

	switch (hdr.imgtype) {

		//-- Uncompressed RGB -----------------------------

		case 2:  {
			for(int iy = hdr.height - 1; iy >= 0; --iy) {
				USHORT *l16=line16.Ptr();
				BMM_Color_64 *l64=line64.Ptr();
				TGAREAD(l16,sizeof(USHORT)*hdr.width);
				for(int ix=0; ix<hdr.width; ++ix, ++l16, ++l64) {
					l64->r = (WORD)(*l16 & 0x7c00) << 1;
					l64->g = (WORD)(*l16 & 0x03e0) << 6;
					l64->b = (WORD)(*l16 & 0x001f) << 11;
					l64->a = (*l16 & 0x8000) ? 0xffff : 0;
				}
                if(we_did_abort(fbi))
                    goto bail_out;
				if(s->PutPixels(0,iy,hdr.width,line64.Ptr())!=1)
					goto bail_out;
			}
			}
			break;
		
		//-- Compressed RGB -------------------------------

		case 10: {
			
			int x = 0;
			int y = hdr.height - 1;
			BYTE rle;
			USHORT pixel;
			BMM_Color_64 pixel64;
			BMM_Color_64 *l64=line64.Ptr();
			
			while(1) {
				
				TGAREAD(&rle,1);
				
				//-- Compressed Block
				
				if(rle>127) {
					rle-=127;
					TGAREAD(&pixel,2);
					pixel64.r = (WORD)(pixel & 0x7c00) << 1;
					pixel64.g = (WORD)(pixel & 0x03e0) << 6;
					pixel64.b = (WORD)(pixel & 0x001f) << 11;
					pixel64.a = (pixel & 0x8000) ? 0xffff : 0;
					for(int ix=0; ix<rle; ++ix) {
						l64[x++] = pixel64;
						if(x>=hdr.width) {
                        if(we_did_abort(fbi))
                            goto bail_out;
						if(s->PutPixels(0,y,hdr.width,line64.Ptr())!=1)
							goto bail_out;
						x=0;
						y--;
						if(y<0)
							goto done;
						}
					}

				//-- Uncompressed Block
				
				} else {

					rle++;
					for(int ix=0; ix<rle; ++ix) {
						TGAREAD(&pixel,2);
						pixel64.r = (WORD)(pixel & 0x7c00) << 1;
						pixel64.g = (WORD)(pixel & 0x03e0) << 6;
						pixel64.b = (WORD)(pixel & 0x001f) << 11;
						pixel64.a = (pixel & 0x8000) ? 0xffff : 0;
						l64[x++] = pixel64;
						if(x>=hdr.width) {
                        if(we_did_abort(fbi))
                            goto bail_out;
						if(s->PutPixels(0,y,hdr.width,line64.Ptr())!=1)
							goto bail_out;
						x=0;
						y--;
						if(y<0)
							goto done;
						}
					}
				}
			}
			}
			break;
		}

	done:
	return s;

}

//-----------------------------------------------------------------------------
// *> BitmapIO_TGA::Load24BitTGA()
//
//    Load an 24-bit TGA file, returning the storage location

BitmapStorage *BitmapIO_TGA::Load24BitTGA(BitmapInfo *fbi, FILE *stream, BitmapManager *manager) {

	//-- Create a storage for this bitmap ------------------------------------

	BitmapStorage *s = BMMCreateStorage(manager,BMM_TRUE_32);

	if(!s)
		return NULL;

	if (s->Allocate(fbi,manager,BMM_OPEN_R)==0) {
		bail_out:
		if(s) {
			delete s;
			s = NULL;
		}
		return NULL;
	}

	//-- Bypass the image identification field, if present -------------------

	if (hdr.idlen) {
		if (fseek(stream,(long)hdr.idlen,SEEK_CUR)!=0) {
			corrupt:
			return NULL;
		}
	}

	//-- Bypass the color map, if present ------------------------------------

	if (hdr.cmlen) {
		if (fseek(stream,(long)hdr.cmlen*(long)((hdr.cmes+7)/8),SEEK_CUR)!=0)
			goto corrupt;
	}

	PixelBuf   line64(hdr.width);
	PixelBuf24 line24(hdr.width);

	//-- Read Image File -----------------------------------------------------

	switch (hdr.imgtype) {

		//-- Uncompressed RGB -----------------------------

		case 2:  {
			for(int iy = hdr.height - 1; iy >= 0; --iy) {
				BMM_Color_24 *l24=line24.Ptr();
				BMM_Color_64 *l64=line64.Ptr();
				TGAREAD(l24,sizeof(BMM_Color_24)*hdr.width);
				for(int ix=0; ix<hdr.width; ++ix, ++l24, ++l64) {
					l64->r = (WORD)l24->b << 8;
					l64->g = (WORD)l24->g << 8;
					l64->b = (WORD)l24->r << 8;
					l64->a = 0;
				}
                if(we_did_abort(fbi))
                    goto bail_out;
				if(s->PutPixels(0,iy,hdr.width,line64.Ptr())!=1)
					goto bail_out;
			}
			}
			break;
		
		//-- Compressed RGB -------------------------------

		case 10: {
			
			int x = 0;
			int y = hdr.height - 1;
			BYTE rle;
			BMM_Color_24 pixel;
			BMM_Color_64 pixel64;
			BMM_Color_64 *l64=line64.Ptr();
			
			while(1) {
				
				TGAREAD(&rle,1);
				
				//-- Compressed Block
				
				if(rle>127) {
					rle-=127;
					TGAREAD(&pixel,3);
					pixel64.r = (WORD)pixel.b << 8;
					pixel64.g = (WORD)pixel.g << 8;
					pixel64.b = (WORD)pixel.r << 8;
					pixel64.a = 0;
					for(int ix=0; ix<rle; ++ix) {
						l64[x++] = pixel64;
						if(x>=hdr.width) {
                        if(we_did_abort(fbi))
                            goto bail_out;
						if(s->PutPixels(0,y,hdr.width,line64.Ptr())!=1)
							goto bail_out;
						x=0;
						y--;
						if(y<0)
							goto done;
						}
					}

				//-- Uncompressed Block
				
				} else {

					rle++;
					for(int ix=0; ix<rle; ++ix) {
						TGAREAD(&pixel,3);
						pixel64.r = (WORD)pixel.b << 8;
						pixel64.g = (WORD)pixel.g << 8;
						pixel64.b = (WORD)pixel.r << 8;
						pixel64.a = 0;
						l64[x++] = pixel64;
					if(x>=hdr.width) {
                        if(we_did_abort(fbi))
                            goto bail_out;
						if(s->PutPixels(0,y,hdr.width,line64.Ptr())!=1)
							goto bail_out;
						x=0;
						y--;
						if(y<0)
							goto done;
						}
					}
				}
			}
			}
			break;
		}

	done:
	return s;

}

//-----------------------------------------------------------------------------
// *> BitmapIO_TGA::Load32BitTGA()
//
//    Load an 32-bit TGA file, returning the storage location

BitmapStorage *BitmapIO_TGA::Load32BitTGA(BitmapInfo *fbi, FILE *stream, BitmapManager *manager) {

	//-- Create a storage for this bitmap ------------------------------------

	BitmapStorage *s = BMMCreateStorage(manager,BMM_TRUE_32);

	if(!s)
		return NULL;

	fbi->SetFlags(MAP_HAS_ALPHA);

	if (s->Allocate(fbi,manager,BMM_OPEN_R)==0) {
		bail_out:
		if(s) {
			delete s;
			s = NULL;
		}
		return NULL;
	}

	//-- Bypass the image identification field, if present -------------------

	if (hdr.idlen) {
		if (fseek(stream,(long)hdr.idlen,SEEK_CUR)!=0) {
			corrupt:
			return NULL;
		}
	}

	//-- Bypass the color map, if present ------------------------------------

	if (hdr.cmlen) {
		if (fseek(stream,(long)hdr.cmlen*(long)((hdr.cmes+7)/8),SEEK_CUR)!=0)
			goto corrupt;
	}

	PixelBuf   line64(hdr.width);
	PixelBuf32 line32(hdr.width);

	BOOL nonPreMult = (extra.alpha_attributes&TGA_ALPHA_PREMULT)?0:1;

	//-- Read Image File -----------------------------------------------------

	switch (hdr.imgtype) {

		//-- Uncompressed RGB -----------------------------

		case 2:  {
			for(int iy = hdr.height - 1; iy >= 0; --iy) {
				BMM_Color_32 *l32=line32.Ptr();
				BMM_Color_64 *l64=line64.Ptr();
				TGAREAD(l32,sizeof(BMM_Color_32)*hdr.width);
				if (nonPreMult) {
					for(int ix=0; ix<hdr.width; ++ix, ++l32, ++l64) {
						WORD a = l32->a;
						l64->r = (WORD)l32->b*a;
						l64->g = (WORD)l32->g*a;
						l64->b = (WORD)l32->r*a;
						l64->a = (WORD)l32->a<<8;
						}
					}
				else {
					for(int ix=0; ix<hdr.width; ++ix, ++l32, ++l64) {
						l64->r = (WORD)l32->b << 8;
						l64->g = (WORD)l32->g << 8;
						l64->b = (WORD)l32->r << 8;
						l64->a = (WORD)l32->a << 8;
						}
					}
	            if(we_did_abort(fbi))
                    goto bail_out;
				if(s->PutPixels(0,iy,hdr.width,line64.Ptr())!=1)
					goto bail_out;
				}
			}
			break;
			
		//-- Compressed RGB -------------------------------

		case 10: {
			
			int x = 0;
			int y = hdr.height - 1;
			BYTE rle;
			BMM_Color_32 pixel;
			BMM_Color_64 pixel64;
			BMM_Color_64 *l64=line64.Ptr();
			
			while(1) {
				
				TGAREAD(&rle,1);

				//-- Compressed Block
				
				if(rle>127) {
					rle-=127;
					TGAREAD(&pixel,4);
					if (nonPreMult) {
						WORD a = pixel.a;
						pixel64.r = (WORD)pixel.b*a;
						pixel64.g = (WORD)pixel.g*a;
						pixel64.b = (WORD)pixel.r*a;
						pixel64.a = (WORD)pixel.a << 8;
						}
					else {
						pixel64.r = (WORD)pixel.b << 8;
						pixel64.g = (WORD)pixel.g << 8;
						pixel64.b = (WORD)pixel.r << 8;
						pixel64.a = (WORD)pixel.a << 8;
						}
					for(int ix=0; ix<rle; ++ix) {
						l64[x++] = pixel64;
						if(x>=hdr.width) {
                        if(we_did_abort(fbi))
                            goto bail_out;
						if(s->PutPixels(0,y,hdr.width,line64.Ptr())!=1)
							goto bail_out;
						x=0;
						y--;
						if(y<0)
							goto done;
						}
					}

				//-- Uncompressed Block
				
				} else {

					rle++;
					for(int ix=0; ix<rle; ++ix) {
						TGAREAD(&pixel,4);
						if (nonPreMult) {
							WORD a = pixel.a;
							pixel64.r = (WORD)pixel.b*a;
							pixel64.g = (WORD)pixel.g*a;
							pixel64.b = (WORD)pixel.r*a;
							pixel64.a = (WORD)pixel.a << 8;
							}
						else {
							pixel64.r = (WORD)pixel.b << 8;
							pixel64.g = (WORD)pixel.g << 8;
							pixel64.b = (WORD)pixel.r << 8;
							pixel64.a = (WORD)pixel.a << 8;
							}
						l64[x++]  = pixel64;
						if(x>=hdr.width) {
                        if(we_did_abort(fbi))
                            goto bail_out;
						if(s->PutPixels(0,y,hdr.width,line64.Ptr())!=1)
							goto bail_out;
						x=0;
						y--;
						if(y<0)
							goto done;
						}
					}
				}
			}
			}
			break;
		}

	done:
	return s;

}

//-----------------------------------------------------------------------------
// *> BitmapIO_TGA::Load32BitTGA()
//
//    Load an TGA file, returning the storage location

BitmapStorage *BitmapIO_TGA::ReadTGAFile(BitmapInfo *fbi, BitmapManager *manager, BMMRES *status) {

	BitmapStorage *s = NULL;
	gotGamma = FALSE;

	File file(fbi->Name(), _T("rb"));

	if(!(inStream = file.stream)) {
		*status = ProcessImageIOError(fbi);
		return NULL;
	}

	if (!ReadFooter()) {
		*status = ProcessImageIOError(fbi,BMMRES_BADFILEHEADER);
		return NULL;
	}

	// Return to the beginning of the file
	fseek(inStream, 0L, SEEK_SET);
		
	if (ReadHeader()) {

		hflip     = (hdr.desc & 0x10) ? TRUE : FALSE;   // Need hflip
		vflip     = (hdr.desc & 0x20) ? TRUE : FALSE;   // Need vflip
		hdr.desc &= 0xcf;                               // Mask off flip bits

		fbi->SetWidth (hdr.width);
		fbi->SetHeight(hdr.height);
		fbi->SetAspect (aspect);

		if (gotGamma) {
			fbi->SetGamma(gamma);
			}

		switch(hdr.pixsize) {

			case 8:
			if (hdr.desc!=0 && hdr.desc!=1 && hdr.desc!=8)
				break;
			// Check for grayscale
			switch(hdr.imgtype) {
				case 1:
				case 9:
					fbi->SetType(BMM_PALETTED);   //-- Must check for grayscale
					s = Load8BitPalTGA(fbi,file.stream, manager);
					break;
				case 3:
				case 11:
					fbi->SetType(BMM_GRAY_8);   //-- Must check for grayscale
					s = Load8BitGrayTGA(fbi,file.stream, manager);
					break;
				}
			break;

			case 16:
			if(hdr.desc!=0 && hdr.desc!=1)
				break;
			fbi->SetType(BMM_TRUE_16);
			s = Load16BitTGA(fbi,file.stream, manager);
			break;

			case 24:
			// GG:11/06/02
			// hdr.desc bits 0..3 define the number of alpha bits. As a 24 bit image has no
			// alpha, a non zero value here makes no sense. However, Adobe Premiere outputs
			// a 24 bit TGA with this set to 8! Hey... go complain to Adobe...
			//if(hdr.desc!=0)
			//	break;
			fbi->SetType(BMM_TRUE_32);
			s = Load24BitTGA(fbi,file.stream, manager);
			break;

			case 32:  
			fbi->SetType(BMM_TRUE_32);
			s = Load32BitTGA(fbi,file.stream, manager);
			break;
		
		}
		
	
	}

	//-- Perform clean-up operations!  

	if (s) {
		int width = s->Width();
		int height = s->Height();
		if (gotGamma) {
			s->SetGamma(gamma);
			}
		if(hflip) {
			PixelBuf buf(width);
			BMM_Color_64 *line = buf.Ptr();
			for(int y = 0; y < height; ++y) {
				s->GetPixels(0, y, width, line);
				for(int xl = 0, xr = width-1; xl < xr; ++xl, --xr) {
					BMM_Color_64 temp = line[xl];
					line[xl] = line[xr];
					line[xr] = temp;
					}
				s->PutPixels(0, y, width, line);
				}
			}
		if(vflip) {
			PixelBuf buf1(width), buf2(width);
			BMM_Color_64 *line1 = buf1.Ptr();
			BMM_Color_64 *line2 = buf2.Ptr();
			for(int yt = 0, yb = height-1; yt < yb; ++yt, --yb) {
				s->GetPixels(0, yt, width, line1);
				s->GetPixels(0, yb, width, line2);
				s->PutPixels(0, yt, width, line2);
				s->PutPixels(0, yb, width, line1);
				}
			}
		return s;
	}

	//-- If we get here, something went wrong!

	*status = ProcessImageIOError(fbi,GetString(IDS_READ_ERROR));
	return NULL;

}

BitmapIO_TGA::BitmapIO_TGA() {
	gamma = aspect = 1.0f;
	InitUserData(&UserData);
	// Zero out info fields
	memset(&hdr, 0, sizeof(TGAHeader));
	memset(&foot, 0, sizeof(TGAFooter));
	memset(&extra, 0, sizeof(TGAExtra));
	}

//-----------------------------------------------------------------------------
// #> BitmapIO_TGA::Ext(int n)
const TCHAR *BitmapIO_TGA::Ext( int n ) {
	switch(n) {
		case 0:
			return _T("tga");
		case 1:
			return _T("vda");
		case 2:
			return _T("icb");
		case 3:
			return _T("vst");
		}
	return _T("");
	}
 
//-----------------------------------------------------------------------------
// #> BitmapIO_TGA::LongDesc()

const TCHAR *BitmapIO_TGA::LongDesc()  {
	return GetString(IDS_TARGA_FILE);
}
	
//-----------------------------------------------------------------------------
// #> BitmapIO_TGA::ShortDesc()

const TCHAR *BitmapIO_TGA::ShortDesc() {
	return GetString(IDS_TARGA);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_TGA::LoadConfigure()

BOOL BitmapIO_TGA::LoadConfigure ( void *ptr ) {
	TGAUSERDATA *buf = (TGAUSERDATA *)ptr;
	if (buf->version == TGAVERSION) {
		memcpy((void *)&UserData,ptr,sizeof(TGAUSERDATA));
		return (TRUE);
	} else
		return (FALSE);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_TGA::SaveConfigure()

BOOL BitmapIO_TGA::SaveConfigure ( void *ptr ) {
	if (ptr) {
 		UserData.saved = TRUE;
		memcpy(ptr,(void *)&UserData,sizeof(TGAUSERDATA));
		return (TRUE);
	} else
		return (FALSE);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_TGA::EvaluateConfigure()

DWORD BitmapIO_TGA::EvaluateConfigure ( ) {
	return (sizeof(TGAUSERDATA));
}


//-----------------------------------------------------------------------------
// *> AboutCtrlDlgProc()
//

INT_PTR CALLBACK AboutCtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {

	switch (message) {
		
		case WM_INITDIALOG: 
			CenterWindow(hWnd,GetParent(hWnd));
			return 1;
		

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:              
				case IDCANCEL:
					EndDialog(hWnd,1);
					break;
			}
			return 1;

	}
	
	return 0;

}

//-----------------------------------------------------------------------------
// #> BitmapIO_TGA::ShowAbout()

void BitmapIO_TGA::ShowAbout(HWND hWnd) {
	DialogBoxParam(
		hInst,
		MAKEINTRESOURCE(IDD_ABOUT),
		hWnd,
		(DLGPROC)AboutCtrlDlgProc,
		(LPARAM)0);
}

//-----------------------------------------------------------------------------
// *> ControlCtrlDlgProc()
//

BOOL BitmapIO_TGA::Control(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {

	static BOOL forced = FALSE;

	switch (message) {
		
		case WM_INITDIALOG: {
	
			CenterWindow(hWnd,GetParent(hWnd));
			SetCursor(LoadCursor(NULL,IDC_ARROW));

			InitCommonControls();
			
			if (!UserData.saved)
				ReadCfg();

			CheckDlgButton(hWnd,IDC_16BITS,UserData.writeType == TGA_16_BITS ? TRUE : FALSE); 
			CheckDlgButton(hWnd,IDC_24BITS,UserData.writeType == TGA_24_BITS ? TRUE : FALSE); 
			CheckDlgButton(hWnd,IDC_32BITS,UserData.writeType == TGA_32_BITS ? TRUE : FALSE); 
			CheckDlgButton(hWnd,IDC_COMPRESS,UserData.compressed); 
			CheckDlgButton(hWnd,IDC_ALPHA_SPLIT,UserData.alphaSplit); 
			CheckDlgButton(hWnd,IDC_PREMULT_ALPHA,UserData.preMultAlpha); 

			// Limit edit fields' lengths
			SendMessage(GetDlgItem(hWnd,IDC_AUTHORNAME), EM_LIMITTEXT, 40, 0);
			SendMessage(GetDlgItem(hWnd,IDC_JOBNAME), EM_LIMITTEXT, 40, 0);
			SendMessage(GetDlgItem(hWnd,IDC_COMMENTS1), EM_LIMITTEXT, 80, 0);
			SendMessage(GetDlgItem(hWnd,IDC_COMMENTS2), EM_LIMITTEXT, 80, 0);
			SendMessage(GetDlgItem(hWnd,IDC_COMMENTS3), EM_LIMITTEXT, 80, 0);
			SendMessage(GetDlgItem(hWnd,IDC_COMMENTS4), EM_LIMITTEXT, 80, 0);

			// Stuff the current text into the edit fields
			SendMessage(GetDlgItem(hWnd,IDC_AUTHORNAME), WM_SETTEXT, 0, (LPARAM)UserData.author);
			SendMessage(GetDlgItem(hWnd,IDC_JOBNAME), WM_SETTEXT, 0, (LPARAM)UserData.jobname);
			SendMessage(GetDlgItem(hWnd,IDC_COMMENTS1), WM_SETTEXT, 0, (LPARAM)UserData.comments1);
			SendMessage(GetDlgItem(hWnd,IDC_COMMENTS2), WM_SETTEXT, 0, (LPARAM)UserData.comments2);
			SendMessage(GetDlgItem(hWnd,IDC_COMMENTS3), WM_SETTEXT, 0, (LPARAM)UserData.comments3);
			SendMessage(GetDlgItem(hWnd,IDC_COMMENTS4), WM_SETTEXT, 0, (LPARAM)UserData.comments4);
			
			return 1;
			
		}

		case WM_COMMAND:

			switch (LOWORD(wParam)) {
				
				case IDOK: {
					if(IsDlgButtonChecked(hWnd,IDC_16BITS))
						UserData.writeType = TGA_16_BITS;
					else
					if(IsDlgButtonChecked(hWnd,IDC_24BITS))
						UserData.writeType = TGA_24_BITS;
					else
					if(IsDlgButtonChecked(hWnd,IDC_32BITS))
						UserData.writeType = TGA_32_BITS;

					UserData.compressed = IsDlgButtonChecked(hWnd,IDC_COMPRESS) ? TRUE : FALSE;
					UserData.alphaSplit = IsDlgButtonChecked(hWnd,IDC_ALPHA_SPLIT) ? TRUE : FALSE;
					UserData.preMultAlpha = IsDlgButtonChecked(hWnd,IDC_PREMULT_ALPHA) ? TRUE : FALSE;

					TCHAR work[100];	// Max editable is 80 chars + 1
					int len = SendDlgItemMessage(hWnd, IDC_AUTHORNAME, WM_GETTEXTLENGTH, 0, 0);
					SendDlgItemMessage(hWnd, IDC_AUTHORNAME, WM_GETTEXT, len+1, (LPARAM)work);
					work[40] = 0;
					_tcscpy(UserData.author, work);
					len = SendDlgItemMessage(hWnd, IDC_JOBNAME, WM_GETTEXTLENGTH, 0, 0);
					SendDlgItemMessage(hWnd, IDC_JOBNAME, WM_GETTEXT, len+1, (LPARAM)work);
					work[40] = 0;
					_tcscpy(UserData.jobname, work);
					len = SendDlgItemMessage(hWnd, IDC_COMMENTS1, WM_GETTEXTLENGTH, 0, 0);
					SendDlgItemMessage(hWnd, IDC_COMMENTS1, WM_GETTEXT, len+1, (LPARAM)work);
					work[80] = 0;
					_tcscpy(UserData.comments1, work);
					len = SendDlgItemMessage(hWnd, IDC_COMMENTS2, WM_GETTEXTLENGTH, 0, 0);
					SendDlgItemMessage(hWnd, IDC_COMMENTS2, WM_GETTEXT, len+1, (LPARAM)work);
					work[80] = 0;
					_tcscpy(UserData.comments2, work);
					len = SendDlgItemMessage(hWnd, IDC_COMMENTS3, WM_GETTEXTLENGTH, 0, 0);
					SendDlgItemMessage(hWnd, IDC_COMMENTS3, WM_GETTEXT, len+1, (LPARAM)work);
					work[80] = 0;
					_tcscpy(UserData.comments3, work);
					len = SendDlgItemMessage(hWnd, IDC_COMMENTS4, WM_GETTEXTLENGTH, 0, 0);
					SendDlgItemMessage(hWnd, IDC_COMMENTS4, WM_GETTEXT, len+1, (LPARAM)work);
					work[80] = 0;
					_tcscpy(UserData.comments4, work);

					WriteCfg();

					EndDialog(hWnd,1);
					}
					break;

				case IDCANCEL:
					EndDialog(hWnd,0);
					break;
		
			}
			return 1;

	}
	
	return 0;

}

//-----------------------------------------------------------------------------
// *> ControlDlgProc
//

static INT_PTR CALLBACK ControlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	static BitmapIO_TGA *bm = NULL;
	if (message == WM_INITDIALOG) 
		bm = (BitmapIO_TGA *)lParam;
	if (bm) 
		return (bm->Control(hWnd,message,wParam,lParam));
	else
		return(FALSE);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_TGA::Control()

BOOL BitmapIO_TGA::ShowControl(HWND hWnd, DWORD flag ) {
	return (
		DialogBoxParam(
		hInst,
		MAKEINTRESOURCE(IDD_TARGA_CONTROL),
		hWnd,
		(DLGPROC)ControlDlgProc,
		(LPARAM)this)
	);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_TGA::GetImageInfo()

BMMRES BitmapIO_TGA::GetImageInfo ( BitmapInfo *fbi ) {

	//-- Get File Header
	
	File file(fbi->Name(), _T("rb"));

	if(!(inStream = file.stream))
		return (ProcessImageIOError(fbi));

	if (!ReadFooter())
		return (ProcessImageIOError(fbi,BMMRES_BADFILEHEADER));

	// Return to the beginning of the file
	fseek(inStream, 0L, SEEK_SET);

	if (ReadHeader()) {
	
		fbi->SetWidth(hdr.width);
		fbi->SetHeight(hdr.height);
		fbi->SetGamma (gamma);
		fbi->SetAspect (aspect);
		fbi->SetFirstFrame(0);
		fbi->SetLastFrame(0);

		switch(hdr.pixsize) {
			case 8:
				// Check for grayscale
				switch(hdr.imgtype) {
					case 1:
					case 9:
						fbi->SetType(BMM_PALETTED);
						break;
					case 3:
					case 11:
						fbi->SetType(BMM_GRAY_8);
						break;
					default:
						return BMMRES_INVALIDFORMAT;
					}
				break;
			case 16:
				fbi->SetType(BMM_TRUE_16);
				break;
			case 24:
				fbi->SetType(BMM_TRUE_24);
				break;
			case 32:  
				fbi->SetType(BMM_TRUE_32);
				break;
			default:
				return BMMRES_INVALIDFORMAT;
		}
		
		return BMMRES_SUCCESS;
	} else
		return (ProcessImageIOError(fbi,BMMRES_BADFILEHEADER));

}

//-----------------------------------------------------------------------------
// #> BitmapIO_TGA::ImageInfoDlg
//

BOOL BitmapIO_TGA::ImageInfoDlg(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {

	switch (message) {
	
		case WM_INITDIALOG: {

			CenterWindow(hWnd,GetParent(hWnd));
			SetCursor(LoadCursor(NULL,IDC_ARROW));

			//-- Filename --------------------------------
			
			SetDlgItemText(hWnd, IDC_INFO_FILENAME, infoBI->Name());
		
			//-- Handle Resolution -----------------------
		
			char buf[64];
			sprintf(buf,"%dx%d",hdr.width,hdr.height);
			SetDlgItemText(hWnd, IDC_INFO_RESOLUTION, buf);
		
			//-- Handle Ganna & Aspect -------------------
		
			sprintf(buf,"%.2f",gamma);
			SetDlgItemText(hWnd, IDC_BMM_INFO_GAMMA, buf);
			sprintf(buf,"%.2f",aspect);
			SetDlgItemText(hWnd, IDC_BMM_INFO_AR, buf);

			//-- Handle Type -----------------------------
		
			int type = IDS_TGA_UNKNOWN;
			switch(infoBI->Type()) {
				case BMM_PALETTED:
					type = IDS_TGA_PALETTED;
					break;
				case BMM_GRAY_8:
					type = IDS_TGA_GRAY_8;
					break;
				case BMM_TRUE_16:
					type = IDS_TGA_TRUE_16;
					break;
				case BMM_TRUE_24:
					type = IDS_TGA_TRUE_24;
					break;
				case BMM_TRUE_32:
					type = IDS_TGA_TRUE_32;
					break;
				}
			TCHAR text[128];
			LoadString(hInst,type,text,128);
			SetDlgItemText(hWnd, IDC_INFO_TYPE,text);

			//-- Handle Author Name ----------------------

			SetDlgItemText(hWnd, IDC_INFO_AUTHOR, extra.authorname);

			//-- Handle Job Name -------------------------

			SetDlgItemText(hWnd, IDC_INFO_JOB, extra.jobname);

			//-- Handle Comments -------------------------

			SetDlgItemText(hWnd, IDC_INFO_COMMENT1, &extra.comments[0] );
			SetDlgItemText(hWnd, IDC_INFO_COMMENT2, &extra.comments[81] );
			SetDlgItemText(hWnd, IDC_INFO_COMMENT3, &extra.comments[162] );
			SetDlgItemText(hWnd, IDC_INFO_COMMENT4, &extra.comments[243] );

			//-- Date, Size, Frames ----------------------

			char sizetxt[128]="";
			char datetxt[128]="";

			BMMGetFullFilename(infoBI);

			HANDLE findhandle;
			WIN32_FIND_DATA file;
			SYSTEMTIME t,l;
			findhandle = FindFirstFile(infoBI->Name(),&file);
			FindClose(findhandle);
			if (findhandle != INVALID_HANDLE_VALUE) {
				sprintf(sizetxt,"%d",file.nFileSizeLow);
				FileTimeToSystemTime(&file.ftLastWriteTime,&t);
				if(!SystemTimeToTzSpecificLocalTime(NULL,&t,&l))
					l = t;
				sprintf(datetxt,"%d/%02d/%02d %02d:%02d:%02d",l.wYear,
					l.wMonth,l.wDay,l.wHour,l.wMinute,l.wSecond);
				}

			SetDlgItemText(hWnd, IDC_INFO_SIZE,sizetxt);
			SetDlgItemText(hWnd, IDC_INFO_DATE,datetxt);
			SetDlgItemText(hWnd, IDC_INFO_FRAMES,_T("1"));
			return 1;
			
			}

		case WM_COMMAND:

			switch (LOWORD(wParam)) {

				//-- Changes Accepted ---------------------

				case IDCANCEL:
				case IDOK:
						EndDialog(hWnd,1);
						break;
		
				}
			return 1;

		}
	return 0;

	}

//-----------------------------------------------------------------------------
// *> ImageInfoDlgProc()
//

static INT_PTR CALLBACK InfoCtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	static BitmapIO_TGA *io = NULL;
	if (message == WM_INITDIALOG) 
		io = (BitmapIO_TGA *)lParam;
	if (io) 
		return (io->ImageInfoDlg(hWnd,message,wParam,lParam));
	else
		return(FALSE);
	}

//-----------------------------------------------------------------------------
// #> BitmapIO_TGA::GetImageInfoDlg()

BMMRES BitmapIO_TGA::GetImageInfoDlg(HWND hWnd, BitmapInfo *fbi, const TCHAR *filename) {

	//-- Take care of local copy of BitmapInfo
	
	infoBI = new BitmapInfo;
	
	if (!infoBI)
		return(BMMRES_MEMORYERROR);
	
	infoBI->Copy(fbi);

	//-- Prepare BitmapInfo if needed
	
	if (filename)
		infoBI->SetName(filename);

	//-- Get File Info -------------------------
	
	BMMRES res = GetImageInfo (infoBI);
	
	if (res != BMMRES_SUCCESS) {
		delete infoBI;
		infoBI = NULL;
		return (res);
		}
	
	//-- Display Dialogue ----------------------
	
	DialogBoxParam(
		hInst,
		MAKEINTRESOURCE(IDD_TARGA_INFO),
		hWnd,
		(DLGPROC)InfoCtrlDlgProc,
		(LPARAM)this);

	delete infoBI;
	infoBI = NULL;
		
	return (BMMRES_SUCCESS);
	
}

//-----------------------------------------------------------------------------
// #> BitmapIO_TGA::Load()

BitmapStorage *BitmapIO_TGA::Load(BitmapInfo *fbi, Bitmap *map, BMMRES *status) {

	//-- Initialize Status Optimistically

	*status = BMMRES_SUCCESS;

	//-- Make sure nothing weird is going on

	if(openMode != BMM_NOT_OPEN) {
		*status = ProcessImageIOError(fbi,BMMRES_INTERNALERROR);
		return NULL;
	}

	openMode = BMM_OPEN_R;
	loadMap  = map;
	
	BitmapStorage *s = ReadTGAFile(fbi, map->Manager(), status);

	if(!s)
		return NULL;

	//-- Set the storage's BitmapInfo

	s->bi.CopyImageInfo(fbi);

	return s;
}

//-----------------------------------------------------------------------------
// #> BitmapIO_TGA::OpenOutput()

BMMRES BitmapIO_TGA::OpenOutput(BitmapInfo *fbi, Bitmap *map) {

	if (openMode != BMM_NOT_OPEN)
		return (ProcessImageIOError(fbi,BMMRES_INTERNALERROR));
		
	if (!map)
		return (ProcessImageIOError(fbi,BMMRES_INTERNALERROR));
		
	//-- Check for Default Configuration -----------------
	
	if (!UserData.saved)
		ReadCfg();
	
	//-- Save Image Info Data

	bi.CopyImageInfo(fbi);    

	this->map   = map;
	openMode    = BMM_OPEN_W;

	return BMMRES_SUCCESS;

}

//-----------------------------------------------------------------------------
// #> BitmapIO_TGA::Write()
//

BMMRES BitmapIO_TGA::Write(int frame) {
	
	//-- If we haven't gone through an OpenOutput(), leave

	if (openMode != BMM_OPEN_W)
		return (ProcessImageIOError(&bi,BMMRES_INTERNALERROR));

	//-- Resolve Filename --------------------------------

	TCHAR filename[MAX_PATH];

	if (frame == BMM_SINGLEFRAME) {
		_tcscpy(filename,bi.Name());
	} else {
		if (!BMMCreateNumberedFilename(bi.Name(),frame,filename))
			return (ProcessImageIOError(&bi,BMMRES_NUMBEREDFILENAMEERROR));
	}
	
	//-- Create Image File -------------------------------
	
	File file(filename, _T("wb"));
	
	if (!file.stream) {
		return (ProcessImageIOError(&bi));
	}

	// Below this line formatted for Tom's editor (sorry.)

	// Find out what kind of output file we're dealing with

	saveStorage = map->Storage();
	if(!saveStorage)
		return (ProcessImageIOError(&bi,BMMRES_INTERNALERROR));

	BOOL hasAlpha = saveStorage->HasAlpha();
	
	int result = TGA_SAVE_UNSUPPORTED_TYPE;

	switch(UserData.writeType) {
		case TGA_16_BITS:
			result = Save16BitTGA(file.stream);
			break;
		case TGA_24_BITS:
			result = Save24BitTGA(file.stream);
			break;
		case TGA_32_BITS:
			result = Save32BitTGA(file.stream);
			break;
		}

	switch(result) {
		case TGA_SAVE_OK:
			break;
		case TGA_SAVE_UNSUPPORTED_TYPE:
			return (ProcessImageIOError(&bi,GetString(IDS_UNSUPPORTED)));
		case TGA_SAVE_WRITE_ERROR:
			return (ProcessImageIOError(&bi,GetString(IDS_WRITE_ERROR)));
		}

	//-- Create Alpha File If Necessary -------------------------------
	
	if(hasAlpha && UserData.alphaSplit) {
		// Break down the name into its component parts...
		TCHAR path[MAX_PATH],file[MAX_PATH],ext[MAX_PATH];
		BMMSplitFilename(bi.Name(),path,file,ext);
		TSTR workname;
		workname.printf("%sA_%s%s", path, file, ext);
		if (frame == BMM_SINGLEFRAME) {
			_tcscpy(filename,workname);
		} else {
			if (!BMMCreateNumberedFilename(workname,frame,filename)) {
				return (ProcessImageIOError(&bi,BMMRES_NUMBEREDFILENAMEERROR));
			}
		}

		File alphaFile(filename, _T("wb"));
	
		if (!alphaFile.stream) {
			return (ProcessImageIOError(&bi));
		}

		// Below this line formatted for Tom's editor (sorry.)

		// Find out what kind of output file we're dealing with

		saveStorage = map->Storage();
		if(!saveStorage)
			return (ProcessImageIOError(&bi,BMMRES_INTERNALERROR));

		BOOL hasAlpha = saveStorage->HasAlpha();
		
		switch(SaveAlpha(alphaFile.stream)) {
			case TGA_SAVE_OK:
				return BMMRES_SUCCESS;
			case TGA_SAVE_UNSUPPORTED_TYPE:
				return (ProcessImageIOError(&bi,GetString(IDS_UNSUPPORTED)));
			case TGA_SAVE_WRITE_ERROR:
			default:
				return (ProcessImageIOError(&bi,GetString(IDS_WRITE_ERROR)));
			}
		}
	else
		return BMMRES_SUCCESS;

}

//-----------------------------------------------------------------------------
// #> BitmapIO_TGA::SavexxBitTGA
//

/* 8-bit encoding macros */
#define   replicant8     (byte==*ptr)
#define   install8()     {*kpptr++=byte;}
/* 16-bit encoding macros */
#define   replicant16     (byte[0]==*ptr[0] && byte[1]==*ptr[1])
#define   install16()     {for(t=0; t<2; t++) *kpptr++=byte[t];}
/* 24-bit encoding macros */
#define   replicant24     (byte[0]==*ptr[0] && byte[1]==*ptr[1] && byte[2]==*ptr[2])
#define   install24()     {for(t=0; t<3; t++) *kpptr++=byte[t];}
/* 32-bit variants */
#define   replicant32     (byte[0]==*ptr[0] && byte[1]==*ptr[1] && byte[2]==*ptr[2] && byte[3]==*ptr[3])
#define   install32()     {for(t=0; t<4; t++) *kpptr++=byte[t];}

// RLE Encoders:

BYTE *
tga_encode8(BYTE *strip, BYTE *out, int cc)
{
  BYTE      *kpptr = out;
  uchar     *ptr, *lastliteral, byte;
  short     n, state=BASE;

  ptr = strip;

  while (cc > 0)
	{
	/* find longest string of identical bytes */
	byte = *ptr;    /* get sample */

	n=0;    /* clear counter */

	for (; cc>0 && replicant8; cc--,ptr++)
		n++;

	again:            /* sorry */

	switch(state)
		{
		case BASE:    /* initial state - set run or literal */
			if (n>1)
			{
				state = RUN;
				if (n>128)
				{
					*kpptr++ = 127+128;
					install8();
					n-=128;
					goto again;
				}
				*kpptr++ = 127+n;
				install8();
			}
			else
			{
				lastliteral = kpptr;
				*kpptr++ = 0;
				install8();
				state = LITERAL;
			}
			break;

		case LITERAL:
			if (n>1)
			{
				state = RUN;
				if (n>128)
				{
					*kpptr++ = 127+128;
					install8();
					n-=128;
					goto again;
				}
				*kpptr++ = 127+n;
				install8();
			}
			else
			{
				if (++(*lastliteral) == 127)
					state = BASE;
				install8();
			}
			break;

		case RUN:
			if (n>1)
			{
				if (n>128)
				{
					*kpptr++ = 127+128;
					install8();
					n-=128;
					goto again;
				}
				*kpptr++ = 127+n;
				install8();
			}
			else
			{
				lastliteral = kpptr;
				*kpptr++ = 0;
				install8();
				state = LITERAL;
			}
			break;
		}
	}
return kpptr;
}

BYTE *
tga_encode16(BYTE *strip, BYTE *out, int cc)
{
  BYTE *kpptr = out;
  BYTE     *ptr[2], *lastliteral, byte[2];
  short     t, n, state=BASE;

  for (t=0; t<2; t++)
	ptr[t] = strip+t;

  while (cc > 0)
	{
	/* find longest string of identical bytes */
	for (t=0; t<2; t++)
		byte[t] = *ptr[t];    /* get sample (R, G, or B) */

	n=0;    /* clear counter */

	for (; cc>0 && replicant16; cc--,ptr[0]+=2,ptr[1]+=2)
		n++;

	again:            /* sorry */

	switch(state)
		{
		case BASE:    /* initial state - set run or literal */
			if (n>1)
			{
				state = RUN;
				if (n>128)
				{
					*kpptr++ = 127+128;
					install16();
					n-=128;
					goto again;
				}
				*kpptr++ = 127+n;
				install16();
			}
			else
			{
				lastliteral = kpptr;
				*kpptr++ = 0;
				install16();
				state = LITERAL;
			}
			break;

		case LITERAL:
			if (n>1)
			{
				state = RUN;
				if (n>128)
				{
					*kpptr++ = 127+128;
					install16();
					n-=128;
					goto again;
				}
				*kpptr++ = 127+n;
				install16();
			}
			else
			{
				if (++(*lastliteral) == 127)
					state = BASE;
				install16();
			}
			break;

		case RUN:
			if (n>1)
			{
				if (n>128)
				{
					*kpptr++ = 127+128;
					install16();
					n-=128;
					goto again;
				}
				*kpptr++ = 127+n;
				install16();
			}
			else
			{
				lastliteral = kpptr;
				*kpptr++ = 0;
				install16();
				state = LITERAL;
			}
			break;
		}
	}
return kpptr;
}

BYTE *
tga_encode24(BYTE *strip, BYTE *out, int cc)
{
  BYTE *kpptr = out;
  BYTE     *ptr[3], *lastliteral, byte[3];
  short     t, n, state=BASE;

  for (t=0; t<3; t++)
	ptr[t] = strip+t;

  while (cc > 0)
	{
	/* find longest string of identical bytes */
	for (t=0; t<3; t++)
		byte[t] = *ptr[t];    /* get sample (R, G, or B) */

	n=0;    /* clear counter */

	for (; cc>0 && replicant24; cc--,ptr[0]+=3,ptr[1]+=3,ptr[2]+=3)
		n++;

	again:            /* sorry */

	switch(state)
		{
		case BASE:    /* initial state - set run or literal */
			if (n>1)
			{
				state = RUN;
				if (n>128)
				{
					*kpptr++ = 127+128;
					install24();
					n-=128;
					goto again;
				}
				*kpptr++ = 127+n;
				install24();
			}
			else
			{
				lastliteral = kpptr;
				*kpptr++ = 0;
				install24();
				state = LITERAL;
			}
			break;

		case LITERAL:
			if (n>1)
			{
				state = RUN;
				if (n>128)
				{
					*kpptr++ = 127+128;
					install24();
					n-=128;
					goto again;
				}
				*kpptr++ = 127+n;
				install24();
			}
			else
			{
				if (++(*lastliteral) == 127)
					state = BASE;
				install24();
			}
			break;

		case RUN:
			if (n>1)
			{
				if (n>128)
				{
					*kpptr++ = 127+128;
					install24();
					n-=128;
					goto again;
				}
				*kpptr++ = 127+n;
				install24();
			}
			else
			{
				lastliteral = kpptr;
				*kpptr++ = 0;
				install24();
				state = LITERAL;
			}
			break;
		}
	}
return kpptr;
}

/* 32-bit encoder */

BYTE *
tga_encode32(BYTE *strip, BYTE *out, int cc)
{
  BYTE *kpptr = out;
  BYTE     *ptr[4], *lastliteral, byte[4];
  short     t, n, state=BASE;

  for (t=0; t<4; t++)
	ptr[t] = strip+t;

  while (cc > 0)
	{
	/* find longest string of identical bytes */
	for (t=0; t<4; t++)
		byte[t] = *ptr[t];    /* get sample (R, G, B, Alpha) */

	n=0;    /* clear counter */

	for (; cc>0 && replicant32; cc--,ptr[0]+=4,ptr[1]+=4,ptr[2]+=4,ptr[3]+=4)
		n++;

	again:            /* sorry */

	switch(state)
		{
		case BASE:    /* initial state - set run or literal */
			if (n>1)
			{
				state = RUN;
				if (n>128)
				{
					*kpptr++ = 127+128;
					install32();
					n-=128;
					goto again;
				}
				*kpptr++ = 127+n;
				install32();
			}
			else
			{
				lastliteral = kpptr;
				*kpptr++ = 0;
				install32();
				state = LITERAL;
			}
			break;

		case LITERAL:
			if (n>1)
			{
				state = RUN;
				if (n>128)
				{
					*kpptr++ = 127+128;
					install32();
					n-=128;
					goto again;
				}
				*kpptr++ = 127+n;
				install32();
			}
			else
			{
				if (++(*lastliteral) == 127)
					state = BASE;
				install32();
			}
			break;

		case RUN:
			if (n>1)
			{
				if (n>128)
				{
					*kpptr++ = 127+128;
					install32();
					n-=128;
					goto again;
				}
				*kpptr++ = 127+n;
				install32();
			}
			else
			{
				lastliteral = kpptr;
				*kpptr++ = 0;
				install32();
				state = LITERAL;
			}
			break;
		}
	}
return kpptr;
}

inline int LIM031(int x)  { return (((x)<0)?0:((x)>31)?31:(x)); }

int BitmapIO_TGA::SaveAlpha( FILE *stream) {
	TGAHeader T;

	// Set up & write .TGA file header

	T.idlen=0;
	T.cmtype=0;
	T.imgtype=UserData.compressed ? 11 : 3;
	T.cmorg=T.cmlen=0;
	T.cmes=0;
	T.xorg=0;
	T.yorg=0;
	T.width=saveStorage->Width();
	T.height=saveStorage->Height();
	T.pixsize=8;
	T.desc=LOWER_LEFT;

	if(!TGAWRITE(&T,sizeof(TGAHeader))) {
		wrterr:
		return TGA_SAVE_WRITE_ERROR;
		}

	// Set up some buffers
	PixelBuf64 tgabuf(T.width);
	BMM_Color_64 *in = tgabuf.Ptr();
	PixelBuf8 outbuf(T.width);
	BYTE *out = outbuf.Ptr();

	// Compression buffers
	PixelBuf8 kpbuffer(T.width*2);			// Extra width just in case compression isn't efficient!
	BYTE *kpbuf = (BYTE *)kpbuffer.Ptr();

	// Now write the image!

	for(int y = T.height-1; y>=0; --y) {
		GetOutputPixels(0,y,T.width,in); 
		BMM_Color_64 *i=in;
		BYTE *o = out;
		for(int x = 0; x < T.width; ++x,++o,++i)
			*o = i->a >> 8;
		if (UserData.compressed) {         /* write compressed */
			BYTE *kpptr = tga_encode8(out, kpbuf, T.width);
			if (!TGAWRITE(kpbuf, kpptr-kpbuf))
				goto wrterr;
			}
		else {                    /* write uncompressed */
			if(TGAWRITE(out, T.width)==0)
				goto wrterr;
			}
		}

	// Write the information & footer
	if(!WriteTargaExtension(stream, TRUE, UserData.preMultAlpha))
		goto wrterr;

	return TGA_SAVE_OK;
	}

int BitmapIO_TGA::Save16BitTGA ( FILE *stream ) {
	TGAHeader T;

	// Set up & write .TGA file header

	T.idlen=0;
	T.cmtype=0;
	T.imgtype=UserData.compressed ? 10 : 2;
	T.cmorg=T.cmlen=0;
	T.cmes=0;
	T.xorg=0;
	T.yorg=0;
	T.width=saveStorage->Width();
	T.height=saveStorage->Height();
	T.pixsize=16;
	T.desc=LOWER_LEFT | 1;

	if(!TGAWRITE(&T,sizeof(TGAHeader))) {
		wrterr:
		return TGA_SAVE_WRITE_ERROR;
		}

	// Set up some buffers
	PixelBuf64 tgabuf(T.width);
	BMM_Color_64 *in = tgabuf.Ptr();
	PixelBuf16 outbuf(T.width);
	USHORT *out = outbuf.Ptr();

	// Compression buffers
	PixelBuf16 kpbuffer(T.width*2);			// Extra width just in case compression isn't efficient!
	BYTE *kpbuf = (BYTE *)kpbuffer.Ptr();

	// Now write the image!

	for(int y = T.height-1; y>=0; --y) {
		GetOutputPixels(0,y,T.width,in,UserData.preMultAlpha); 
		BMM_Color_64 *i = in;
		USHORT *o = out;
		USHORT r,g,b,a;
		if (DitherTrueColor())  {
			int e,x;
			for(int ix=0; ix<T.width; ++ix,++i,++o) {
				e = arand();
				x = (e&15)-8; e>>=4;
				r = LIM031( ((UWORD(i->r)>>8)+x) >>3 );
				x = (e&15)-8; e>>=4;	
				g = LIM031( ((UWORD(i->g)>>8)+x) >>3 );
				x = (e&15)-8; 	
				b = LIM031( ((UWORD(i->b)>>8)+x) >>3 );
				a = i->a ? 0x8000 : 0;
				*o = (r << 10) | (g << 5) | b | a;
				}
			}
		else {
			for(int ix=0; ix<T.width; ++ix,++i,++o) {
				r = (USHORT)(i->r >> 11) & 0x1f;
				g = (USHORT)(i->g >> 11) & 0x1f;
				b = (USHORT)(i->b >> 11) & 0x1f;
				a = i->a ? 0x8000 : 0;
				*o = (r << 10) | (g << 5) | b | a;
				}
			}
		if (UserData.compressed) {         /* write compressed */
			BYTE *kpptr = tga_encode16((BYTE *)out, kpbuf, T.width);
			if (!TGAWRITE(kpbuf, kpptr-kpbuf))
				goto wrterr;
			}
		else {                    /* write uncompressed */
			if(TGAWRITE(out,sizeof(USHORT)*T.width)==0)
				goto wrterr;
			}
		}

	// Write the information & footer
	if(!WriteTargaExtension(stream, TRUE, UserData.preMultAlpha))
		goto wrterr;

	return TGA_SAVE_OK;
	}

int BitmapIO_TGA::Save24BitTGA ( FILE *stream ) {
	TGAHeader T;

	// Set up & write .TGA file header

	T.idlen=0;
	T.cmtype=0;
	T.imgtype=UserData.compressed ? 10 : 2;
	T.cmorg=T.cmlen=0;
	T.cmes=0;
	T.xorg=0;
	T.yorg=0;
	T.width=saveStorage->Width();
	T.height=saveStorage->Height();
	T.pixsize=24;
	T.desc=LOWER_LEFT;

	if(!TGAWRITE(&T,sizeof(TGAHeader))) {
		wrterr:
		return TGA_SAVE_WRITE_ERROR;
		}

	// Set up some buffers
	PixelBuf64 tgabuf(T.width);
	BMM_Color_64 *in = tgabuf.Ptr();
	PixelBuf24 outbuf(T.width);
	BMM_Color_24 *out = outbuf.Ptr();

	// Compression buffers
	PixelBuf24 kpbuffer(T.width*2);			// Extra width just in case compression isn't efficient!
	BYTE *kpbuf = (BYTE *)kpbuffer.Ptr();

	// Now write the image!

	for(int y = T.height-1; y>=0; --y) {
		GetOutputPixels(0,y,T.width,in,UserData.preMultAlpha); // Get gamma-corrected pixels
		BMM_Color_64 *i = in;
		BMM_Color_24 *o = out;
		if (DitherTrueColor()) { 
			int e,x;
			for (int ix=0; ix<T.width; ix++,i++,o++) {
				e = arand()&0xff;
				x = (i->b+e)>>8;	o->r = (x>255)?255:x;
				x = (i->g+e)>>8;	o->g = (x>255)?255:x;
				x = (i->r+e)>>8;	o->b = (x>255)?255:x;
				}
			}
		else {
			for(int ix=0; ix<T.width; ++ix,++i,++o) {
				o->r = (BYTE)(i->b >> 8);		// Must swap R/B for targa file
				o->g = (BYTE)(i->g >> 8);
				o->b = (BYTE)(i->r >> 8);
				}
			}
		if (UserData.compressed) {         /* write compressed */
			BYTE *kpptr = tga_encode24((BYTE *)out, kpbuf, T.width);
			if (!TGAWRITE(kpbuf, kpptr-kpbuf))
				goto wrterr;
			}
		else {                    /* write uncompressed */
			if(TGAWRITE(out,sizeof(BMM_Color_24)*T.width)==0)
				goto wrterr;
			}
		}

	// Write the information & footer
	if(!WriteTargaExtension(stream, FALSE, UserData.preMultAlpha))
		goto wrterr;

	return TGA_SAVE_OK;
	}

int BitmapIO_TGA::Save32BitTGA ( FILE *stream ) {
	TGAHeader T;

	// Set up & write .TGA file header

	T.idlen=0;
	T.cmtype=0;
	T.imgtype=UserData.compressed ? 10 : 2;
	T.cmorg=T.cmlen=0;
	T.cmes=0;
	T.xorg=0;
	T.yorg=0;
	T.width=saveStorage->Width();
	T.height=saveStorage->Height();
	T.pixsize=32;
	T.desc=LOWER_LEFT | 8;

	if(!TGAWRITE(&T,sizeof(TGAHeader))) {
		wrterr:
		return TGA_SAVE_WRITE_ERROR;
		}

	// Set up some buffers
	PixelBuf64 tgabuf(T.width);
	BMM_Color_64 *in = tgabuf.Ptr();
	PixelBuf32 outbuf(T.width);
	BMM_Color_32 *out = outbuf.Ptr();

	// Compression buffers
	PixelBuf32 kpbuffer(T.width*2);			// Extra width just in case compression isn't efficient!
	BYTE *kpbuf = (BYTE *)kpbuffer.Ptr();

	// Now write the image!

	for(int y = T.height-1; y>=0; --y) {
		GetOutputPixels(0,y,T.width,in, UserData.preMultAlpha); // Get gamma-corrected pixels
		BMM_Color_64 *i = in;
		BMM_Color_32 *o = out;
		if (DitherTrueColor()) { 
			int e,x;
			for (int ix=0; ix<T.width; ix++,i++,o++) {
				e = arand()&0xff;
 				// Must swap R/B for targa file
				x = (i->b+e)>>8;	o->r = (x>255)?255:x;
				x = (i->g+e)>>8;	o->g = (x>255)?255:x;
				x = (i->r+e)>>8;	o->b = (x>255)?255:x;
				x = (i->a+e)>>8;	o->a = (x>255)?255:x;
				}
			}
		else {
			for(int ix=0; ix<T.width; ++ix,++i,++o) {
				// Must swap R/B for targa file
				o->r = (BYTE)(i->b >> 8);
				o->g = (BYTE)(i->g >> 8);
				o->b = (BYTE)(i->r >> 8);
				o->a = (BYTE)(i->a >> 8);
				}
			}
		if (UserData.compressed) {         /* write compressed */
			BYTE *kpptr = tga_encode32((BYTE *)out, kpbuf, T.width);
			if (!TGAWRITE(kpbuf, kpptr-kpbuf))
				goto wrterr;
			}
		else {                    /* write uncompressed */
			if(TGAWRITE(out,sizeof(BMM_Color_32)*T.width)==0)
				goto wrterr;
			}
		}

	// Write the information & footer
	if(!WriteTargaExtension(stream, TRUE, UserData.preMultAlpha))
		goto wrterr;

	return TGA_SAVE_OK;
	}

/* Write the extension area of a targa file */

int BitmapIO_TGA::WriteTargaExtension(FILE *stream, BOOL alphaUsed, BOOL preMultAlpha) {
	TGAExtra ext;
	TGAFooter foot={0,0,"TRUEVISION-XFILE."};

	ext.extsize=494;

	/* Set comments */
	for(int ix=0; ix<324; ++ix)
	ext.comments[ix]=0;		/* Blank out comments */

	CStr string;
	string = CStr(UserData.comments1);
	if(string.Length() > 80)
		string.Resize(80);
	strcpy(&ext.comments[0],string.data());
	string = CStr(UserData.comments2);
	if(string.Length() > 80)
		string.Resize(80);
	strcpy(&ext.comments[81],string.data());
	string = CStr(UserData.comments3);
	if(string.Length() > 80)
		string.Resize(80);
	strcpy(&ext.comments[162],string.data());
	string = CStr(UserData.comments4);
	if(string.Length() > 80)
		string.Resize(80);
	strcpy(&ext.comments[243],string.data());

	ext.td_month=1;
	ext.td_day=1;
	ext.td_year=1996;

	ext.td_hour=0;
	ext.td_minute=0;
	ext.td_second=0;

	string = CStr(UserData.author);
	if(string.Length() > 40)
		string.Resize(40);
	strcpy(ext.authorname,string.data());

	string = CStr(UserData.jobname);
	if(string.Length() > 40)
		string.Resize(40);
	strcpy(ext.jobname,string.data());

	ext.jt_hours=0;
	ext.jt_minutes=0;
	ext.jt_seconds=0;

#ifdef DESIGN_VER
	strcpy(ext.softwareID,"Autodesk VIZ");
#else
	strcpy(ext.softwareID,"Autodesk 3ds max");
#endif // DESIGN_VER
	ext.sw_version=100;
	ext.version_letter=' ';
	ext.key_a=ext.key_r=ext.key_g=ext.key_b=0;

	/* Figure out numerator/denominator aspect ratio */
	float asp = bi.Aspect();
	if(asp==1.0 || asp==0.0)
		{
		square_aspect:
		ext.aspect_w=ext.aspect_h=1;
		}
	else
		{
		if(asp<1.0)
			{
			ext.aspect_w=(int)(asp*10000.0);
			ext.aspect_h=10000;
			}
		else
			{
			if(aspect<10.0)
				{
				ext.aspect_w=(int)(asp*1000.0);
				ext.aspect_h=1000;
				}
			else
				goto square_aspect;
			}
		}

	/* Set gamma */
	ext.gamma_numerator=(short)(OutputGamma()*1000.0);
	ext.gamma_denominator=1000;

	ext.color_corr_table=0;
	ext.postage_stamp=0;
	ext.scan_line=0;
	ext.alpha_attributes=0;
	if (alphaUsed) 
		ext.alpha_attributes=(preMultAlpha) ? TGA_ALPHA_PREMULT: TGA_ALPHA_HASALPHA;

	foot.ext_area=ftell(stream);

	if(TGAWRITE(&ext,sizeof(TGAExtra))==0)
	return(0);
	if(TGAWRITE(&foot,sizeof(TGAFooter))==0)
	return(0);
	return(1);
	}

//-----------------------------------------------------------------------------
// #> BitmapIO_TGA::GetCfgFilename()
//

void BitmapIO_TGA::GetCfgFilename( TCHAR *filename ) {
	_tcscpy(filename,TheManager->GetDir(APP_PLUGCFG_DIR));
	int len = _tcslen(filename);
	if (len) {
		if (_tcscmp(&filename[len-1],_T("\\")))
			_tcscat(filename,_T("\\"));
		}   
	_tcscat(filename,TGACONFIGNAME);   
	}

//-----------------------------------------------------------------------------
// #> BitmapIO_TGA::ReadCfg()
//

BOOL BitmapIO_TGA::ReadCfg() {
	
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
			InitUserData(&UserData);
					
		LocalFree(buf);
	
		return (res);
	
	}
	
	return (FALSE);
}
	
//-----------------------------------------------------------------------------
// #> BitmapIO_TGA::WriteCfg()
//

void BitmapIO_TGA::WriteCfg() {
 
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

//-----------------------------------------------------------------------------
// #> BitmapIO_TGA::Close()
//

int  BitmapIO_TGA::Close( int flag ) {
	if(openMode != BMM_OPEN_W)
		return 0;
	return 1;
	}

//-- EOF: targa.cpp -----------------------------------------------------------
