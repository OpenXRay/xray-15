// File ....: dds.cpp
// ------------------
// Author...: Sean Palmer
// Date ....: April 2001
// Descr....: DDS File I/O Module

//-- Include files

#include <max.h>
#include <bmmlib.h>
#include <pixelbuf.h>
#include "dds.h"
#include "ddsres.h"

#include <stdio.h>

#include "dxtlib.h"



static FILE* ghFile=NULL;

void WriteDTXnFile(DWORD count, void *buffer)
{
  fwrite(buffer, count, 1, ghFile);
}

void ReadDTXnFile(DWORD count, void *buffer)
{
  fread(buffer, count, 1, ghFile);
}



//-- Globals ------------------------------------------------------------------

HINSTANCE hInst = NULL;

TCHAR *GetString(int id)
{
  static TCHAR buf[256];
  if (hInst)
    return LoadString(hInst, id, buf, sizeof(buf)) ? buf : NULL;
  return NULL;
}

//-----------------------------------------------------------------------------
//-- File Class

class File
{
public:
  FILE  *stream;
  File  (const TCHAR *name, const TCHAR *mode) { stream = _tfopen(name,mode); }
  ~File () { Close(); }
  void Close() { if(stream) fclose(stream); stream = NULL; }
};

//-----------------------------------------------------------------------------
// DDS Class Description

class DDSClassDesc : public ClassDesc
{
public:
  int             IsPublic     ()                   { return 1;                }
  void           *Create       (BOOL loading=FALSE) { return new BitmapIO_DDS; }
  const TCHAR    *ClassName    ()                   { return GetString(IDS_DDS); }
  SClass_ID       SuperClassID ()                   { return BMM_IO_CLASS_ID;  }
  Class_ID        ClassID      ()                   { return DDS_CLASS_ID;    }
  const TCHAR    *Category     ()                   { return GetString(IDS_BITMAP_IO); }
};

static DDSClassDesc DDSDesc;

//-----------------------------------------------------------------------------
// *> AboutCtrlDlgProc()
//

BOOL CALLBACK AboutCtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
  switch (message)
  {
    case WM_INITDIALOG:
    {
      CenterWindow(hWnd,GetParent(hWnd));
      return 1;
    }

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
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

//-----------------------------------------------------------------------------
// #> BitmapIO_DDS::BitmapIO_DDS()

BitmapIO_DDS::BitmapIO_DDS()
{
  mParams.outFormat  = TF_DXT3 | TF_MIPMAPS;
}

BitmapIO_DDS::~BitmapIO_DDS () {}

//-----------------------------------------------------------------------------
// #> BitmapIO_DDS::LongDesc()

const TCHAR *BitmapIO_DDS::LongDesc()
{
  return GetString(IDS_DDS_FILE);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_DDS::ShortDesc()

const TCHAR *BitmapIO_DDS::ShortDesc()
{
  return GetString(IDS_DDS);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_DDS::LoadConfigure()

BOOL BitmapIO_DDS::LoadConfigure (void *ptr)
{
  DDSParams *buf = (DDSParams*) ptr;
  memcpy(&mParams, ptr, sizeof(DDSParams));
  return TRUE;
}

//-----------------------------------------------------------------------------
// #> BitmapIO_DDS::SaveConfigure()

BOOL BitmapIO_DDS::SaveConfigure (void *ptr)
{
  if (ptr)
  {
    memcpy(ptr, &mParams, sizeof(DDSParams));
    return TRUE;
  }
  return FALSE;
}

//-----------------------------------------------------------------------------
// #> BitmapIO_DDS::ShowAbout()

void BitmapIO_DDS::ShowAbout(HWND hWnd)
{
  DialogBoxParam(
    hInst,
    MAKEINTRESOURCE(IDD_DDS_ABOUT),
    hWnd,
    (DLGPROC)AboutCtrlDlgProc,
    (LPARAM)0);
}


//-----------------------------------------------------------------------------
// #> BitmapIO_DDS::ConfigCtrlDlgProc

BOOL BitmapIO_DDS::ConfigCtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{

  switch (message)
  {
    case WM_INITDIALOG:
      switch (mParams.outFormat & TF_FMTMASK)
      {
        case TF_DXT1:    CheckDlgButton(hWnd, IDC_DDS_DXT1,     TRUE); break;
        case TF_DXT1_1BitAlpha: CheckDlgButton(hWnd, IDC_DDS_DXT1ALPHA, TRUE); break;
        case TF_DXT3:    CheckDlgButton(hWnd, IDC_DDS_DXT3,     TRUE); break;
        case TF_DXT5:    CheckDlgButton(hWnd, IDC_DDS_DXT5,     TRUE); break;
        case TF_RGB4444: CheckDlgButton(hWnd, IDC_DDS_ARGB4444, TRUE); break;
        case TF_RGB1555: CheckDlgButton(hWnd, IDC_DDS_ARGB1555, TRUE); break;
        case TF_RGB565:  CheckDlgButton(hWnd, IDC_DDS_RGB565,   TRUE); break;
        case TF_RGB8888: CheckDlgButton(hWnd, IDC_DDS_ARGB8888, TRUE); break;
      }
      CheckDlgButton(hWnd, IDC_DDS_DITHER, (mParams.outFormat & TF_DITHER)!=0);
      CheckDlgButton(hWnd, IDC_DDS_MIPMAPS, (mParams.outFormat & TF_MIPMAPS)!=0);
      break;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDOK:
          EndDialog(hWnd,1);
          break;
        case IDCANCEL:
          EndDialog(hWnd,0);
          break;

        case IDC_DDS_DXT1:     mParams.outFormat = TF_DXT1    | (mParams.outFormat&~TF_FMTMASK); break;
        case IDC_DDS_DXT1ALPHA: mParams.outFormat = TF_DXT1_1BitAlpha | (mParams.outFormat&~TF_FMTMASK); break;
        case IDC_DDS_DXT3:     mParams.outFormat = TF_DXT3    | (mParams.outFormat&~TF_FMTMASK); break; 
        case IDC_DDS_DXT5:     mParams.outFormat = TF_DXT5    | (mParams.outFormat&~TF_FMTMASK); break;
        case IDC_DDS_ARGB4444: mParams.outFormat = TF_RGB4444 | (mParams.outFormat&~TF_FMTMASK); break;
        case IDC_DDS_ARGB1555: mParams.outFormat = TF_RGB1555 | (mParams.outFormat&~TF_FMTMASK); break;
        case IDC_DDS_RGB565:   mParams.outFormat = TF_RGB565  | (mParams.outFormat&~TF_FMTMASK); break;
        case IDC_DDS_ARGB8888: mParams.outFormat = TF_RGB8888 | (mParams.outFormat&~TF_FMTMASK); break;

        case IDC_DDS_DITHER:
          if (IsDlgButtonChecked(hWnd, IDC_DDS_DITHER))
            mParams.outFormat |= TF_DITHER;
          else
            mParams.outFormat &= ~TF_DITHER;
          break;

        case IDC_DDS_MIPMAPS:
          if (IsDlgButtonChecked(hWnd, IDC_DDS_MIPMAPS))
            mParams.outFormat |= TF_MIPMAPS;
          else
            mParams.outFormat &= ~TF_MIPMAPS;
          break;
      }
      return TRUE;
  }
  return FALSE;
}

//-----------------------------------------------------------------------------
// #> StaticDialogProc

BOOL CALLBACK StaticDialogProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l)
{
  BitmapIO_DDS* p;

  if (msg==WM_INITDIALOG)
  {
    p = (BitmapIO_DDS*)l;
    SetWindowLong(hwnd, GWL_USERDATA,l);
  }
  else
  {
    if ((p = (BitmapIO_DDS*)GetWindowLong(hwnd, GWL_USERDATA)) == NULL)
      return FALSE;
  }
  return p->ConfigCtrlDlgProc(hwnd,msg,w,l);
}


//-----------------------------------------------------------------------------
// #> BitmapIO_DDS::ShowControl()

BOOL BitmapIO_DDS::ShowControl(HWND hWnd, DWORD flag)
{
  return DialogBoxParam(
    hInst,
    MAKEINTRESOURCE (IDD_DDS_CONFIG),
    hWnd,
    (DLGPROC) StaticDialogProc,
    (LPARAM) this);
}


//-----------------------------------------------------------------------------
// #> BitmapIO_DDS::GetImageInfo()

BMMRES BitmapIO_DDS::GetImageInfo (BitmapInfo *fbi)
{
  //-- Open DDS File -----------------------------------

  File file(fbi->Name(), _T("rb"));

  if (!file.stream)
    return (ProcessImageIOError(fbi));

  //-- Read File Header --------------------------------

  if (!ReadDDSHeader(file.stream))
    return (ProcessImageIOError(fbi,BMMRES_BADFILEHEADER));

  //-- Update Bitmap Info ------------------------------

  fbi->SetWidth((WORD)hdr.dwWidth);
  fbi->SetHeight((WORD)hdr.dwHeight);

  switch (hdr.dwFourCC)
  {
    case 0:
      switch (hdr.dwRGBBitCount)
      {
        //case  1: fbi->SetType(BMM_LINE_ART); break;
        //case  4: fbi->SetType(BMM_BMP_4);    break;
        //case  8: fbi->SetType(BMM_PALETTED); break;
        case 16: 
          if (hdr.dwRGBAlphaBitMask)
            fbi->SetType(BMM_TRUE_32);  
          else
            fbi->SetType(BMM_TRUE_24);  
          break;
        case 24: fbi->SetType(BMM_TRUE_24);  break;
        case 32: fbi->SetType(BMM_TRUE_32);   break;
        default: fbi->SetType(BMM_NO_TYPE);  break;
      }
      break;
    case 'D'|('X'<<8)|('T'<<16)|('1'<<24):
    case 'D'|('X'<<8)|('T'<<16)|('2'<<24):
    case 'D'|('X'<<8)|('T'<<16)|('3'<<24):
    case 'D'|('X'<<8)|('T'<<16)|('4'<<24):
    case 'D'|('X'<<8)|('T'<<16)|('5'<<24):
      fbi->SetType(BMM_TRUE_32);   
      break;
  }

//  fbi->SetGamma(1.0f);
  fbi->SetAspect(1.0f);
  fbi->SetFirstFrame(0);
  fbi->SetLastFrame(0);

  return BMMRES_SUCCESS;
}

inline WORD Conv4To16(int d) { return (d<<12) | (d<<8) | (d<<4) | d; }
inline WORD Conv5To16(int d) { return (d<<11) | (d<<6) | (d<<1); }
inline WORD Conv6To16(int d) { return (d<<10) | (d<<4) | (d>>2); }
inline WORD Conv8To16(int d) { return (d<<8)  | d; }

//-----------------------------------------------------------------------------
//-- BitmapIO_DDS::Load()

BitmapStorage *BitmapIO_DDS::Load(BitmapInfo *fbi, Bitmap *map, BMMRES *status)
{
  RGBQUAD      *rgb = NULL;
  BMM_Color_48 *pal = NULL;
  BitmapStorage  *s = NULL;
  BMM_Color_64   *b = NULL;
  BYTE           *p = NULL;


  int pixels = 0;
  int rows   = 0;
  int w      = 0;
  int wb     = 0;
  int h      = 0;

  //-- Initialize Status Optimistically

  *status = BMMRES_SUCCESS;

  //-- Make sure nothing weird is going on

  if(openMode != BMM_NOT_OPEN)
  {
    *status = ProcessImageIOError(fbi,BMMRES_INTERNALERROR);
    return NULL;
  }

  //-- Open DDS File -----------------------------------

  File file(fbi->Name(), _T("rb"));

  if (!file.stream)
  {
    *status = ProcessImageIOError(fbi);
    return(NULL);
  }

  ghFile = file.stream;

  int width;
  int height;
  int planes;
  int lTotalWidth; 
  int rowBytes;
      
  //-- Read File Header --------------------------------

  unsigned char* data = nvDXTdecompress(width, height, planes, lTotalWidth, rowBytes);

  if (!data)
  {
    *status = ProcessImageIOError(fbi,BMMRES_BADFILEHEADER);
    return (NULL);
  }

  ghFile = NULL;
 

  //-- Update Bitmap Info ------------------------------

  fbi->SetWidth(width);
  fbi->SetHeight(height);

  //fbi->SetGamma(1.0f);
  fbi->SetAspect(1.0f);

  switch(planes)
  {
    case 3:
      fbi->SetType(BMM_TRUE_24);
      break;

    case 4:
      fbi->SetType(BMM_TRUE_32);
      break;
  }

  fbi->SetFirstFrame(0);
  fbi->SetLastFrame(0);

  //-- Create Image Storage ----------------------------

  switch(planes)
  {
    case 3:
    case 4:
      s = BMMCreateStorage(map->Manager(),BMM_TRUE_32);
      break;
  }

  if(!s)
  {
    *status = ProcessImageIOError(fbi,BMMRES_CANTSTORAGE);
    return NULL;
  }

  //-- Allocate Image Storage --------------------------

  if (s->Allocate(fbi,map->Manager(),BMM_OPEN_R)==0)
  {

    memory_error_out:
    *status = ProcessImageIOError(fbi,BMMRES_MEMORYERROR);
    goto bail_out;

    io_error_out:
    *status = ProcessImageIOError(fbi);
    bail_out:

    if (s) delete s;
    if (b) free(b);
    if (p) free(p);
    if (rgb) free(rgb);
    if (pal) free(pal);

    return NULL;

  }

  switch (planes)
  {
    case 3:
    {
      //-- Read Image (24 Bits) ----------------------------

      w    = width;
      wb   = lTotalWidth*planes; //rowBytes;
      h    = height;

      b = (BMM_Color_64  *)malloc(width*sizeof(BMM_Color_64));
      p = (BYTE          *)data;

      if(!b || !p)
        goto memory_error_out;

      BYTE *ptr;

      do
      {
        ptr = p;
        p += wb;
        for (int x = 0; x < w; x++)
        {
          b[x].b = Conv8To16(*ptr++);
          b[x].g = Conv8To16(*ptr++);
          b[x].r = Conv8To16(*ptr++);
        }
        if (s->PutPixels(0,rows,w,b)!=1)
          goto io_error_out;
        rows++;
        if (rows>=h) break;

        //-- Progress Report

        if (fbi->GetUpdateWindow())
          SendMessage(fbi->GetUpdateWindow(),BMM_PROGRESS,rows,h);

      } while (true);
      break;
    }
    case 4:
    {
      //-- Read Image (32 Bits) ----------------------------

      w    = width;
      wb   = lTotalWidth*planes; //rowBytes;
      h    = height;

      b = (BMM_Color_64  *)malloc(width*sizeof(BMM_Color_64));
      p = (BYTE          *)data;

      if(!b || !p)
        goto memory_error_out;

      BYTE *ptr;

      bool foundalpha = false;
 
      do
      {
        ptr = p;
        p += wb;
        for (int x = 0; x < w; x++)
        {
          b[x].b = Conv8To16(*ptr++);
          b[x].g = Conv8To16(*ptr++);
          b[x].r = Conv8To16(*ptr++);
          if (*ptr != 0xFF) foundalpha=true;
          b[x].a = Conv8To16(*ptr++);
        }
        if (s->PutPixels(0,rows,w,b)!=1)
          goto io_error_out;
        rows++;
        if (rows>=h) break;

        //-- Progress Report

        if (fbi->GetUpdateWindow())
          SendMessage(fbi->GetUpdateWindow(),BMM_PROGRESS,rows,h);

      } while (true);
      if (foundalpha)
  	    fbi->SetFlags(MAP_HAS_ALPHA);
      else
        fbi->SetType(BMM_TRUE_24);
      break;
    }
  }

  //-- Clean Up ----------------------------------------

  free(data);

  if (b) free(b);

  //-- Set the storage's BitmapInfo

  s->bi.CopyImageInfo(fbi);

  return  s;

}

//-----------------------------------------------------------------------------
// #> BitmapIO_DDS::OpenOutput()
//

BMMRES BitmapIO_DDS::OpenOutput(BitmapInfo *fbi, Bitmap *map)
{

  if (openMode != BMM_NOT_OPEN)
    return (ProcessImageIOError(&bi,BMMRES_INTERNALERROR));

  if (!map)
    return (ProcessImageIOError(&bi,BMMRES_INTERNALERROR));

  //-- Save Image Info Data

  bi.CopyImageInfo(fbi);
  bi.SetUpdateWindow(fbi->GetUpdateWindow());

  this->map = map;
  openMode    = BMM_OPEN_W;

  return (BMMRES_SUCCESS);

}

//-----------------------------------------------------------------------------
// #> BitmapIO_DDS::Write()
//
//

BMMRES BitmapIO_DDS::Write(int frame)
{

  BMMRES result = BMMRES_SUCCESS;

  //-- If we haven't gone through an OpenOutput(), leave

  if (openMode != BMM_OPEN_W)
    return ProcessImageIOError(&bi,BMMRES_INTERNALERROR);

  //-- Resolve Filename --------------------------------

  TCHAR filename[MAX_PATH];

  if (frame == BMM_SINGLEFRAME)
  {
    _tcscpy(filename,bi.Name());
  }
  else
  {
    if (!BMMCreateNumberedFilename(bi.Name(),frame,filename))
      return ProcessImageIOError(&bi,BMMRES_NUMBEREDFILENAMEERROR);
  }

  //-- Create Image File -------------------------------

  PBITMAPINFO pbmi = GetDitheredOutputDib(32);
  if (!pbmi)
    return ProcessImageIOError(&bi,GetString(IDS_CONVERT_ERROR));
  int w=map->Width();
  int h=map->Height();
  int wb=w*4;
  unsigned char* data = new unsigned char[wb*h];
  const unsigned char* psrc = (const unsigned char*)pbmi->bmiColors;
  unsigned char* pdst = data + wb*h;
  for (int j=h; --j>=0; )
  {
    pdst -= wb;
    memcpy(pdst,psrc,wb);
    psrc += wb;
  }

  //-- Write Image File --------------------------------

  if (bi.GetUpdateWindow())
    SendMessage(bi.GetUpdateWindow(),BMM_PROGRESS,50,100);

  File file(filename, _T("wb"));

  if (!file.stream)
  {
    LocalFree(pbmi);
    return ProcessImageIOError(&bi);
  }

  ghFile = file.stream;
  
/*
   Compresses an image with a user supplied callback with the data for each MIP level created
   Only supports input of RGB 24 or ARGB 32 bpp
   if callback is == 0 (or not specified), then WriteDTXnFile is called with all file info
*/
  HRESULT res = nvDXTcompress(data, // pointer to data (24 or 32 bit)
                  w, // width in texels
                  h, // height in texels
                  mParams.outFormat&TF_FMTMASK, 
                  (mParams.outFormat&TF_MIPMAPS)!=0,    // auto gen MIP maps
                  (mParams.outFormat&TF_DITHER)!=0,    // dither
                  4,  // 3 or 4 bytes per pixel
                  0);   // callback for generated levels

  delete[] data;

  LocalFree(pbmi);
  ghFile = NULL;

  if (res != 0)
  {
    switch (res)
    {
      default:
      case DXTERR_INPUT_POINTER_ZERO:
        return ProcessImageIOError(&bi,GetString(IDS_CONVERT_ERROR));
      case DXTERR_DEPTH_IS_NOT_3_OR_4:
        return ProcessImageIOError(&bi,GetString(IDS_UNSUPPORTEDFORMAT_ERROR));
      case DXTERR_NON_POWER_2:
        return ProcessImageIOError(&bi,GetString(IDS_NONPOWEROF2_ERROR));
    }
  }

  if (bi.GetUpdateWindow())
    SendMessage(bi.GetUpdateWindow(),BMM_PROGRESS,100,100);

  return result;
}

//-----------------------------------------------------------------------------
// BitmapIO_DDS::Close()
//

int  BitmapIO_DDS::Close(int flag)
{
  return 1;
}

//-----------------------------------------------------------------------------
// #> BitmapIO_DDS::ReadDDSHeader()
//

int  BitmapIO_DDS::ReadDDSHeader(FILE *stream)
{
  //-- Read File Header --------------------------------

  int res = fread(&hdr,1,sizeof(DDSFILEHEADER),stream);

  if (res != sizeof(DDSFILEHEADER))
    return 0;

  //-- Validate ----------------------------------------

  if (hdr.dwMagic != 0x20534444)
    return 0;

  //-- Done
  return 1;
}


//-----------------------------------------------------------------------------
// Interface

DLLEXPORT const TCHAR * LibDescription ()
{
  return GetString(IDS_DDS_DESC);
}

DLLEXPORT int LibNumberClasses ()
{
  return 1;
}

DLLEXPORT ClassDesc *LibClassDesc(int i)
{
  switch(i)
  {
    case  0: return &DDSDesc; break;
    default: return 0;        break;
  }
}

DLLEXPORT ULONG LibVersion ()
{
  return (VERSION_3DSMAX);
}

// Let the plug-in register itself for deferred loading
__declspec(dllexport) ULONG CanAutoDefer()
{
  return 1;
}

//-----------------------------------------------------------------------------
//-- DLL Declaration

BOOL WINAPI DllMain(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID lpvReserved)
{
  switch (fdwReason)
  {
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

