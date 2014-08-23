/*
======================================================================
util.c

Ernie Wright  22 Mar 00

Utility routines for the LightWave IFF-ILBM loader/saver sample.
====================================================================== */

#include "iff.h"
#include <stdlib.h>
#include <math.h>


/*
=====================================================================
FreeILBM()

Free a PicInfo.

All PicInfos should be sent here at the end of their lives.  This
routine is written so that the resources tied to an incompletely
constructed PicInfo can be freed.  NULL is always echoed to the
caller.
===================================================================== */

PicInfo *FreeILBM( PicInfo *pic )
{
   if ( pic ) {
      if ( pic->fp ) fclose( pic->fp );
      if ( pic->filename ) free( pic->filename );
      if ( pic->cmap ) free( pic->cmap );
      if ( pic->buf ) free( pic->buf );
      if ( pic->body ) free( pic->body );
      free( pic );
   }
   return NULL;
}


/*
=====================================================================
find_aspect()

Search for the best numerator-denominator pair to express the aspect.

INPUTS
   a        floating-point aspect
   ax       storage for the numerator
   ay       storage for the denominator

RESULTS
   Finds the ratio of integers < 256 closest to the floating-point
   aspect and sets ax and ay.

The ILBM bitmap header expresses the pixel aspect as the ratio of two
unsigned chars.  We're looking for the best two here.
===================================================================== */

void find_aspect( float a, int *ax, int *ay )
{
   float d, dmin = 1e6f;
   int x, y;

   *ax = 1;
   *ay = 1;
   if ( a == 1.0f ) return;

   for ( y = 2; y < 256; y++ ) {
      x = ( int )( a * y );
      if ( x > 255 ) break;

      d = ( float ) fabs( a - ( float ) x / y );
      if ( d < dmin ) {
         dmin = d;
         *ax = x;
         *ay = y;
      }
      if ( d == 0.0f ) break;
   }
}


#ifdef _WIN32
/*
=====================================================================
revbytes()

Reverses byte order in place.

INPUTS
   bp       bytes to reverse
   elsize   size of the underlying data type
   elcount  number of elements to swap

RESULTS
   Reverses the byte order in each of elcount elements.

IFF ILBMs use a byte order variously called "big-endian", "Motorola"
or "Internet order".  So do most systems.  The big exception is
Windows, which is where the ILBM code needs this function.  The
iff.h header substitutes an empty macro on other systems.
===================================================================== */

void revbytes( void *bp, int elsize, int elcount )
{
   register unsigned char *p, *q;

   p = ( unsigned char * ) bp;

   if ( elsize == 2 ) {
      q = p + 1;
      while ( elcount-- ) {
         *p ^= *q;
         *q ^= *p;
         *p ^= *q;
         p += 2;
         q += 2;
      }
      return;
   }

   while ( elcount-- ) {
      q = p + elsize - 1;
      while ( p < q ) {
         *p ^= *q;
         *q ^= *p;
         *p ^= *q;
         ++p;
         --q;
      }
      p += elsize >> 1;
   }
}
#endif


/*
=====================================================================
unpack()

Decompress a run-length encoded bitplane row.

INPUTS
   src      pointer to an array of encoded bytes
   dst      array of at least rowsize bytes to receive decoded data
   rowsize  number of bytes in an uncompressed row

RESULTS
   If successful, the decoded row is in dst, the src pointer is moved
   to the end of the encoded array, and the function returns TRUE.  If
   an error occurs, FALSE is returned.

The pixel data for an IFF ILBM is a byte stream containing code bytes
followed by data bytes.  The meaning of the code bytes, when regarded
as unsigned, is:

   n < 128     copy the next n+1 bytes
   n > 128     repeat the next byte 257-n times
   n = 128     no-op

Some versions of Photoshop incorrectly use the n = 128 no-op as a
repeat code, which breaks strictly conforming readers.  We allow the
use of n = 128 as a repeat.  This is pretty safe, since no one to my
knowledge explicitly writes no-ops into their ILBMs.  The reason
n = 128 is a no-op is historical:  the Mac Packbits buffer was only
128 bytes, and a repeat code of 128 generates 129 bytes.
===================================================================== */

int unpack( unsigned char **psrc, unsigned char *dst, int rowsize )
{
   int c, n;
   unsigned char *src = *psrc;

   while ( rowsize > 0 ) {
      n = *src++;

      if ( n < 128 ) {
         ++n;
         rowsize -= n;
         if ( rowsize < 0 ) return FALSE;
         while ( n-- ) *dst++ = *src++;
      }
      else {
         n = 257 - n;
         rowsize -= n;
         if ( rowsize < 0 ) return FALSE;
         c = *src++;
         while ( n-- ) *dst++ = c;
      }
   }
   *psrc = src;
   return TRUE;
}


/*
=====================================================================
unpack_ro()

Pretend to decompress a run-length encoded bitplane row, so that you
can move the source pointer.  Like unpack(), but doesn't write.
===================================================================== */

int unpack_ro( unsigned char **psrc, int rowsize )
{
   int n;
   unsigned char *src = *psrc;

   while ( rowsize > 0 ) {
      n = *src++;

      if ( n < 128 ) {
         ++n;
         rowsize -= n;
         if ( rowsize < 0 ) return FALSE;
         src += n;
      }
      else {
         n = 257 - n;
         rowsize -= n;
         if ( rowsize < 0 ) return FALSE;
         ++src;
      }
   }

   *psrc = src;

   return TRUE;
}


/*
=====================================================================
pack()

Run-length encode a bitplane row.

INPUTS
   src      pointer to an array bytes
   dst      array of at least rowsize bytes to receive encoded data
   rowsize  number of bytes in an uncompressed row

RESULTS
   Puts the encoded row in dst and returns the compressed length.

See unpack() for a description of the encoding.
===================================================================== */

#define DUMP    0
#define RUN     1
#define MINRUN  3
#define MAXRUN  128
#define MAXDUMP 128

int pack( unsigned char *src, unsigned char *dst, int rowsize )
{
   char c, lastc;
   int
      mode = DUMP,
      rstart = 0,
      putsize = 0,
      sp = 1,
      i;


   lastc = *src;
   rowsize--;

   while ( rowsize > 0 ) {
      c = *( src + sp );
      sp++;
      rowsize--;

      switch ( mode ) {
         case DUMP:
            if ( sp > MAXDUMP ) {
               *dst++ = sp - 2;
               for ( i = 0; i < sp - 1; i++ )
                  *dst++ = *( src + i );
               putsize += sp;
               src += sp - 1;
               sp = 1;
               rstart = 0;
               break;
            }

            if ( c == lastc ) {
               if (( sp - rstart ) >= MINRUN ) {
                  if ( rstart > 0 ) {
                     *dst++ = rstart - 1;
                     for ( i = 0; i < rstart; i++ )
                        *dst++ = *( src + i );
                     putsize += rstart + 1;
                  }
                  mode = RUN;
               }
               else if ( rstart == 0 ) mode = RUN;
            }
            else rstart = sp - 1;
            break;

         case RUN:
            if (( c != lastc ) || ( sp - rstart > MAXRUN )) {
               *dst++ = rstart + 2 - sp;
               *dst++ = lastc;
               putsize += 2;
               src += sp - 1;
               sp = 1;
               rstart = 0;
               mode = DUMP;
            }
      }
      lastc = c;
   }

   switch ( mode ) {
      case DUMP:
         *dst++ = sp - 1;
         for ( i = 0; i < sp; i++ )
            *dst++ = *( src + i );
         putsize += sp + 1;
         break;

      case RUN:
         *dst++ = rstart + 1 - sp;
         *dst   = lastc;
         putsize += 2;
   }

   return ( putsize );
}


/*
=====================================================================
bit rotation

Fast 90-degree bit rotation routines.

INPUTS
   src       starting address of 8 x 8 bit source tile
   srcstep   byte offset between adjacent rows in source
   dst       starting address of 8 x 8 bit destination tile
   dststep   byte offset between adjacent rows in destination

RESULTS
   Bits from the source are rotated 90 degrees either clockwise or
   counterclockwise and written to the destination.

Based on Sue-Ken Yap, "A Fast 90-Degree Bitmap Rotator," in GRAPHICS
GEMS II, James Arvo ed., Academic Press, 1991, ISBN 0-12-064480-0.
===================================================================== */

#define rtable( name, n ) \
   static unsigned long name[ 16 ] = { \
      0x00000000ul << n, 0x00000001ul << n, 0x00000100ul << n, 0x00000101ul << n, \
      0x00010000ul << n, 0x00010001ul << n, 0x00010100ul << n, 0x00010101ul << n, \
      0x01000000ul << n, 0x01000001ul << n, 0x01000100ul << n, 0x01000101ul << n, \
      0x01010000ul << n, 0x01010001ul << n, 0x01010100ul << n, 0x01010101ul << n };

rtable( rtab0, 0 )
rtable( rtab1, 1 )
rtable( rtab2, 2 )
rtable( rtab3, 3 )
rtable( rtab4, 4 )
rtable( rtab5, 5 )
rtable( rtab6, 6 )
rtable( rtab7, 7 )

#define extract( d, t ) \
   lonyb = *d & 0xF; hinyb = *d >> 4; \
   lo |= t[ lonyb ]; hi |= t[ hinyb ]; d += pstep;


/*
=====================================================================
bitrot_cw()

Rotate bits clockwise.  The ILBM loader uses this to convert pixel
bits from planar to chunky.
===================================================================== */

void bitrot_cw( unsigned char *src, int srcstep, unsigned char *dst, int dststep )
{
   unsigned char *p;
   int pstep, lonyb, hinyb;
   unsigned long lo, hi;

   lo = hi = 0;

   p = src; pstep = srcstep;
   extract( p, rtab0 )
   extract( p, rtab1 )
   extract( p, rtab2 )
   extract( p, rtab3 )
   extract( p, rtab4 )
   extract( p, rtab5 )
   extract( p, rtab6 )
   extract( p, rtab7 )

#define writebits_cw( d, w ) \
   *d = ( unsigned char )(( w >> 24 ) & 0xFF ); d += pstep; \
   *d = ( unsigned char )(( w >> 16 ) & 0xFF ); d += pstep; \
   *d = ( unsigned char )(( w >>  8 ) & 0xFF ); d += pstep; \
   *d = ( unsigned char )( w & 0xFF );

   p = dst; pstep = dststep;
   writebits_cw( p, hi )
   p += pstep;
   writebits_cw( p, lo )
}


/*
=====================================================================
bitrot_ccw()

Rotate bits counterclockwise.  The ILBM saver uses this to convert
pixel bits from chunky to planar.
===================================================================== */

void bitrot_ccw( unsigned char *src, int srcstep, unsigned char *dst, int dststep )
{
   unsigned char *p;
   int pstep, lonyb, hinyb;
   unsigned long lo, hi;

   lo = hi = 0;

   p = src; pstep = srcstep;
   extract( p, rtab7 )
   extract( p, rtab6 )
   extract( p, rtab5 )
   extract( p, rtab4 )
   extract( p, rtab3 )
   extract( p, rtab2 )
   extract( p, rtab1 )
   extract( p, rtab0 )

#define writebits_ccw( d, w ) \
   *d = ( unsigned char )( w & 0xFF );          d += pstep; \
   *d = ( unsigned char )(( w >>  8 ) & 0xFF ); d += pstep; \
   *d = ( unsigned char )(( w >> 16 ) & 0xFF ); d += pstep; \
   *d = ( unsigned char )(( w >> 24 ) & 0xFF );

   p = dst; pstep = dststep;
   writebits_ccw( p, lo )
   p += pstep;
   writebits_ccw( p, hi )
}
