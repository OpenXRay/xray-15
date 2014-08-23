/*
======================================================================
search.c

Implements searching for the LightWave SDK BinView sample plug-in.

Ernie Wright  18 Jul 99
====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "binview.h"


static char buf[ 1024 ];
static char searchstr[ 64 ];


/*
======================================================================
search0()

Search an open file for a byte pattern.

INPUTS
   fp       the file to search
   pos      start the search at this byte position in the file
   pat      the bytes to search for
   len      the length of pat, in bytes

RESULTS
   Returns the absolute file position at which the byte pattern was
   first found, or -1 if the pattern wasn't found at all.
====================================================================== */

static int search0( FILE *fp, int pos, char *pat, int len )
{
   int rlen;
   int i, found = 0;

   pos++;
   fseek( fp, pos, SEEK_SET );
   rlen = fread( buf, 1, len, fp );

   while ( 1 ) {
      rlen = fread( buf + len, 1, 1024 - len, fp );
      for ( i = 0; i < rlen + len; i++ )
         if ( !memcmp( pat, &buf[ i ], len )) {
            found = 1;
            break;
         }
      if ( found || ( rlen < ( 1024 - len ))) break;
      memcpy( buf, buf + 1024 - len, len );
      pos += 1024 - len;
   }

   if ( found )
      return pos + i;
   else
      return -1;
}


/*
======================================================================
search()

Search an open file.

INPUTS
   fp          the file to search
   pos         start position, storage for found position
   userstr     search key as entered by the user
   searchtype  how to interpret the search key
   datatype    type of binary data to search for
   unsign      whether ints in the search key are unsigned
   byteorder   little-endian or big-endian

RESULTS
   The search key as entered by the user is massaged to produce a byte
   pattern, which is then submitted to search0().  If the byte pattern
   is found, the pos argument is set to the position of the pattern in
   the file and the function returns TRUE (1).  Otherwise the function
   returns FALSE (0) without modifying pos.

The contents of userstr are interpreted differently depending on the
value of searchtype.  Possible values of searchtype are

   0  userstr contains text
   1  userstr contains a mix of text and hex digits
   2  userstr contains numbers to be converted to binary

See the comments in parse.c for an explanation of the mixed mode.  If
searchtype == 2, the numbers in userstr are converted to binary form
as the type in datatype, which can be one of the following.

   0  hex digits (equivalent to searchtype == 1)
   1  short int
   2  long int
   3  float
   4  double

If datatype is 1 or 2, the unsign flag determines whether the numbers
are interpreted as unsigned.  The byteorder code, which can be

   0  little-endian (Intel)
   1  big-endian (Motorola)

determines whether the bytes of the internal representation need to be
reversed.
====================================================================== */

int search( FILE *fp, int *pos, char *userstr, int searchtype,
   int datatype, int unsign, int byteorder )
{
   int len, result;

   switch ( searchtype ) {
      case 0:
         strcpy( searchstr, userstr );
         len = strlen( searchstr );
         break;
      case 1:
         len = parse( userstr, searchstr, 0, unsign, byteorder );
         break;
      case 2:
         len = parse( userstr, searchstr, datatype, unsign, byteorder );
         break;
   }

   if ( len ) {
      result = search0( fp, *pos, searchstr, len );
      if ( result != -1 ) {
         *pos = result;
         return 1;
      }
   }

   return 0;
}
