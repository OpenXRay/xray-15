/*
======================================================================
str.c

Edit field string formatting for DOF Toy.

Ernie Wright  11 Jul 00

Rather than use LWPanels built-in numeric edit fields, we create our
own using STR_CTRL() and these conversion functions.  We get more
control and flexibility this way, at the cost of some additional work.
The zoom factor, for example, can be represented as a focal length or
field of view angle using the same edit field.
====================================================================== */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "doftoy.h"


static struct {
   char  *unit;
   double scale;
} distrec[] =
{
   "km",    1000.0,
   "cm",       0.01,
   "mm",       0.001,
   "um",    1e-6,
   "\xB5m", 1e-6,
   "nm",    1e-9,
   "mi",    1609.344,
   "yd",       0.9144,
   "ft",       0.3048,
   "in",       0.0254,
   "'",        0.3048,
   "\"",       0.0254,
   NULL,       0.0
};


double sget_dist( char *a )
{
   double d, s = 1.0;
   int i;

   for ( i = 0; distrec[ i ].unit; i++ )
      if ( strstr( a, distrec[ i ].unit ))
         s = distrec[ i ].scale;

   d = atof( a );
   d *= s;
   return d;
}


char *sput_dist( char *a, double d )
{
   int mag;

   if ( d <= 0.0 || d == INVALID_NUMBER )
      sprintf( a, "***" );

   else {
      mag = ( int ) floor( log10( d ));
      if ( mag >= 3 )
         sprintf( a, "%.4g km", d * 0.001 );
      else if ( mag >= 0 )
         sprintf( a, "%.4g m", d );
      else if ( mag == -1 )
         sprintf( a, "%.4g cm", d * 100.0 );
      else if ( mag >= -3 )
         sprintf( a, "%.4g mm", d * 1000.0 );
      else if ( mag >= -6 )
         sprintf( a, "%.4g \xB5m", d * 1e6 );
      else
         sprintf( a, "%.4g nm", d * 1e9 );
   }

   return a;
}


double sget_angle( char *a )
{
   double d;

   d = atof( a );
   d *= PI / 180.0;
   return d;
}


char *sput_angle( char *a, double d )
{
   d *= 180.0 / PI;
   if ( d > 0.0 && d != INVALID_NUMBER )
      sprintf( a, "%.4g%c", d, 176 );
   else
      sprintf( a, "***" );
   return a;
}


double sget_double( char *a )
{
   return atof( a );
}


char *sput_double( char *a, double d )
{
   if ( d > 0.0 && d != INVALID_NUMBER )
      sprintf( a, "%.4g", d );
   else
      sprintf( a, "***" );
   return a;
}
