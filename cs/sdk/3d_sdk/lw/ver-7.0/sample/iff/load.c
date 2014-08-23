/*
======================================================================
load.c

Ernie Wright  22 Mar 00

Functions for reading IFF ILBMs (IFF interleaved bitmaps).

PUBLIC FUNCTIONS
   ReadILBM()    open, parse an ILBM, prepare to decode pixel data
   GetRowILBM()  decode one scanline from an ILBM

SUPPORTED IMAGE TYPES
   Encoding          Depth (planes)
   --------------------------------
    color map         1 to 8
    gray level        1 to 8
    RGB level         24
    Hold and Modify   5 to 8
    Extra Halfbrite   6
====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iff.h"


/*
======================================================================
unHAM()

Decodes one CAMG_HAM scanline from an image file.

INPUTS
   pic   pointer to a PicInfo initialized by ReadILBM() and pre-
         processed for the current scanline by getrow_indexed()

RESULTS
   The red, green and blue scanline buffers are filled with byte-per-
   pixel channel data.

HAM (hold-and-modify) images store pixel colors as codes which are
divided into a mode in the high two bits and data in the other bits:

   00  data bits are an index into the color map
   01  data bits are blue level
   10  data bits are red level
   11  data bits are green level

Unless a pixel is color-mapped, only one of its three levels is given
in its code.  The other two are assumed to be the same as those for
the pixel to its left.  If the pixel is the first one in a scanline,
the pixel to its left is assumed to be RGB(0, 0, 0).

The number of data bits is 4 for standard HAM and 6 for HAM8.  The
data bits are precision-extended when the levels are moved to the
8-bit RGB buffers--regardless of the number of bits, the maximum
level will translate to 255 at 8 bits of precision.

It is possible for the mode to be a single bit.  In this case the
low bit is explicit and the high bit is assumed to be 0, implying
that only the blue level can be modified.  This is very seldom, if
ever, used, but is supported here because the cost is negligible
(which is probably why the Amiga display hardware supported it).
====================================================================== */

static void unHAM( PicInfo *pic )
{
   RGBTriple prev;
   int i, j, hbits, mbits, mask;


   prev.red = prev.green = prev.blue = 0;
   hbits = pic->bmhd.nPlanes > 6 ? 6 : 4;
   mbits = 8 - hbits;
   mask  = ( 1 << hbits ) - 1;

   for ( i = 0; i < pic->bmhd.w; i++ ) {
      j = pic->buf[ i ];
      switch ( j >> hbits ) {
         case 0:
            pic->rgb[ 0 ][ i ] = pic->cmap[ j & mask ].red;
            pic->rgb[ 1 ][ i ] = pic->cmap[ j & mask ].green;
            pic->rgb[ 2 ][ i ] = pic->cmap[ j & mask ].blue;
            break;

         case 1:
            pic->rgb[ 0 ][ i ] = prev.red;
            pic->rgb[ 1 ][ i ] = prev.green;
            pic->rgb[ 2 ][ i ] = ( j & mask ) << mbits;
            pic->rgb[ 2 ][ i ] |= pic->rgb[ 2 ][ i ] >> hbits;
            break;

         case 2:
            pic->rgb[ 0 ][ i ] = ( j & mask ) << mbits;
            pic->rgb[ 0 ][ i ] |= pic->rgb[ 0 ][ i ] >> hbits;
            pic->rgb[ 1 ][ i ] = prev.green;
            pic->rgb[ 2 ][ i ] = prev.blue;
            break;

         case 3:
            pic->rgb[ 0 ][ i ] = prev.red;
            pic->rgb[ 1 ][ i ] = ( j & mask ) << mbits;
            pic->rgb[ 1 ][ i ] |= pic->rgb[ 1 ][ i ] >> hbits;
            pic->rgb[ 2 ][ i ] = prev.blue;
            break;
      }
      prev.red   = pic->rgb[ 0 ][ i ];
      prev.green = pic->rgb[ 1 ][ i ];
      prev.blue  = pic->rgb[ 2 ][ i ];
   }
}


/*
======================================================================
getrow_indexed()

Read one scanline from a color-mapped image file.

INPUTS
   pic   pointer to a PicInfo initialized by ReadILBM()

RESULTS
   If successful, the red, green and blue scanline buffers are filled
   with byte-per-pixel channel data and the function returns TRUE.
   Otherwise it returns FALSE.

IFF ILBMs store images with 256 or fewer colors as arrays of color
map indexes.  The color map index for a given pixel is developed by
collecting each bit at a given position from each bitplane.  The
index is then used to look up the pixel's RGB levels in a table,
which is stored in ILBMs as a CMAP chunk.  Bit collection is as
described for getrow_24(), except that only one byte array results,
rather than three.

If the image is HAM (hold-and-modify), the numbers in the index array
are actually codes that may contain either color map references or
RGB levels.  See the comments for the unHAM() function.  If the image
is EHB (extra-halfbrite), the color map in the file contains 32
entries, but the indexes range from 0 to 63.
====================================================================== */

static int getrow_indexed( PicInfo *pic )
{
   unsigned char *b;
   int j, k, p;


   /* read scanline bytes */

   memset( pic->buf, 0, pic->rowsize * 8 );

   switch ( pic->bmhd.compression ) {
      case cmpByteRun1:
         b = pic->buf;
         for ( p = 0; p < pic->bmhd.nPlanes; p++ ) {
            if ( !unpack( &pic->bp, b, pic->rowsize )) return FALSE;
            b += pic->rowsize;
         }
         break;

      case cmpNone:
         memcpy( pic->buf, pic->bp, pic->rowsize * pic->bmhd.nPlanes );
         pic->bp += pic->rowsize * pic->bmhd.nPlanes;
         break;

      default:
         return FALSE;
   }

   /* Collect bits from pic->bmhd.nPlanes bitplanes into a single byte.
      Bits form indexes into the colormap (or HAM or EHB codes), which
      we'll store temporarily in the blue buffer and copy back to the
      buf buffer. */

   k = 0;
   b = pic->buf;
   for ( j = 0; j < pic->rowsize; j++ ) {
      bitrot_cw( b, pic->rowsize, pic->rgb[ 2 ] + k, 1 );
      k += 8;
      ++b;
   }
   memcpy( pic->buf, pic->rgb[ 2 ], pic->bmhd.w );

   /* convert from indexes in buf to 24-bit RGB */

   if ( pic->camg & CAMG_HAM )
      unHAM( pic );

   else
      /* fill rgb buffers with values from the colormap */

      for ( j = 0; j < pic->bmhd.w; j++ ) {
         k = pic->buf[ j ];
         pic->rgb[ 0 ][ j ] = pic->cmap[ k ].red;
         pic->rgb[ 1 ][ j ] = pic->cmap[ k ].green;
         pic->rgb[ 2 ][ j ] = pic->cmap[ k ].blue;
      }

   return TRUE;
}


/*
======================================================================
getrow_24()

Read one scanline from a 24-bit image file.

INPUTS
   pic   pointer to a PicInfo initialized by ReadILBM()

RESULTS
   If successful, the red, green and blue scanline buffers are filled
   with byte-per-pixel channel data and the function returns TRUE.
   Otherwise it returns FALSE.

IFF24s store 24-bit images as 24 bit-per-pixel planes.  The color of
a given pixel is developed by collecting each bit at a given position
from each of the 24 planes, like pushing a pin through a stack of 24
sheets of paper.  getrow_24() regards the bit collection process as a
90 degree rotation of three width x 8 bitmaps.  The rotated 8 x width
bitmaps contain, from top to bottom, the bytes that form byte-per-
pixel scanline channels.
====================================================================== */

static int getrow_24( PicInfo *pic )
{
   unsigned char *b;
   int i, j, k, p;


   /* for each RGB channel */

   for ( i = 0; i < 3; i++ ) {

      /* read scanline bytes */

      switch ( pic->bmhd.compression ) {
         case cmpByteRun1:
            b = pic->buf;
            for ( p = 0; p < 8; p++ ) {
               if ( !unpack( &pic->bp, b, pic->rowsize )) return FALSE;
               b += pic->rowsize;
            }
            break;

         case cmpNone:
            memcpy( pic->buf, pic->bp, pic->rowsize * 8 );
            pic->bp += pic->rowsize * 8;
            break;

         default:
            return FALSE;
      }

      /* collect bits from eight bitplanes into a single byte */

      k = 0;
      b = pic->buf;
      for ( j = 0; j < pic->rowsize; j++ ) {
         bitrot_cw( b, pic->rowsize, pic->rgb[ i ] + k, 1 );
         k += 8;
         ++b;
      }
   }

   return TRUE;
}


/*
======================================================================
GetRowILBM()

Extract one scanline from a PicInfo.

INPUTS
   pic   pointer to a PicInfo initialized by ReadILBM()

RESULTS
   If successful, the red, green and blue scanline buffers are filled
   with byte-per-pixel channel data and the function returns TRUE.
   Otherwise it returns FALSE.

If the ILBM isn't 24-bit, the original index data is preserved in
pic->buf in chunky form.

Once an IFF ILBM has been opened and parsed by ReadILBM(), this
routine may be called to read the image one scanline at a time.  On
successive calls, scanlines are delivered in order from the top of
the image to the bottom.  The number of scanlines (the image's pixel
height) is available in pic->bmhd.h.  The number of bytes per scan-
line channel (the image width) is in pic->bmhd.w.

If masking is of the mskHasMask type, an extra bitplane representing
the mask is interleaved with the pixel data in the file.  This data
is skipped here.
====================================================================== */

int GetRowILBM( PicInfo *pic )
{
   int ok;

   if ( pic->bmhd.nPlanes == 24 )
      ok = getrow_24( pic );
   else if ( pic->bmhd.nPlanes <= 8 )
      ok = getrow_indexed( pic );
   else
      ok = FALSE;

   /* skip the mask plane */

   if ( ok && pic->bmhd.masking == mskHasMask )
      if ( pic->bmhd.compression == cmpByteRun1 )
         ok = unpack_ro( &pic->bp, pic->rowsize );
      else {
         pic->bp += pic->rowsize;
         ok = TRUE;
      }

   return ok;
}


/*
======================================================================
ReadILBM()

Read an image file and create a PicInfo describing it.

INPUTS
   filename    a pathspec for an IFF ILBM file
   result      pointer to storage for an error code

RESULTS
   If successful, this function returns a pointer to a full PicInfo,
   basically a memory copy of the image, and the result value will be
   0.  Otherwise NULL is returned and result is set to an error code.

ERROR CONDITIONS
   Memory allocation failed
   File open, read or seek failed
   File is a mangled IFF
   File isn't an IFF
   File isn't an ILBM
   Image isn't a recognized depth (1-8 or 24)
   BMHD chunk not found
   BODY chunk not found
====================================================================== */

PicInfo *ReadILBM( const char *filename, int *result )
{
   long ck[ 3 ];
   int i;
   PicInfo *pic;


   /* allocate a PicInfo */

   *result = EPNOMEM;
   pic = ( PicInfo * ) calloc( sizeof( PicInfo ), 1 );
   if ( !pic ) return NULL;

   /* cache the filename */

   pic->filename = malloc( strlen( filename ) + 1 );
   if ( !pic->filename ) return FreeILBM( pic );
   strcpy( pic->filename, filename );

   /* open the image file */

   *result = EPNOFILE;
   pic->fp = fopen( filename, "rb" );
   if ( !pic->fp ) return FreeILBM( pic );

   /* read the FORM header */

   *result = EPNOILBM;
   fread( ck, 12, 1, pic->fp );

   /* FORM ILBM? */

   if ( ck[ 0 ] != ID_FORM || ck[ 2 ] != ID_ILBM ) return FreeILBM( pic );

   /* remember the FORM size */

   ck[ 2 ] = ck[ 1 ];
   revbytes( &ck[ 2 ], 4, 1 );

   /* get BMHD, CAMG, CMAP and stop on BODY */

   fread( ck, 8, 1, pic->fp );
   revbytes( &ck[ 1 ], 4, 1 );

   while ( 1 ) {
      ck[ 1 ] += ( ck[ 1 ] & 1 );
      if ( ck[ 0 ] == ID_BODY ) break;

      if ( ck[ 0 ] == ID_BMHD ) {
         fread( &pic->bmhd, ck[ 1 ], 1, pic->fp );
         revbytes( &pic->bmhd.w, 2, 4 );
         revbytes( &pic->bmhd.transparentColor, 2, 1 );
         revbytes( &pic->bmhd.pw, 2, 2 );
      }

      else if ( ck[ 0 ] == ID_CMAP ) {
         *result = EPNOMEM;
         pic->cmap = malloc( ck[ 1 ] );
         if ( !pic->cmap ) return FreeILBM( pic );
         pic->ncolors = ck[ 1 ] / 3;
         fread( pic->cmap, ck[ 1 ], 1, pic->fp );
      }

      else if ( ck[ 0 ] == ID_CAMG ) {
         fread( &pic->camg, ck[ 1 ], 1, pic->fp );
         revbytes( &pic->camg, ck[ 1 ], 1 );
      }

      else
         fseek( pic->fp, ck[ 1 ], SEEK_CUR );

      if ( ck[ 2 ] < ftell( pic->fp ))   /* past the end of the FORM? */
         break;

      fread( ck, 8, 1, pic->fp );
      revbytes( &ck[ 1 ], 4, 1 );
   }

   /* found a BODY? */

   *result = EPNOBODY;
   if ( ck[ 0 ] != ID_BODY ) return FreeILBM( pic );

   /* found a BMHD? */

   *result = EPNOBMHD;
   if ( pic->bmhd.w == 0 ) return FreeILBM( pic );

   /* number of planes makes sense? */

   *result = EPPLANES;
   if ( pic->bmhd.nPlanes > 8 && pic->bmhd.nPlanes != 24 )
      return FreeILBM( pic );

   /* if no CMAP, assume gray and create our own color table */

   if ( pic->bmhd.nPlanes <= 8 && !pic->cmap ) {
      pic->isgray = TRUE;
      pic->ncolors = 1 << pic->bmhd.nPlanes;
      *result = EPNOMEM;
      pic->cmap = calloc( pic->ncolors, sizeof( RGBTriple ));
      if ( !pic->cmap ) return FreeILBM( pic );
      for ( i = 0; i < pic->ncolors; i++ )
         pic->cmap[ i ].red = pic->cmap[ i ].green = pic->cmap[ i ].blue
            = i * 255 / pic->ncolors;
   }

   /* if EHB, extend the color table */

   if ( pic->camg & CAMG_EHB ) {
      pic->cmap = realloc( pic->cmap, pic->ncolors * 6 );
      if ( !pic->cmap )
         return FreeILBM( pic );
      for ( i = pic->ncolors; i < pic->ncolors * 2; i++ ) {
         pic->cmap[ i ].red   = pic->cmap[ i - pic->ncolors ].red   >> 1;
         pic->cmap[ i ].green = pic->cmap[ i - pic->ncolors ].green >> 1;
         pic->cmap[ i ].blue  = pic->cmap[ i - pic->ncolors ].blue  >> 1;
      }
      pic->ncolors *= 2;
   }

   /* allocate scanline buffers */

   *result = EPNOMEM;
   pic->rowsize = (( pic->bmhd.w + 15 ) >> 3 ) & 0xFFFE;
   pic->buf = calloc( pic->rowsize, 32 );
   pic->body = pic->bp = malloc( ck[ 1 ] );
   if ( !pic->buf || !pic->body ) return FreeILBM( pic );

   pic->rgb[ 0 ] = pic->buf + pic->rowsize * 8;
   pic->rgb[ 1 ] = pic->buf + pic->rowsize * 16;
   pic->rgb[ 2 ] = pic->buf + pic->rowsize * 24;

   /* read the body */

   fread( pic->body, ck[ 1 ], 1, pic->fp );

   /* done with the file */

   fclose( pic->fp );
   pic->fp = NULL;

   /* successful */

   *result = 0;
   return pic;
}
