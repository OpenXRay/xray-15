/*
======================================================================
lwio.c

Functions for reading and writing basic data types.

Ernie Wright  4 Dec 00
====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "flex.h"


/*
======================================================================
flen

This accumulates a count of the number of bytes read.  Callers can set
it at the beginning of a sequence of reads and then retrieve it to get
the number of bytes actually read.  If one of the I/O functions fails,
flen is set to an error code, after which the I/O functions ignore
read requests until flen is reset.
====================================================================== */

#define FLEN_ERROR INT_MIN

static int flen;

void set_flen( int i ) { flen = i; }

int get_flen( void ) { return flen; }


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

This only needs to be defined on little-endian platforms, most
notably Windows.  lwo2.h replaces this with a #define on big-endian
platforms.
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


void skipbytes( FILE *fp, int n )
{
   if ( flen == FLEN_ERROR ) return;
   if ( fseek( fp, n, SEEK_CUR ))
      flen = FLEN_ERROR;
   else
      flen += n;
}


void putbytes( FILE *fp, char *buf, int n )
{
   if ( flen == FLEN_ERROR ) return;
   if ( 1 != fwrite( buf, n, 1, fp ))
      flen = FLEN_ERROR;
   else
      flen += n;
}


short getI2( FILE *fp )
{
   short i;

   if ( flen == FLEN_ERROR ) return 0;
   if ( 1 != fread( &i, 2, 1, fp )) {
      flen = FLEN_ERROR;
      return 0;
   }
   revbytes( &i, 2, 1 );
   flen += 2;
   return i;
}


unsigned int getU4( FILE *fp )
{
   unsigned int i;

   if ( flen == FLEN_ERROR ) return 0;
   if ( 1 != fread( &i, 4, 1, fp )) {
      flen = FLEN_ERROR;
      return 0;
   }
   revbytes( &i, 4, 1 );
   flen += 4;
   return i;
}


float getF4( FILE *fp )
{
   float f;

   if ( flen == FLEN_ERROR ) return 0.0f;
   if ( 1 != fread( &f, 4, 1, fp )) {
      flen = FLEN_ERROR;
      return 0.0f;
   }
   revbytes( &f, 4, 1 );
   flen += 4;
   return f;
}


void putI2( FILE *fp, short i )
{
   if ( flen == FLEN_ERROR ) return;
   revbytes( &i, 2, 1 );
   if ( 1 != fwrite( &i, 2, 1, fp ))
      flen = FLEN_ERROR;
   else
      flen += 2;
}


void putU4( FILE *fp, unsigned int i )
{
   if ( flen == FLEN_ERROR ) return;
   revbytes( &i, 4, 1 );
   if ( 1 != fwrite( &i, 4, 1, fp ))
      flen = FLEN_ERROR;
   else
      flen += 4;
}


void putF4( FILE *fp, float f )
{
   if ( flen == FLEN_ERROR ) return;
   revbytes( &f, 4, 1 );
   if ( 1 != fwrite( &f, 4, 1, fp ))
      flen = FLEN_ERROR;
   else
      flen += 4;
}
