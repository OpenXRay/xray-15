/*
======================================================================
parse.c

User input massage for the search facility, part of the LightWave SDK
BinView sample plug-in.

Ernie Wright  18 Jul 99
====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "binview.h"


/*
======================================================================
stpblk()

Skip whitespace, returning a pointer to the first non-blank character.
A useful little function in the Lattice C runtime.
====================================================================== */

static char *stpblk( char *a )
{
   while ( *a && isspace( *a )) ++a;
   return a;
}


/*
======================================================================
parse()

Convert a search string typed by the user into a byte sequence that
can be compared directly to values in the buffer.

INPUTS
   a        the string entered by the user
   b        storage for the converted string
   type     the kind of data in string a
   unsign   whether ints are unsigned
   byteorder

RESULTS
   Writes the converted string in b.  Returns the length of b.

Type 0 allows the user to enter both text and hexadecimal digits.  Hex
digits can be separated by spaces or by any other non-hex characters,
which are ignored.  Text is entered by escaping each character to be
treated as text using a forward slash ('/').  For example,

   "01 02 E5 /t/e/x/t"

is converted to the same internal representation as that produced by
the C expression

   char b[] = { 0x01, 0x02, 0xE5, 't', 'e', 'x', 't' };

The other types correspond to the binary data types short int, long
int (both possibly unsigned), float and double.
====================================================================== */

int parse( char *a, char *b, int type, int unsign, int byteorder )
{
   short          *w;
   unsigned short *uw;
   long           *l;
   unsigned long  *ul;
   float          *f;
   double         *d;

   int c, i = 0, j = 0, natorder;


   natorder = native_order();

   switch ( type )
   {
      case 0:
         while ( c = *a++ ) {
            if ( isxdigit( c )) {
               if ( isdigit( c )) c -= '0';
               else c = 10 + (( c - 'A' ) & 7 );
               if ( i & 1 ) b[ j++ ] |= c;
               else b[ j ] = c << 4;
               ++i;
            }
            else if ( c == '/' ) {
               b[ j++ ] = *a++;
               i += ( i & 1 );
            }
         }
         return j;

      case 1:
         if ( unsign ) {
            uw = ( unsigned short * ) b;
            while ( *a ) {
               *uw++ = ( unsigned short ) strtoul( a, &a, 10 );
               a = stpblk( a );
               ++i;
            }
         }
         else {
            w = ( short * ) b;
            while ( *a ) {
               *w++ = ( short ) strtol( a, &a, 10 );
               a = stpblk( a );
               ++i;
            }
         }
         i *= 2;
         if ( byteorder != natorder ) reverse_bytes( b, 2, i );
         return i;

      case 2:
         if ( unsign ) {
            ul = ( unsigned long * ) b;
            while ( *a ) {
               *ul++ = strtoul( a, &a, 10 );
               a = stpblk( a );
               ++i;
            }
         }
         else {
            l = ( long * ) b;
            while ( *a ) {
               *l++ = strtol( a, &a, 10 );
               a = stpblk( a );
               ++i;
            }
         }
         i *= 4;
         if ( byteorder != natorder ) reverse_bytes( b, 4, i );
         return i;

      case 3:
         f = ( float * ) b;
         while ( *a ) {
            *f++ = ( float ) strtod( a, &a );
            a = stpblk( a );
            ++i;
         }
         i *= 4;
         if ( byteorder != natorder ) reverse_bytes( b, 4, i );
         return i;

      case 4:
         d = ( double * ) b;
         while ( *a ) {
            *d++ = strtod( a, &a );
            a = stpblk( a );
            ++i;
         }
         i *= 8;
         if ( byteorder != natorder ) reverse_bytes( b, 8, i );
         return i;

      default:
         break;
   }
   return 0;
}
