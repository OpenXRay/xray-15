/*
======================================================================
save.c

Ernie Wright  22 Mar 00

Functions for writing IFF ILBMs (IFF interleaved bitmaps).

PUBLIC FUNCTIONS
   InitILBM()    begin FORM, write BMHD, CMAP, begin BODY
   PutRowILBM()  write one BODY scanline
   CloseILBM()   close BODY and FORM

SUPPORTED IMAGE TYPES
   Encoding          Depth (planes)
   --------------------------------
    color map         1 - 8
    gray level        1 - 8
    RGB level         24
====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iff.h"


/*
======================================================================
write_form()

Write the first 12 bytes of an ILBM.  Called by InitILBM().
====================================================================== */

static int write_form( FILE *fp )
{
   unsigned long ck[ 3 ] = { ID_FORM, 0, ID_ILBM };

   if ( 1 != fwrite( ck, 12, 1, fp )) return 0;
   return 1;
}


/*
======================================================================
write_bmhd()

Write an ILBM bitmap header.  Called by InitILBM().
====================================================================== */

static int write_bmhd( FILE *fp, BMHD *bmhd )
{
   BMHD b = *bmhd;
   unsigned long ck[ 2 ] = { ID_BMHD, 20 };

   revbytes( &ck[ 1 ], 4, 1 );
   revbytes( &b.w, 2, 4 );
   revbytes( &b.pw, 2, 2 );

   if ( 1 != fwrite( ck, 8, 1, fp )) return 0;
   if ( 1 != fwrite( &b, 20, 1, fp )) return 0;

   return 1;
}


/*
======================================================================
write_cmap()

Write an ILBM color table.  Called by InitILBM().
====================================================================== */

static int write_cmap( FILE *fp, RGBTriple *cmap, int ncolors )
{
   unsigned long ck[ 2 ] = { ID_CMAP, ncolors * 3 };

   revbytes( &ck[ 1 ], 4, 1 );

   if ( 1 != fwrite( ck, 8, 1, fp )) return 0;
   if ( 1 != fwrite( cmap, ncolors * 3, 1, fp )) return 0;

   return 1;
}


/*
======================================================================
write_channel()

Write one 8-bit scanline channel to an ILBM BODY chunk.  The channel
comes in with one byte per pixel, but nplanes bitplanes are written,
and this can be less than 8 (it should never be more).  Called by
PutRowILBM().
====================================================================== */

static int write_channel( FILE *fp, unsigned char *src, unsigned char *rot,
   unsigned char *dst, int rowbytes, int nplanes )
{
   unsigned char *p, *q;
   int i, x, n, size;

   p = src;
   q = rot;
   for ( x = 0; x < rowbytes; x++ ) {
      bitrot_ccw( p, 1, q, rowbytes );
      p += 8;
      q++;
   }

   p = rot;
   q = dst;
   n = size = 0;
   for ( i = 0; i < nplanes; i++ ) {
      size += n = pack( p, q, rowbytes );
      p += rowbytes;
      q += n;
   }

   if ( 1 != fwrite( dst, size, 1, fp )) return 0;

   return size;
}


/*
======================================================================
InitILBM()

Write the ILBM header chunks.  The file pointer is positioned at the
beginning of the BODY chunk, ready to receive scanlines.
====================================================================== */

int InitILBM( PicInfo *pic )
{
   unsigned long ck[ 2 ] = { ID_BODY, 0 };

   if ( !write_form( pic->fp )) return FALSE;
   if ( !write_bmhd( pic->fp, &pic->bmhd )) return FALSE;

   if ( pic->bmhd.nPlanes <= 8 && !pic->isgray )
      write_cmap( pic->fp, pic->cmap, pic->ncolors );

   if ( 1 != fwrite( ck, 8, 1, pic->fp )) return FALSE;

   /* we'll need to know this later */

   pic->BODYpos = ftell( pic->fp );

   return TRUE;
}


/*
======================================================================
PutRowILBM()

Write one scanline to the ILBM BODY chunk.
====================================================================== */

int PutRowILBM( PicInfo *pic )
{
   int i, size, nchans, nchplanes;

   nchans = ( pic->bmhd.nPlanes == 24 ) ? 3 : 1;
   nchplanes = ( nchans == 3 ) ? 8 : pic->bmhd.nPlanes;

   for ( i = 0; i < nchans; i++ ) {
      size = write_channel( pic->fp, pic->rgb[ i ], pic->buf, pic->body,
         pic->rowsize, nchplanes );
      if ( !size ) return FALSE;
   }

   return TRUE;
}


/*
======================================================================
CloseILBM()

Write one scanline to the ILBM BODY chunk.
====================================================================== */

int CloseILBM( PicInfo *pic )
{
   unsigned long size, pos;

   /* size of the BODY chunk */

   size = ftell( pic->fp ) - pic->BODYpos;

   /* if odd, add a pad byte (not part of the BODY size) */

   if ( size & 1 )
      fputc( 0, pic->fp );

   /* FORM size */

   pos = ftell( pic->fp ) - 8;

   /* seek to the start of the BODY chunk and write the size */

   fseek( pic->fp, pic->BODYpos - 4, SEEK_SET );
   revbytes( &size, 4, 1 );
   fwrite( &size, 4, 1, pic->fp );

   /* seek to the start of the file and write the FORM size */

   fseek( pic->fp, 4, SEEK_SET );
   revbytes( &pos, 4, 1 );
   fwrite( &pos, 4, 1, pic->fp );

   /* close the file */

   fclose( pic->fp );
   pic->fp = NULL;

   return TRUE;
}
