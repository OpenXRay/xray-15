// File ....: dds.h
// ----------------
// Author...: Sean Palmer
// Date ....: April 2001
// Descr....: DDS File I/O Module

#ifndef _DDSCLASS_
#define _DDSCLASS_

#define DLLEXPORT __declspec(dllexport)

#define DDS_CLASS_ID Class_ID(0xe3061ca, 0xd2120df)

#pragma pack(push,1)

struct DDSCOLORKEY
{
  DWORD       dwColorSpaceLowValue;   // low boundary of color space that is to
                                      // be treated as Color Key, inclusive
  DWORD       dwColorSpaceHighValue;  // high boundary of color space that is
};

struct DDSFILEHEADER
{
  DWORD dwMagic;         // (0x20534444, or "DDS ") 

  // the following matches DDSURFACEDESC2
  DWORD               dwSize;                 // size of the DDSURFACEDESC structure
  DWORD               dwFlags;                // determines what fields are valid
  DWORD               dwHeight;               // height of surface to be created
  DWORD               dwWidth;                // width of input surface
  long                lPitch;                 // distance to start of next line (return value only)
  DWORD               dwDepth;                // the depth if this is a volume texture 
  DWORD               dwMipMapCount;          // number of mip-map levels requested
  DWORD               dwAlphaBitDepth;        // depth of alpha buffer requested
  DWORD               dwReserved;             // reserved
  void*               lpSurface;              // pointer to the associated surface memory
  DDSCOLORKEY         ddckCKDestOverlay;      // color key for destination overlay use
  DDSCOLORKEY         ddckCKDestBlt;          // color key for destination blt use
  DDSCOLORKEY         ddckCKSrcOverlay;       // color key for source overlay use
  DDSCOLORKEY         ddckCKSrcBlt;           // color key for source blt use

  // from DDPIXELFORMAT structure
  DWORD   dwPFSize;               // size of DDPIXELFORMAT structure
  DWORD   dwPFFlags;              // pixel format flags
  DWORD   dwFourCC;               // (FOURCC code)
  DWORD   dwRGBBitCount;          // how many bits per pixel
  DWORD   dwRBitMask;             // mask for red bit
  DWORD   dwGBitMask;             // mask for green bits
  DWORD   dwBBitMask;             // mask for blue bits
  DWORD   dwRGBAlphaBitMask;      // mask for alpha channel

  // from DDSCAPS2 structure
  DWORD       dwCaps;         // capabilities of surface wanted
  DWORD       dwCaps2;
  DWORD       dwCaps3;
  DWORD       dwVolumeDepth;

  DWORD               dwTextureStage;         // stage in multitexture cascade
//BYTE bData1[];         // Data for the main surface 
//[BYTE bData2[]];       // Data for attached surfaces, if any, follows. 
};

#pragma pack(pop)

enum
{
  TF_DITHER  = 0x40000000,
  TF_MIPMAPS = 0x80000000,
  TF_FMTMASK = 0x0000FFFF,
};

struct DDSParams
{
  int outFormat;   // Output format
};

class BitmapIO_DDS : public BitmapIO
{

private:

  DDSFILEHEADER  hdr;
  DDSParams     mParams;

public:

  //-- Constructors/Destructors

  BitmapIO_DDS       ();
  ~BitmapIO_DDS      ();

  //-- Number of extemsions supported

  int            ExtCount           ()       { return 1; }

  //-- Extension #n (i.e. "3DS")

  const TCHAR   *Ext                (int n) { return _T("dds"); }

  //-- Descriptions

  const TCHAR   *LongDesc           ();
  const TCHAR   *ShortDesc          ();

  //-- Miscelaneous Messages

  const TCHAR   *AuthorName         ()       { return _T("Sean Palmer");}
  const TCHAR   *CopyrightMessage   ()       { return _T("Copyright 2001 Treyarch LLC");}
  const TCHAR   *OtherMessage1      ()       { return _T("");}
  const TCHAR   *OtherMessage2      ()       { return _T("");}

  unsigned int   Version            ()       { return (100);}

  //-- Driver capabilities

  int            Capability         ()       { return BMMIO_READER    |
//                                                      BMMIO_WRITER    |
                                                      BMMIO_EXTENSION |
                                                      BMMIO_CONTROLWRITE;}

  //-- Driver Configuration

  BOOL           LoadConfigure      (void *ptr);
  BOOL           SaveConfigure      (void *ptr);
  DWORD          EvaluateConfigure  ()       { return sizeof(DDSParams); }

  //-- Show DLL's "About..." box

  void           ShowAbout          (HWND hWnd);

  //-- Show Image's control Dlg Box
  BOOL           ShowControl      (HWND hWnd, DWORD flag);

  //-- Return info about image

  BMMRES         GetImageInfo       (BitmapInfo *fbi);

  //-- Image Input

  BitmapStorage *Load               (BitmapInfo *fbi, Bitmap *map, BMMRES *status);

  //-- Image Output

  BMMRES         OpenOutput         (BitmapInfo *fbi, Bitmap *map);
  BMMRES         Write              (int frame);
  int            Close              (int flag);

  //-- This handler's specialized functions

  int            ReadDDSHeader      (FILE *stream);

  //-- Dialog Proc for the Image control Dlg box

  BOOL ConfigCtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
};

#endif _DDSCLASS_

