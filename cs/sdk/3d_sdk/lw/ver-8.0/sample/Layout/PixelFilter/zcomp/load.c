/*
======================================================================
load.c

An LWImageLoader for loading 3D Nature IFF z-buffer files.  Requires
LW 6 or later.

Ernie Wright  22 Jul 00
Chris "Xenon" Hanson  7 Aug 96

LW 6 supports images with floating-point RGB levels.  We're taking
advantage of that by loading ZBUF files as grayscale images, which
among other things gives us LW's image sequence management for free.
====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lwserver.h>
#include <lwimage.h>
#include "zbuf6.h"


/*
======================================================================
open_zbuf()

Open and read the start of an image file.  If the file is a 3D Nature
ZBUF file containing data in a format we can use, and there aren't any
read errors, the header structure is filled in and the file pointer is
left at the start of the actual z-buffer data.  Returns TRUE if
successful, otherwise FALSE.

We're part of a chain of image loaders called for every image loaded
into LW, so we have to check the files carefully and assume that most
of them are *not* ZBUF files.
====================================================================== */

static int open_zbuf( FILE *fp, ZBufferHeader *zhd )
{
   unsigned long bodsize, ck[ 3 ] = { 0 };

   /* FORM ILBM? */

   if ( 1 != fread( ck, 12, 1, fp )) return 0;
   if ( ck[ 0 ] != ID_FORM || ck[ 2 ] != ID_ILBM ) return 0;

   /* remember the FORM size */

   ck[ 2 ] = ck[ 1 ];
   revbytes( &ck[ 2 ], 4, 1 );

   /* look for ZBUF and ZBOD */

   zhd->w = 0;
   if ( 1 != fread( ck, 8, 1, fp )) return 0;
   revbytes( &ck[ 1 ], 4, 1 );

   while ( 1 ) {
      ck[ 1 ] += ( ck[ 1 ] & 1 );
      if ( ck[ 0 ] == ID_ZBOD ) break;

      if ( ck[ 0 ] == ID_ZBUF ) {
         if ( ck[ 1 ] != 36 ) return 0;
         if ( 1 != fread( zhd, 36, 1, fp )) return 0;
         revbytes( &zhd->w, 4, 2 );
         revbytes( &zhd->datatype, 2, 4 );
         revbytes( &zhd->min, 4, 5 );
      }
      else
         if ( fseek( fp, ck[ 1 ], SEEK_CUR )) return 0;

      /* past the end of the FORM? */

      if (( signed ) ck[ 2 ] < ftell( fp )) return 0;

      if ( 1 != fread( ck, 8, 1, fp )) return 0;
      revbytes( &ck[ 1 ], 4, 1 );
   }

   /* found ZBOD and ZBUF? */

   if ( ck[ 0 ] != ID_ZBOD ) return 0;
   if ( zhd->w == 0 ) return 0;

   /* test for some error conditions */

   if ( zhd->compression != ZBCOMP_NONE ) return 0;
   if ( zhd->sorting != ZBSORT_NEARTOFAR ) return 0;
   if ( zhd->scale != 1.0f ) return 0;
   if ( zhd->datatype != ZBVAR_FLOAT && zhd->datatype != ZBVAR_DOUBLE )
      return 0;
   bodsize = zhd->w * zhd->h * 4;
   if ( zhd->datatype == ZBVAR_DOUBLE ) bodsize *= 2;
   if ( ck[ 1 ] != bodsize ) return 0;
   if ( ck[ 1 ] > ck[ 2 ] ) return 0;

   /* success */

   return 1;
}


/*
======================================================================
read_zbuf()

Read depth values from a ZBUF file.  On entry, the file pointer is
assumed to be at the start of the z values, which is where open_zbuf()
leaves it.  The values are passed to LW one row at a time.
====================================================================== */

static void read_zbuf( LWImageLoaderLocal *local, FILE *fp, int w, int h,
   int datatype )
{
   LWImageProtocolID ip = NULL;
   float *fbuf;
   int x, y = 0;


   /* get the row buffer */

   fbuf = malloc( w * 4 );
   if ( !fbuf ) {
      local->result = IPSTAT_FAILED;
      return;
   }

   /* get the LW image protocol */

   ip = local->begin( local->priv_data, LWIMTYP_GREYFP );
   if ( !ip ) {
      free( fbuf );
      local->result = IPSTAT_FAILED;
      return;
   }

   /* set some parameters */

   LWIP_SETSIZE( ip, w, h );
   LWIP_ASPECT( ip, 1.0f );

   /* send the data */

   if ( datatype == ZBVAR_FLOAT ) {
      for ( y = 0; y < h; y++ ) {
         if ( 4 != fread( fbuf, w, 4, fp )) break;
         revbytes( fbuf, 4, w );
         if ( LWIP_SENDLINE( ip, y, fbuf )) break;
      }
   }

   else if ( datatype == ZBVAR_DOUBLE ) {
      double *dbuf = malloc( w * 8 );
      if ( dbuf ) {
         for ( y = 0; y < h; y++ ) {
            if ( 8 != fread( dbuf, w, 8, fp )) break;
            revbytes( fbuf, 8, w );
            for ( x = 0; x < w; x++ )
               fbuf[ x ] = ( float ) dbuf[ x ];
            if ( LWIP_SENDLINE( ip, y, fbuf )) break;
         }
         free( dbuf );
      }
   }

   /* done */

   free( fbuf );
   local->result = LWIP_DONE( ip, y < h ? IPSTAT_FAILED : IPSTAT_OK );
   local->done( local->priv_data, ip );
}


/*
======================================================================
ZLoad()

Handle request to load an image.  Called by LW.
====================================================================== */

XCALL_( int )
ZLoad( long version, GlobalFunc *global, LWImageLoaderLocal *local,
   void *serverData )
{
   ZBufferHeader zhd;
   FILE *fp;

   fp = fopen( local->filename, "rb" );
   if ( !fp ) {
      local->result = IPSTAT_BADFILE;
      return AFUNC_OK;
   }

   if ( !open_zbuf( fp, &zhd )) {
      fclose( fp );
      local->result = IPSTAT_NOREC;
      return AFUNC_OK;
   }

   read_zbuf( local, fp, zhd.w, zhd.h, zhd.datatype );

   fclose( fp );
   return AFUNC_OK;
}
