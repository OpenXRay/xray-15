/*
======================================================================
func.c

Functions that return one depth-of-field parameter in terms of others.

Ernie Wright  11 Jul 00

This is where all of the depth-of-field math is done.  I first did the
algebra for the conversion functions by hand, which gave me a better
appreciation for the interdependence of the parameters, and then I ran
the equations through a symbolic math program to verify my results.

This is what the one-letter variables represent:

   n  Near
   s  Focal Distance
   x  Far
   f  Focal Length
   c  Circle of Confusion Diameter
   a  F-Stop (Aperture)
   h  Hyperfocal Distance
   z  Zoom Factor
   p  Aperture Height

Here are a couple of websites that explain the math.

   http://www.photo.net/photo/optics/lensTutorial.html
   http://www.outsight.com/hyperfocal.html
====================================================================== */

#include <stdio.h>
#include <math.h>
#include "doftoy.h"


/*
======================================================================
nx_s()

Calculate near n or far x (the one not given), given

   s           focal distance
   norx        near or far focus limit
====================================================================== */

double nx_s( double s, double norx )
{
   double d;

   d = 2 * norx - s;
   if ( d > 0.0 )
      return s * norx / d;
   else
      return INVALID_NUMBER;
}


/*
======================================================================
s_nx()

Calculate focal distance s, given

   n           near focus limit
   x           far focus limit
====================================================================== */

double s_nx( double n, double x )
{
   double d;

   d = n + x;
   if ( d > 0.0 )
      return 2.0 * n * x / d;
   else
      return INVALID_NUMBER;
}


/*
======================================================================
s_n()

Calculate focal distance s, given

   f           focal length
   a           aperture (f-stop)
   c           circle of confusion diameter
   n           near focus limit
====================================================================== */

double s_n( double f, double a, double c, double n )
{
   double d;

   d = a * c * n - f * f;
   if ( d != 0.0 )
      return f * n * ( a * c - f ) / d;
   else
      return INVALID_NUMBER;
}


/*
======================================================================
s_x()

Calculate focal distance s, given

   f           focal length
   a           aperture (f-stop)
   c           circle of confusion diameter
   x           far focus limit
====================================================================== */

double s_x( double f, double a, double c, double x )
{
   double d;

   d = a * c * x + f * f;
   if ( d != 0.0 )
      return f * x * ( a * c + f ) / d;
   else
      return INVALID_NUMBER;
}


/*
======================================================================
a_n()

Calculate aperture (f-stop) a, given

   f           focal length
   s           focal distance
   n           near focus limit
   c           circle of confusion diameter
====================================================================== */

double a_n( double f, double s, double n, double c )
{
   double d;

   d = c * n * ( f - s );
   if ( d != 0.0 )
      return f * f * ( n - s ) / d;
   else
      return INVALID_NUMBER;
}


/*
======================================================================
f_n()

Calculate focal length f, given

   a           aperture (f-stop)
   c           circle of confusion diameter
   n           near focus limit
   s           focal distance
====================================================================== */

double f_n( double a, double c, double n, double s )
{
   double d, acn;

   d = 2.0 * ( s - n );
   if ( d != 0.0 ) {
      acn = a * c * n;
      return ( sqrt( acn * ( acn + 4.0 * s * ( s - n ))) - acn ) / d;
   }
   else
      return INVALID_NUMBER;
}


/*
======================================================================
n_h()

Calculate near focus limit n, given

   f           focal length
   a           aperture (f-stop)
   c           circle of confusion diameter
   s           focal distance
====================================================================== */

double n_h( double f, double a, double c, double s )
{
   double h, d1, d2;

   d1 = a * c;
   if ( d1 != 0.0 ) {
      h = f * f / d1;
      d2 = h + s - f;
      if ( d2 != 0.0 )
         return h * s / d2;
   }
   return INVALID_NUMBER;
}


/*
======================================================================
z_vfov()

Calculate zoom factor z, given

   vfov        vertical field of view (radians)
====================================================================== */

double z_vfov( double vfov )
{
   if ( vfov > 0.0 && vfov < PI )
      return 1.0 / tan( vfov / 2.0 );
   else
      return INVALID_NUMBER;
}


/*
======================================================================
z_hfov()

Calculate zoom factor z, given

   hfov        horizontal field of view (radians)
   faspect     frame aspect ratio (width / height)
====================================================================== */

double z_hfov( double hfov, double faspect )
{
   if ( hfov > 0.0 && hfov < PI )
      return faspect / tan( hfov / 2.0 );
   else
      return INVALID_NUMBER;
}


/*
======================================================================
vfov_z()

Calculate vertical field of view vfov, given

   z           zoom factor
====================================================================== */

double vfov_z( double z )
{
   return 2.0 * atan2( 1.0, z );
}


/*
======================================================================
hfov_z()

Calculate horizontal field of view hfov, given

   z           zoom factor
   faspect     frame aspect ratio (width / height)
====================================================================== */

double hfov_z( double z, double faspect )
{
   if ( !( faspect == 0.0 && z == 0.0 ))
      return 2.0 * atan2( faspect, z );
   else
      return INVALID_NUMBER;
}


/*
======================================================================
z_fp()

Calculate zoom factor z, given

   f           focal length
   p           aperture height
====================================================================== */

double z_fp( double f, double p )
{
   if ( p > 0.0 )
      return 2.0 * f / p;
   else
      return INVALID_NUMBER;
}


/*
======================================================================
f_pz()

Calculate focal length f, given

   p           aperture height
   z           zoom factor
====================================================================== */

double f_pz( double p, double z )
{
   return z * p / 2.0;
}


/*
======================================================================
p_fz()

Calculate aperture height p, given

   f           focal length
   z           zoom factor
====================================================================== */

double p_fz( double f, double z )
{
   if ( z > 0.0 )
      return 2.0 * f / z;
   else
      return INVALID_NUMBER;
}


/*
======================================================================
update_params()

Restore mathematical consistency in a way that depends on which
parameter has changed and which two are constrained.

INPUTS
   changed     the parameter (0-6) changed by the user

RESULTS
   Returns an integer with bits 0-6 set for parameters that have been
   modified by this routine.  These flag bits are then used to update
   the user interface to reflect the changes.

The depth-of-field parameters form a pretty wiggly system--the choice
of constraints and the resulting degrees of freedom aren't easy to
analyze, for me anyway.  Here's what we do.

The "sliderized" parameters are divided into two groups, the near/
focal distance/far group, and the f-stop/zoom group.  One parameter
from each group is locked (fixed) by the user.  The choice of which
other parameters change when the user changes one of them depends on
these locks.

If the user changes a LOCKED parameter, the other parameters in the
SAME group are changed.  If the user changes an UNLOCKED parameter,
the UNLOCKED parameters in the OTHER group are changed.

If the user changes the circle of confusion diameter or the aperture
height, the unlocked parameters in the near/far group are changed.

These behaviors pretty much follow from the way the DOF parameters
depend on each other, as well as from the varieties of fluidity that
users are most likely to want (or understand).

Note that the circle of confusion diameter and the aperture height are
never changed by this routine, even though there's no mathematical
reason they can't be free to satisfy the constraints.  These two
parameters are just the most likely to be fixed in users' minds, so
allowing them to vary would probably be confusing.

Note too that this system can easily be mathematically consistent
while not making any physical sense--near can be farther than far, for
example.  How to represent this to the user is left to the interface
routines.
====================================================================== */

#define n param[ 0 ]    /* near                         */
#define s param[ 1 ]    /* focal distance               */
#define x param[ 2 ]    /* far                          */
#define a param[ 3 ]    /* f-stop (aperture)            */
#define z param[ 4 ]    /* zoom                         */
#define c param[ 5 ]    /* circle of confusion diameter */
#define p param[ 6 ]    /* aperture height              */

int update_params( int changed, double *param, int locka, int lockb )
{
   double f;            /* focal length */
   int update = 0;

   if ( changed < 3 ) {
      /* near, focal distance, far */
      switch ( changed ) {
         case 0:
            /* near was changed */
            switch ( locka ) {
               case 0:
                  /* near locked, update focal distance and far */
                  f = f_pz( p, z );
                  s = s_n( f, a, c, n );
                  x = nx_s( s, n );
                  update = 1 << 1 | 1 << 2;
                  break;
               case 1:
                  /* focal distance locked, update far */
                  x = nx_s( s, n );
                  update = 1 << 2;
                  break;
               case 2:
                  /* far locked, update focal distance */
                  s = s_nx( n, x );
                  update = 1 << 1;
                  break;
            }
            break;

         case 1:
            /* focal distance was changed */
            switch ( locka ) {
               case 0:
                  /* near locked, update far */
                  x = nx_s( s, n );
                  update = 1 << 2;
                  break;
               case 1:
                  /* focal distance locked, update near and far */
                  f = f_pz( p, z );
                  n = n_h( f, a, c, s );
                  x = nx_s( s, n );
                  update = 1 << 0 | 1 << 2;
                  break;
               case 2:
                  /* far locked, update near */
                  n = nx_s( s, x );
                  update = 1 << 0;
                  break;
            }
            break;

         case 2:
            /* far was changed */
            switch ( locka ) {
               case 0:
                  /* near locked, update focal distance */
                  s = s_nx( n, x );
                  update = 1 << 1;
                  break;
               case 1:
                  /* focal distance locked, update near */
                  n = nx_s( s, x );
                  update = 1 << 0;
                  break;
               case 2:
                  /* far locked, update focal distance and near */
                  f = f_pz( p, z );
                  s = s_x( f, a, c, x );
                  n = nx_s( s, x );
                  update = 1 << 0 | 1 << 1;
                  break;
            }
            break;
      }

      if ( changed != locka )
         /* an unlocked parameter was changed, we need to update the unlocked
            parameter in the other group */
         switch ( lockb ) {
            case 3:
               /* f-stop locked, change zoom */
               f = f_n( a, c, n, s );
               z = z_fp( f, p );
               update |= 1 << 4;
               break;
            case 4:
               /* zoom locked, change f-stop */
               f = f_pz( p, z );
               a = a_n( f, s, n, c );
               update |= 1 << 3;
               break;
         }
   }

   else if ( changed == lockb ) {
      /* the locked parameter in the f-stop/zoom group was changed */
      switch ( changed ) {
         case 3:
            /* f-stop was changed, update zoom */
            f = f_n( a, c, n, s );
            z = z_fp( f, p );
            update = 1 << 4;
            break;
         case 4:
            /* zoom was changed, update f-stop */
            f = f_pz( p, z );
            a = a_n( f, s, n, c );
            update = 1 << 3;
            break;
      }
   }

   else {
      /* the circle of confusion diameter, the aperture height, or an unlocked
         parameter in the f-stop/zoom group changed, so we update the unlocked
         parameters in the other group */

      f = f_pz( p, z );
      switch ( locka ) {
         case 0:
            /* near locked, update focal distance and far */
            s = s_n( f, a, c, n );
            x = nx_s( s, n );
            update = 1 << 1 | 1 << 2;
            break;
         case 1:
            /* focal distance locked, update near and far */
            n = n_h( f, a, c, s );
            x = nx_s( s, n );
            update = 1 << 0 | 1 << 2;
            break;
         case 2:
            /* far locked, update near and focal distance */
            s = s_x( f, a, c, x );
            n = nx_s( s, x );
            update = 1 << 0 | 1 << 1;
            break;
      }
   }

   return update;
}
