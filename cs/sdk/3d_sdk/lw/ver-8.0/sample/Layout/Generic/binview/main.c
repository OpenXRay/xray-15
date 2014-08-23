/*
======================================================================
main.c

Entry point and high-level display functions for the LightWave SDK
BinView sample plug-in, a binary file viewer.

Ernie Wright  18 Jul 99

BinView is a non-trivial (I hope) demo of the "classic" LightWave
panels interface components.  It's also useful as a programming tool
for debugging code that reads and writes binary files.  I've been
using this code in one form or another for over a decade, replacing
the system-specific code in window.c whenever I move it to a new
platform.

main.c contains very little LightWave-specific code.  The functions
here write to a virtual display, a 78 x 32 array of character cells.
The show functions are called from other modules to format the text
output, and they call the external functions outtext() and clrtext()
to draw the output on the display.
====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <lwserver.h>
#include <lwgeneric.h>
#include <lwcmdseq.h>
#include "binview.h"


/* byte width of each data type */

static int size[] = { 1, 2, 4, 4, 8, 2, 4 };


/*
======================================================================
testf()

Convert an IEEE float to text.

INPUTS
   f     pointer to bytes that may contain a valid IEEE float
   c     string to put text in

RESULTS
   The bytes at f are examined to determine whether they represent an
   IEEE single-precision floating-point number or the reserved values
   <infinity> or <not a number>, and an appropriate text representa-
   tion is written to c.

The bit pattern's IEEE interpretation is

   sign     exp      mant     value
   --------------------------------
   0        0        0        0.0
   0        127      0        1.0
   any      255      0        INF
   any      255      not 0    NAN
====================================================================== */

static int testf( void *f, char *c )
{
   unsigned long n, e, m;

   n = *(( unsigned long * ) f );
   e = ( n & 0x7F800000 ) >> 23;
   m = n & 0x007FFFFF;
   if ( e == 255 )
      return sprintf( c, "%12s", m ? "NAN" : "INF" );
   else
      return sprintf( c, "%12.5hg", *(( float * ) f ));
}


/*
======================================================================
testd()

Convert an IEEE double to text.

INPUTS
   d     pointer to bytes that may contain a valid IEEE double
   c     string to put text in

RESULTS
   The bytes at d are examined to determine whether they represent an
   IEEE double-precision floating-point number or the reserved values
   <infinity> or <not a number>, and an appropriate text representa-
   tion is written to c.

The bit pattern's IEEE interpretation is

   sign     exp      mant     value
   --------------------------------
   0        0        0        0.0
   0        1023     0        1.0
   any      2047     0        INF
   any      2047     not 0    NAN
====================================================================== */

static int testd( void *d, char *c )
{
   unsigned long n, e, m;

   n = *(( unsigned long * ) d );
   e = ( n & 0x7FF00000 ) >> 20;
   m = ( n & 0x000FFFFF ) | *( (( unsigned long * ) d ) + 1 );

   if ( e == 2047 )
      return sprintf( c, "%24s", m ? "NAN" : "INF" );
   else
      return sprintf( c, "%24.14lg", *(( double * ) d ));
}


/*
======================================================================
num_string()

Convert a binary field into its string representation.

INPUTS
   bp       pointer to bytes
   str      string to put text in
   type     a numeric type code

RESULTS
   An appropriate text representation is written to str.

Valid type codes are

   0  byte
   1  signed 2-byte integer
   2  signed 4-byte integer
   3  4-byte IEEE float
   4  8-byte IEEE double
   5  unsigned 2-byte integer
   6  unsigned 4-byte integer
====================================================================== */

static int num_string( unsigned char *bp, char *str, int type )
{
   static long n[ 2 ];

   memcpy( n, bp, size[ type ] );
   switch ( type ) {
      case 0:  return sprintf( str, " %02.2X", bp[ 0 ] );
      case 1:  return sprintf( str, "%6hd",  *(( short * ) n ));
      case 2:  return sprintf( str, "%12ld", *(( long  * ) n ));
      case 3:  return testf( n, str );
      case 4:  return testd( n, str );
      case 5:  return sprintf( str, "%6hu",  *(( unsigned short * ) n ));
      case 6:  return sprintf( str, "%12lu", *(( unsigned long  * ) n ));
      default: return 0;
   }
}


/*
======================================================================
num_line()

Convert a line of binary fields into their string representations.

INPUTS
   bp          pointer to bytes
   line        string to put text in
   rowbytes    number of source bytes
   type        a numeric type code

RESULTS
   Calls num_string() for each field in the line.

See the num_string() comments for valid type codes.  If rowbytes isn't
evenly divisible by the byte width of the type, the remaining bytes
are displayed as type 0.  If the resulting text is less than 48
characters long, the remainder of the text line is padded with spaces.

Called by show_num() and print_buf().
====================================================================== */

static int num_line( char *bp, char *line, int rowbytes, int type )
{
   int i, len;
   char *c;

   c = line;
   len = rowbytes / size[ type ];
   for ( i = 0; i < len; i++ ) {
      c += num_string( bp, c, type );
      bp += size[ type ];
   }

   len = rowbytes % size[ type ];
   for ( i = 0; i < len; i++ ) {
      c += num_string( bp, c, 0 );
      ++bp;
   }

   len = c - line;
   while ( c - line < 48 ) *c++ = ' ';
   line[ 48 ] = 0;
   return len;
}


/*
======================================================================
reverse_bytes()

Reverse the byte order of a memory block in place.

INPUTS
   bp          memory block
   elsize      size of a binary field
   elcount     number of binary fields in the block
====================================================================== */

void reverse_bytes( void *bp, int elsize, int bytecount )
{
   register unsigned char *p, *q;
   register int elcount;

   p = ( unsigned char * ) bp;
   elcount = bytecount / elsize;

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


/*
======================================================================
show_num()

Draw the main display.

INPUTS
   buf         memory block
   bufsize     size of the block (<= 512)
   type        data type code
   unsign      whether the fields should be treated as unsigned
   rev         whether the bytes in the block should be swabbed
   rowbytes    number of input bytes in one row of output
====================================================================== */

void show_num( char *buf, int bufsize, int type, int unsign, int rev,
   int rowbytes )
{
   static char a[ 64 ];
   int w, row = 1;

   if ( unsign && ( type == 1 || type == 2 )) type += 4;

   while ( bufsize > 0 ) {
      w = bufsize > rowbytes ? rowbytes : bufsize;
      if ( rev ) reverse_bytes( buf, size[ type ], w );
      num_line( buf, a, w, type );
      buf += w;
      bufsize -= w;
      outtext( row++, 13, a );
   }
   if ( row <= NROWS ) clrtext( row, 13, NROWS, 60 );
}


/*
======================================================================
show_byt()

Draw the byte position column of the display.

INPUTS
   pos         byte position of the start of the block
   bufsize     size of the block (<= 512)
   w           number of input bytes in one row of output
====================================================================== */

void show_byt( long pos, int bufsize, int w )
{
   char a[ 16 ];
   int i, row;

   row = 1;
   for ( i = 0; i < bufsize; i += w ) {
      sprintf( a, "%10ld", pos + i );
      outtext( row++, 1, a );
   }
   if ( row <= NROWS ) clrtext( row, 1, NROWS, 10 );
}


/*
======================================================================
show_prt()

Draw the printable character column of the display.

INPUTS
   buf         memory block
   bufsize     size of the block (<= 512)
   w           number of input bytes in one row of output
====================================================================== */

void show_prt( char *buf, int bufsize, int w )
{
   static char a[ 17 ];
   int row = 1, i, j = 0;

   memset( a, ' ', 16 );
   for ( i = 0; i < bufsize; i++ ) {
      if (( buf[ i ] > 31 ) && ( buf[ i ] < 127 )) a[ j ] = buf[ i ];
      if (( ++j == w ) || ( i == bufsize - 1 )) {
         outtext( row, 63, a );
         j = 0;
         ++row;
         memset( a, ' ', 16 );
      }
   }
   if ( row <= NROWS ) clrtext( row, 63, NROWS, 78 );
}


/*
======================================================================
print_buf()

Print the display to a file.

INPUTS
   buf         memory block
   bufsize     size of the block
   type        data type code
   unsign      whether the fields should be treated as unsigned
   rev         whether the bytes in the block should be swabbed
   rowbytes    number of input bytes in one row of output
   pos         input file position
   flags       columns to print (1 = pos, 2 = buffer, 4 = printable)
   fp          the output file
====================================================================== */

void print_buf( char *buf, int bufsize, int type, int unsign, int rev,
   int rowbytes, long pos, int flags, FILE *fp )
{
   static char a[ ROWBYTES * 4 ];
   int w, i;

   if ( unsign && ( type == 1 || type == 2 )) type += 4;

   while ( bufsize > 0 ) {
      w = bufsize > rowbytes ? rowbytes : bufsize;

      if ( flags & 1 )
         fprintf( fp, "   %10ld", pos );

      if ( flags & 2 ) {
         if ( rev && size[ type ] > 1 )
            reverse_bytes( buf, size[ type ], w );
         num_line( buf, a, w, type );
         a[ rowbytes * 3 ] = 0;
         fprintf( fp, "   %s", a );
         if ( rev && size[ type ] > 1 )
            reverse_bytes( buf, size[ type ], w );
      }

      if ( flags & 4 ) {
         for ( i = 0; i < w; i++ )
            if (( buf[ i ] > 31 ) && ( buf[ i ] < 127 ))
               a[ i ] = buf[ i ];
            else
               a[ i ] = ' ';
         a[ w ] = 0;
         fprintf( fp, "   %s", a );
      }

      fputc( '\n', fp );
      pos += w;
      buf += w;
      bufsize -= w;
   }
}


/*
======================================================================
native_order()

Determines the byte order of the machine.
====================================================================== */

int native_order( void )
{
   int i = 0x04030201, j;

   j = *(( char * ) &i );
   switch ( j ) {
      case 1:  return 0;  // Intel
      case 4:  return 1;  // Motorola
      default: return 2;  // other
   }
}


/*
======================================================================
BinView()

Entry point.
====================================================================== */

XCALL_( int )
BinView( long version, GlobalFunc *global, void *local, void *serverData )
{
   int result;

   result = open_window( global );
   if ( result == AFUNC_OK )
      free_window();

   return result;
}


ServerRecord ServerDesc[] = {
   { LWLAYOUTGENERIC_CLASS, PLUGIN_NAME, BinView },
   { LWMODCOMMAND_CLASS, PLUGIN_NAME, BinView },
   { NULL }
};
