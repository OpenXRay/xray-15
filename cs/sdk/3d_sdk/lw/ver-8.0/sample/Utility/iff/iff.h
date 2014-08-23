/*
=====================================================================
iff.h

Ernie Wright  22 Mar 00

Definitions and function prototypes for the IFF loader/saver plug-in.
===================================================================== */

#ifndef IFF_H
#define IFF_H

#include <stdio.h>

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif


/* IFF IDs */

#ifdef _WIN32
#define MAKE_ID(a,b,c,d) ((a)|((b)<<8)|((c)<<16)|((d)<<24))
#else
#define MAKE_ID(a,b,c,d) (((a)<<24)|((b)<<16)|((c)<<8)|(d))
#endif

#define ID_FORM MAKE_ID('F','O','R','M')
#define ID_ILBM MAKE_ID('I','L','B','M')
#define ID_BMHD MAKE_ID('B','M','H','D')
#define ID_CMAP MAKE_ID('C','M','A','P')
#define ID_CAMG MAKE_ID('C','A','M','G')
#define ID_BODY MAKE_ID('B','O','D','Y')


/* ReadILBM() error codes */

#define EP       100
#define EPNOMEM  (EP + 0)              // Memory allocation failed.
#define EPNOFILE (EP + 1)              // Attempt to open file failed.
#define EPNOILBM (EP + 2)              // Not an IFF ILBM.
#define EPNOBMHD (EP + 3)              // BMHD chunk not found.
#define EPNOBODY (EP + 4)              // BODY chunk not found.
#define EPPLANES (EP + 5)              // More than 8 planes and not 24.


/* the ILBM BitmapHeader structure */

typedef struct {
   unsigned short w, h;                //  image width, height in pixels
   short          x, y;                //  image position in destination
   unsigned char  nPlanes;             //  number of bitplanes (depth)
   unsigned char  masking;             //  mask type
   unsigned char  compression;         //  compression algorithm
   unsigned char  pad1;                //  pad char for alignment
   unsigned short transparentColor;    //  a logical color number
   unsigned char  xAspect, yAspect;    //  pixel ratio of width : height
   short          pw, ph;              //  source page size
} BMHD;

#define mskNone     0
#define mskHasMask  1
#define mskHasTransparentColor 2
#define mskLasso    3

#define cmpNone     0
#define cmpByteRun1 1


/* special Commodore-Amiga display modes */

#define CAMG_HAM 0x0800
#define CAMG_EHB 0x0080


/* entries in the CMAP color table */

typedef struct { unsigned char red, green, blue; } RGBTriple;


/* an ILBM instance */

typedef struct {
   char          *filename;            // name of the file
   FILE          *fp;                  // file pointer
   unsigned char *rgb[ 3 ];            // 24-bit scanline buffer (3 channels)
   unsigned char *buf;                 // input buffer
   BMHD           bmhd;                // ILBM bitmap header
   unsigned long  camg;                // Amiga display mode
   int            isgray;              // TRUE if grayscale
   int            ncolors;             // number of colors in color table
   RGBTriple     *cmap;                // color table
   unsigned char *body;                // BODY bytes
   unsigned char *bp;                  // pointer into body
   int            rowsize;             // bytes per bitplane row
   int            result;              // for tracking save errors
   unsigned long  BODYpos;             // remember where we parked
} PicInfo;


/* function prototypes */

PicInfo *ReadILBM( const char *filename, int *result );
int GetRowILBM( PicInfo *pic );

int InitILBM( PicInfo *pic );
int PutRowILBM( PicInfo *pic );
int CloseILBM( PicInfo *pic );

PicInfo *FreeILBM( PicInfo *pic );

#ifdef _WIN32
void revbytes( void *bp, int elsize, int elcount );
#else
#define revbytes( b, s, c )
#endif

void find_aspect( float a, int *ax, int *ay );
int unpack( unsigned char **psrc, unsigned char *dst, int rowsize );
int unpack_ro( unsigned char **psrc, int rowsize );
int pack( unsigned char *src, unsigned char *dst, int rowsize );
void bitrot_cw( unsigned char *src, int srcstep, unsigned char *dst, int dststep );
void bitrot_ccw( unsigned char *src, int srcstep, unsigned char *dst, int dststep );


#endif
