/*
======================================================================
vecmath.c

A Global class plug-in that supplies vector and matrix math functions.

Ernie Wright  19 Mar 00

I was looking at Ron Goldman's revised version of Andrew Glassner's
GRAPHICS GEMS vector library when I wrote this.  See GG II, James
Arvo ed., Academic Press, 1991, ISBN 0-12-064480-0.  I also looked at
Richard Carling's matrix inversion using adjoints and determinants in
GG I (A. Glassner ed., ISBN 0-12-286165-5) and my undergraduate linear
algebra textbook.

The quaternion routines are from Alan Watt and Mark Watt, ADVANCED
ANIMATION AND RENDERING TECHNIQUES, ACM Press, 1992, ISBN
0-201-54412-1.

These implementations favor simplicity over efficiency.  If you need
speed, find a good LU decomposition routine for matrix inversion and
inline the rest.

To call this from your code, first get the structure with the math
functions by calling the global() function.  You can get single or
double precision versions of these.

   #include "vecmath.h"

   LWFVecMathFuncs *vec;

   vec = global( LWFVECMATHFUNCS_GLOBAL, GFUSE_TRANSIENT );
   if ( !vec ) return AFUNC_BADGLOBAL;

Then just call the functions.

   LWFVector a, b, c;

   ...
   vec->add( a, b, c );     // c = a + b
   ...
====================================================================== */

#include <lwserver.h>
#include <lwglobsrv.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "vecmath.h"


#define EPSILON_F  1e-6
#define EPSILON_D  1e-15

/* some matrix shorthand */

#define m00  m[ 0 ][ 0 ]
#define m01  m[ 0 ][ 1 ]
#define m02  m[ 0 ][ 2 ]
#define m03  m[ 0 ][ 3 ]
#define m10  m[ 1 ][ 0 ]
#define m11  m[ 1 ][ 1 ]
#define m12  m[ 1 ][ 2 ]
#define m13  m[ 1 ][ 3 ]
#define m20  m[ 2 ][ 0 ]
#define m21  m[ 2 ][ 1 ]
#define m22  m[ 2 ][ 2 ]
#define m23  m[ 2 ][ 3 ]
#define m30  m[ 3 ][ 0 ]
#define m31  m[ 3 ][ 1 ]
#define m32  m[ 3 ][ 2 ]
#define m33  m[ 3 ][ 3 ]


/*
======================================================================
Single precision functions.
====================================================================== */

void initv( LWFVector a, float x, float y, float z )
{
   a[ 0 ] = x;
   a[ 1 ] = y;
   a[ 2 ] = z;
}


void initm3( LWFMatrix3 m,
   float a1, float b1, float c1,
   float a2, float b2, float c2,
   float a3, float b3, float c3 )
{
   m00 = a1;  m01 = b1;  m02 = c1;
   m10 = a2;  m11 = b2;  m12 = c2;
   m20 = a3;  m21 = b3;  m22 = c3;
}


void initm4( LWFMatrix4 m,
   float a1, float b1, float c1, float d1,
   float a2, float b2, float c2, float d2,
   float a3, float b3, float c3, float d3,
   float a4, float b4, float c4, float d4 )
{
   m00 = a1;  m01 = b1;  m02 = c1;  m03 = d1;
   m10 = a2;  m11 = b2;  m12 = c2;  m13 = d2;
   m20 = a3;  m21 = b3;  m22 = c3;  m23 = d3;
   m30 = a4;  m31 = b4;  m32 = c4;  m33 = d4;
}


void identity3( LWFMatrix3 m )
{
   initm3( m,
      1.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 1.0f );
}


void identity4( LWFMatrix4 m )
{
   initm4( m,
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f );
}


void copyv( LWFVector to, LWFVector from )
{
   to[ 0 ] = from[ 0 ];
   to[ 1 ] = from[ 1 ];
   to[ 2 ] = from[ 2 ];
}


void copym3( LWFMatrix3 to, LWFMatrix3 from )
{
   memcpy( to, from, sizeof( LWFMatrix3 ));
}


void copym4( LWFMatrix4 to, LWFMatrix4 from )
{
   memcpy( to, from, sizeof( LWFMatrix4 ));
}


float dot( LWFVector a, LWFVector b )
{
   return a[ 0 ] * b[ 0 ] + a[ 1 ] * b[ 1 ] + a[ 2 ] * b[ 2 ];
}


float lenSquared( LWFVector a )
{
   return dot( a, a );
}


float len( LWFVector a )
{
   return ( float ) sqrt( lenSquared( a ));
}


void neg( LWFVector a )
{
   a[ 0 ] = -a[ 0 ];
   a[ 1 ] = -a[ 1 ];
   a[ 2 ] = -a[ 2 ];
}


void scalev( LWFVector a, float s )
{
   a[ 0 ] *= s;
   a[ 1 ] *= s;
   a[ 2 ] *= s;
}


void setlen( LWFVector a, float d )
{
   float d0 = len( a );

   if ( d0 != 0.0f ) scalev( a, d / d0 );
}


void normalize( LWFVector a )
{
   setlen( a, 1.0f );
}


void add( LWFVector a, LWFVector b, LWFVector c )
{
   c[ 0 ] = a[ 0 ] + b[ 0 ];
   c[ 1 ] = a[ 1 ] + b[ 1 ];
   c[ 2 ] = a[ 2 ] + b[ 2 ];
}


void sub( LWFVector a, LWFVector b, LWFVector c )
{
   c[ 0 ] = a[ 0 ] - b[ 0 ];
   c[ 1 ] = a[ 1 ] - b[ 1 ];
   c[ 2 ] = a[ 2 ] - b[ 2 ];
}


void mul( LWFVector a, LWFVector b, LWFVector c )
{
   c[ 0 ] = a[ 0 ] * b[ 0 ];
   c[ 1 ] = a[ 1 ] * b[ 1 ];
   c[ 2 ] = a[ 2 ] * b[ 2 ];
}


void lerp( LWFVector a, LWFVector b, float t, LWFVector c )
{
   c[ 0 ] = a[ 0 ] + ( b[ 0 ] - a[ 0 ] ) * t;
   c[ 1 ] = a[ 1 ] + ( b[ 1 ] - a[ 1 ] ) * t;
   c[ 2 ] = a[ 2 ] + ( b[ 2 ] - a[ 2 ] ) * t;
}


void combine( LWFVector a, LWFVector b, LWFVector c, float sa, float sb )
{
   c[ 0 ] = sa * a[ 0 ] + sb * b[ 0 ];
   c[ 1 ] = sa * a[ 1 ] + sb * b[ 1 ];
   c[ 2 ] = sa * a[ 2 ] + sb * b[ 2 ];
}


void cross( LWFVector a, LWFVector b, LWFVector c )
{
   c[ 0 ] = a[ 1 ] * b[ 2 ] - a[ 2 ] * b[ 1 ];
   c[ 1 ] = a[ 2 ] * b[ 0 ] - a[ 0 ] * b[ 2 ];
   c[ 2 ] = a[ 0 ] * b[ 1 ] - a[ 1 ] * b[ 0 ];
}


void polynorm( LWFVector v1, LWFVector v2, LWFVector vlast, LWFVector norm )
{
   LWFVector a, b;

   sub( v2, v1, a );
   sub( vlast, v1, b );
   cross( a, b, norm );
   normalize( norm );
}


float dist( LWFVector a, LWFVector b )
{
   LWFVector c;

   sub( a, b, c );
   return len( c );
}


float angle( LWFVector a, LWFVector b )
{
   LWFVector na, nb;

   copyv( na, a );
   normalize( na );
   copyv( nb, b );
   normalize( nb );
   return ( float ) acos( dot( na, nb ));
}


void vec_hp( LWFVector a, float *h, float *p )
{
   LWFVector n;

   copyv( n, a );
   normalize( n );

   *p = ( float ) asin( -n[ 1 ] );
   if ( 1.0f - fabs( n[ 1 ] ) > EPSILON_F ) {
      *h = ( float )( acos( n[ 2 ] / cos( *p )));
      if ( n[ 0 ] < 0.0f )
         *h = ( float )( 2 * PI - *h );
   }
   else *h = 0.0f;
}


void hp_vec( float h, float p, LWFVector a )
{
   float
      ch = ( float ) cos( h ),
      sh = ( float ) sin( h ),
      cp = ( float ) cos( p ),
      sp = ( float ) sin( p );

   a[ 0 ] = sh * cp;
   a[ 1 ] = -sp;
   a[ 2 ] = ch * cp;
}


void transform( LWFVector a, LWFMatrix3 m, LWFVector b )
{
   int i;

   for ( i = 0; i < 3; i++ )
      b[ i ] = a[ 0 ] * m[ 0 ][ i ]
             + a[ 1 ] * m[ 1 ][ i ]
             + a[ 2 ] * m[ 2 ][ i ];
}


void transformp( LWFVector a, LWFMatrix4 m, LWFVector b )
{
   float w;
   int i;

   for ( i = 0; i < 3; i++ )
      b[ i ] = a[ 0 ] * m[ 0 ][ i ]
             + a[ 1 ] * m[ 1 ][ i ]
             + a[ 2 ] * m[ 2 ][ i ] + m[ 3 ][ i ];

   w = a[ 0 ] * m[ 0 ][ 3 ]
     + a[ 1 ] * m[ 1 ][ 3 ]
     + a[ 2 ] * m[ 2 ][ 3 ] + m[ 3 ][ 3 ];

   if ( w != 0.0f ) {
      b[ 0 ] /= w;
      b[ 1 ] /= w;
      b[ 2 ] /= w;
   }
}


void matmul3( LWFMatrix3 a, LWFMatrix3 b, LWFMatrix3 c )
{
   int i, j, k;
   LWFMatrix3 m = { 0.0f };

   for ( i = 0; i < 3; i++ )
      for ( j = 0; j < 3; j++ )
         for ( k = 0; k < 3; k++ )
            m[ i ][ j ] += a[ i ][ k ] * b[ k ][ j ];

   memcpy( &c[ 0 ][ 0 ], &m[ 0 ][ 0 ], sizeof( LWFMatrix3 ));
}


void matmul4( LWFMatrix4 a, LWFMatrix4 b, LWFMatrix4 c )
{
   int i, j, k;
   LWFMatrix4 m = { 0.0f };

   for ( i = 0; i < 4; i++ )
      for ( j = 0; j < 4; j++ )
         for ( k = 0; k < 4; k++ )
            m[ i ][ j ] += a[ i ][ k ] * b[ k ][ j ];

   memcpy( &c[ 0 ][ 0 ], &m[ 0 ][ 0 ], sizeof( LWFMatrix4 ));
}


void scalem4( LWFMatrix4 a, float s )
{
   int i, j;

   for ( i = 0; i < 4; i++ )
      for ( j = 0; j < 4; j++ )
         a[ i ][ j ] *= s;
}


void scalem3( LWFMatrix3 a, float s )
{
   int i, j;

   for ( i = 0; i < 3; i++ )
      for ( j = 0; j < 3; j++ )
         a[ i ][ j ] *= s;
}


float det2( float a, float b, float c, float d )
{
    return a * d - b * c;
}


float det3( LWFMatrix3 m )
{
   return   m00 * det2( m11, m21, m12, m22 )
          - m01 * det2( m10, m20, m12, m22 )
          + m02 * det2( m10, m20, m11, m21 );
}


float det4( LWFMatrix4 m )
{
   LWFMatrix3 p;
   float a, b, c, d;

   initm3( p, m11, m21, m31, m12, m22, m32, m13, m23, m33 );  a = det3( p );
   initm3( p, m10, m20, m30, m12, m22, m32, m13, m23, m33 );  b = det3( p );
   initm3( p, m10, m20, m30, m11, m21, m31, m13, m23, m33 );  c = det3( p );
   initm3( p, m10, m20, m30, m11, m21, m31, m12, m22, m32 );  d = det3( p );

   return m00 * a - m01 * b + m02 * c - m03 * d;
}


void adjoint3( LWFMatrix3 m, LWFMatrix3 adj )
{
   adj[ 0 ][ 0 ] =  det2( m11, m12, m21, m22 );
   adj[ 1 ][ 0 ] = -det2( m10, m12, m20, m22 );
   adj[ 2 ][ 0 ] =  det2( m10, m11, m20, m21 );
   adj[ 0 ][ 1 ] = -det2( m01, m02, m21, m22 );
   adj[ 1 ][ 1 ] =  det2( m00, m02, m20, m22 );
   adj[ 2 ][ 1 ] = -det2( m00, m01, m20, m21 );
   adj[ 0 ][ 2 ] =  det2( m01, m02, m11, m12 );
   adj[ 1 ][ 2 ] = -det2( m00, m02, m10, m12 );
   adj[ 2 ][ 2 ] =  det2( m00, m01, m10, m11 );
}


void adjoint4( LWFMatrix4 m, LWFMatrix4 adj )
{
   LWFMatrix3 a;

   initm3( a, m11, m21, m31, m12, m22, m32, m13, m23, m33 );  adj[ 0 ][ 0 ] =  det3( a );
   initm3( a, m10, m20, m30, m12, m22, m32, m13, m23, m33 );  adj[ 1 ][ 0 ] = -det3( a );
   initm3( a, m10, m20, m30, m11, m21, m31, m13, m23, m33 );  adj[ 2 ][ 0 ] =  det3( a );
   initm3( a, m10, m20, m30, m11, m21, m31, m12, m22, m32 );  adj[ 3 ][ 0 ] = -det3( a );
   initm3( a, m01, m21, m31, m02, m22, m32, m03, m23, m33 );  adj[ 0 ][ 1 ] = -det3( a );
   initm3( a, m00, m20, m30, m02, m22, m32, m03, m23, m33 );  adj[ 1 ][ 1 ] =  det3( a );
   initm3( a, m00, m20, m30, m01, m21, m31, m03, m23, m33 );  adj[ 2 ][ 1 ] = -det3( a );
   initm3( a, m00, m20, m30, m01, m21, m31, m02, m22, m32 );  adj[ 3 ][ 1 ] =  det3( a );
   initm3( a, m01, m11, m31, m02, m12, m32, m03, m13, m33 );  adj[ 0 ][ 2 ] =  det3( a );
   initm3( a, m00, m10, m30, m02, m12, m32, m03, m13, m33 );  adj[ 1 ][ 2 ] = -det3( a );
   initm3( a, m00, m10, m30, m01, m11, m31, m03, m13, m33 );  adj[ 2 ][ 2 ] =  det3( a );
   initm3( a, m00, m10, m30, m01, m11, m31, m02, m12, m32 );  adj[ 3 ][ 2 ] = -det3( a );
   initm3( a, m01, m11, m21, m02, m12, m22, m03, m13, m23 );  adj[ 0 ][ 3 ] = -det3( a );
   initm3( a, m00, m10, m20, m02, m12, m22, m03, m13, m23 );  adj[ 1 ][ 3 ] =  det3( a );
   initm3( a, m00, m10, m20, m01, m11, m21, m03, m13, m23 );  adj[ 2 ][ 3 ] = -det3( a );
   initm3( a, m00, m10, m20, m01, m11, m21, m02, m12, m22 );  adj[ 3 ][ 3 ] =  det3( a );
}


void inverse3( LWFMatrix3 m, LWFMatrix3 inv )
{
   float det;

   det = det3( m );
   if ( fabs( det ) < EPSILON_F ) return;

   adjoint3( m, inv );
   scalem3( inv, 1.0f / det );
}


void inverse4( LWFMatrix4 m, LWFMatrix4 inv )
{
   float det;

   det = det4( m );
   if ( fabs( det ) < EPSILON_F ) return;

   adjoint4( m, inv );
   scalem4( inv, 1.0f / det );
}


static int nxt[] = { 1, 2, 0 };


void mat4_quat( LWFMatrix4 m, float q[ 4 ] )
{
   float tr, s;
   int i, j, k;

   tr = m00 + m11 + m22;
   if ( tr > 0.0f ) {
      s = ( float ) sqrt( tr + 1.0 );
      q[ 3 ] = s * 0.5f;
      s = 0.5f / s;
      q[ 0 ] = ( m12 - m21 ) * s;
      q[ 1 ] = ( m20 - m02 ) * s;
      q[ 2 ] = ( m01 - m10 ) * s;
   }
   else {
      i = 0;
      if ( m11 > m00 ) i = 1;
      if ( m22 > m[ i ][ i ] ) i = 2;
      j = nxt[ i ];
      k = nxt[ j ];
      s = ( float ) sqrt(( m[ i ][ i ] - ( m[ j ][ j ] + m[ k ][ k ] )) + 1.0 );
      q[ i ] = s * 0.5f;
      s = 0.5f / s;
      q[ 3 ] = ( m[ j ][ k ] - m[ k ][ j ] ) * s;
      q[ j ] = ( m[ i ][ j ] + m[ j ][ i ] ) * s;
      q[ k ] = ( m[ i ][ k ] + m[ k ][ i ] ) * s;
   }
}


void quat_mat4( float q[ 4 ], LWFMatrix4 m )
{
   float s, xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

   s = 2.0f / ( q[ 0 ] * q[ 0 ]
              + q[ 1 ] * q[ 1 ]
              + q[ 2 ] * q[ 2 ]
              + q[ 3 ] * q[ 3 ] );

   xs = q[ 0 ] * s;
   ys = q[ 1 ] * s;
   zs = q[ 2 ] * s;
   wx = q[ 3 ] * xs;
   wy = q[ 3 ] * ys;
   wz = q[ 3 ] * zs;
   xx = q[ 0 ] * xs;
   xy = q[ 0 ] * ys;
   xz = q[ 0 ] * zs;
   yy = q[ 1 ] * ys;
   yz = q[ 1 ] * zs;
   zz = q[ 2 ] * zs;

   m00 = 1.0f - ( yy + zz );
   m01 = xy + wz;
   m02 = xz - wy;

   m10 = xy - wz;
   m11 = 1.0f - ( xx + zz );
   m12 = yz + wx;

   m20 = xz + wy;
   m21 = yz - wx;
   m22 = 1.0f - ( xx + yy );

   m03 = m13 = m23 = m30 = m31 = m32 = 0.0f;
   m33 = 1.0f;
}


/* spherical linear interpolation between 2 quaternions */

void slerp( float p[ 4 ], float q[ 4 ], float t, float qt[ 4 ] )
{
   float omega, cosom, sinom, sclp, sclq;
   int i;

   cosom = p[ 0 ] * q[ 0 ]
         + p[ 1 ] * q[ 1 ]
         + p[ 2 ] * q[ 2 ]
         + p[ 3 ] * q[ 3 ];

   if (( 1.0f + cosom ) > EPSILON_F ) {
      if (( 1.0f - cosom ) > EPSILON_F ) {
         omega = ( float ) acos( cosom );
         sinom = ( float ) sin( omega );
         sclp  = ( float )( sin(( 1.0 - t ) * omega ) / sinom );
         sclq  = ( float )( sin( t * omega ) / sinom );
      }
      else {
         sclp = 1.0f - t;
         sclq = t;
      }
      for ( i = 0; i < 4; i++ )
         qt[ i ] = sclp * p[ i ] + sclq * q[ i ];
   }
   else {
      qt[ 0 ] = -p[ 1 ];
      qt[ 1 ] =  p[ 0 ];
      qt[ 2 ] = -p[ 3 ];
      qt[ 3 ] =  p[ 2 ];
      sclp = ( float ) sin(( 1.0 - t ) * HALFPI );
      sclq = ( float ) sin( t * HALFPI );
      for ( i = 0; i < 3; i++ )
         qt[ i ] = sclp * p[ i ] + sclq * qt[ i ];
   }
}


/*
======================================================================
Double precision functions.
====================================================================== */

void dinitv( LWDVector a, double x, double y, double z )
{
   a[ 0 ] = x;
   a[ 1 ] = y;
   a[ 2 ] = z;
}


void dinitm3( LWDMatrix3 m,
   double a1, double b1, double c1,
   double a2, double b2, double c2,
   double a3, double b3, double c3 )
{
   m00 = a1;  m01 = b1;  m02 = c1;
   m10 = a2;  m11 = b2;  m12 = c2;
   m20 = a3;  m21 = b3;  m22 = c3;
}


void dinitm4( LWDMatrix4 m,
   double a1, double b1, double c1, double d1,
   double a2, double b2, double c2, double d2,
   double a3, double b3, double c3, double d3,
   double a4, double b4, double c4, double d4 )
{
   m00 = a1;  m01 = b1;  m02 = c1;  m03 = d1;
   m10 = a2;  m11 = b2;  m12 = c2;  m13 = d2;
   m20 = a3;  m21 = b3;  m22 = c3;  m23 = d3;
   m30 = a4;  m31 = b4;  m32 = c4;  m33 = d4;
}


void didentity3( LWDMatrix3 m )
{
   dinitm3( m,
      1.0, 0.0, 0.0,
      0.0, 1.0, 0.0,
      0.0, 0.0, 1.0 );
}


void didentity4( LWDMatrix4 m )
{
   dinitm4( m,
      1.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 1.0, 0.0,
      0.0, 0.0, 0.0, 1.0 );
}


void dcopyv( LWDVector to, LWDVector from )
{
   to[ 0 ] = from[ 0 ];
   to[ 1 ] = from[ 1 ];
   to[ 2 ] = from[ 2 ];
}


void dcopym3( LWDMatrix3 to, LWDMatrix3 from )
{
   memcpy( to, from, sizeof( LWDMatrix3 ));
}


void dcopym4( LWDMatrix4 to, LWDMatrix4 from )
{
   memcpy( to, from, sizeof( LWDMatrix4 ));
}


double ddot( LWDVector a, LWDVector b )
{
   return a[ 0 ] * b[ 0 ] + a[ 1 ] * b[ 1 ] + a[ 2 ] * b[ 2 ];
}


double dlenSquared( LWDVector a )
{
   return ddot( a, a );
}


double dlen( LWDVector a )
{
   return ( double ) sqrt( dlenSquared( a ));
}


void dneg( LWDVector a )
{
   a[ 0 ] = -a[ 0 ];
   a[ 1 ] = -a[ 1 ];
   a[ 2 ] = -a[ 2 ];
}


void dscalev( LWDVector a, double s )
{
   a[ 0 ] *= s;
   a[ 1 ] *= s;
   a[ 2 ] *= s;
}


void dsetlen( LWDVector a, double d )
{
   double d0 = dlen( a );

   if ( d0 != 0.0 ) dscalev( a, d / d0 );
}


void dnormalize( LWDVector a )
{
   dsetlen( a, 1.0 );
}


void dadd( LWDVector a, LWDVector b, LWDVector c )
{
   c[ 0 ] = a[ 0 ] + b[ 0 ];
   c[ 1 ] = a[ 1 ] + b[ 1 ];
   c[ 2 ] = a[ 2 ] + b[ 2 ];
}


void dsub( LWDVector a, LWDVector b, LWDVector c )
{
   c[ 0 ] = a[ 0 ] - b[ 0 ];
   c[ 1 ] = a[ 1 ] - b[ 1 ];
   c[ 2 ] = a[ 2 ] - b[ 2 ];
}


void dmul( LWDVector a, LWDVector b, LWDVector c )
{
   c[ 0 ] = a[ 0 ] * b[ 0 ];
   c[ 1 ] = a[ 1 ] * b[ 1 ];
   c[ 2 ] = a[ 2 ] * b[ 2 ];
}


void dlerp( LWDVector a, LWDVector b, double t, LWDVector c )
{
   c[ 0 ] = a[ 0 ] + ( b[ 0 ] - a[ 0 ] ) * t;
   c[ 1 ] = a[ 1 ] + ( b[ 1 ] - a[ 1 ] ) * t;
   c[ 2 ] = a[ 2 ] + ( b[ 2 ] - a[ 2 ] ) * t;
}


void dcombine( LWDVector a, LWDVector b, LWDVector c, double sa, double sb )
{
   c[ 0 ] = sa * a[ 0 ] + sb * b[ 0 ];
   c[ 1 ] = sa * a[ 1 ] + sb * b[ 1 ];
   c[ 2 ] = sa * a[ 2 ] + sb * b[ 2 ];
}


void dcross( LWDVector a, LWDVector b, LWDVector c )
{
   c[ 0 ] = a[ 1 ] * b[ 2 ] - a[ 2 ] * b[ 1 ];
   c[ 1 ] = a[ 2 ] * b[ 0 ] - a[ 0 ] * b[ 2 ];
   c[ 2 ] = a[ 0 ] * b[ 1 ] - a[ 1 ] * b[ 0 ];
}


void dpolynorm( LWDVector v1, LWDVector v2, LWDVector vlast, LWDVector norm )
{
   LWDVector a, b;

   dsub( v2, v1, a );
   dsub( vlast, v1, b );
   dcross( a, b, norm );
   dnormalize( norm );
}


double ddist( LWDVector a, LWDVector b )
{
   LWDVector c;

   dsub( a, b, c );
   return dlen( c );
}


double dangle( LWDVector a, LWDVector b )
{
   LWDVector na, nb;

   dcopyv( na, a );
   dnormalize( na );
   dcopyv( nb, b );
   dnormalize( nb );
   return acos( ddot( na, nb ));
}


void dvec_hp( LWDVector a, double *h, double *p )
{
   LWDVector n;

   dcopyv( n, a );
   dnormalize( n );

   *p = ( double ) asin( -n[ 1 ] );
   if ( 1.0 - fabs( n[ 1 ] ) > EPSILON_D ) {
      *h = acos( n[ 2 ] / cos( *p ));
      if ( n[ 0 ] < 0.0 )
         *h = 2 * PI - *h;
   }
   else *h = 0.0;
}


void dhp_vec( double h, double p, LWDVector a )
{
   double
      ch = cos( h ),
      sh = sin( h ),
      cp = cos( p ),
      sp = sin( p );

   a[ 0 ] = sh * cp;
   a[ 1 ] = -sp;
   a[ 2 ] = ch * cp;
}


void dtransform( LWDVector a, LWDMatrix3 m, LWDVector b )
{
   int i;

   for ( i = 0; i < 3; i++ )
      b[ i ] = a[ 0 ] * m[ 0 ][ i ]
             + a[ 1 ] * m[ 1 ][ i ]
             + a[ 2 ] * m[ 2 ][ i ];
}


void dtransformp( LWDVector a, LWDMatrix4 m, LWDVector b )
{
   double w;
   int i;

   for ( i = 0; i < 3; i++ )
      b[ i ] = a[ 0 ] * m[ 0 ][ i ]
             + a[ 1 ] * m[ 1 ][ i ]
             + a[ 2 ] * m[ 2 ][ i ] + m[ 3 ][ i ];

   w = a[ 0 ] * m[ 0 ][ 3 ]
     + a[ 1 ] * m[ 1 ][ 3 ]
     + a[ 2 ] * m[ 2 ][ 3 ] + m[ 3 ][ 3 ];

   if ( w != 0.0 ) {
      b[ 0 ] /= w;
      b[ 1 ] /= w;
      b[ 2 ] /= w;
   }
}


void dmatmul3( LWDMatrix3 a, LWDMatrix3 b, LWDMatrix3 c )
{
   int i, j, k;
   LWDMatrix3 m = { 0.0 };

   for ( i = 0; i < 3; i++ )
      for ( j = 0; j < 3; j++ )
         for ( k = 0; k < 3; k++ )
            m[ i ][ j ] += a[ i ][ k ] * b[ k ][ j ];

   memcpy( &c[ 0 ][ 0 ], &m[ 0 ][ 0 ], sizeof( LWDMatrix3 ));
}


void dmatmul4( LWDMatrix4 a, LWDMatrix4 b, LWDMatrix4 c )
{
   int i, j, k;
   LWDMatrix4 m = { 0.0 };

   for ( i = 0; i < 4; i++ )
      for ( j = 0; j < 4; j++ )
         for ( k = 0; k < 4; k++ )
            m[ i ][ j ] += a[ i ][ k ] * b[ k ][ j ];

   memcpy( &c[ 0 ][ 0 ], &m[ 0 ][ 0 ], sizeof( LWDMatrix4 ));
}


void dscalem4( LWDMatrix4 a, double s )
{
   int i, j;

   for ( i = 0; i < 4; i++ )
      for ( j = 0; j < 4; j++ )
         a[ i ][ j ] *= s;
}


void dscalem3( LWDMatrix3 a, double s )
{
   int i, j;

   for ( i = 0; i < 3; i++ )
      for ( j = 0; j < 3; j++ )
         a[ i ][ j ] *= s;
}


double ddet2( double a, double b, double c, double d )
{
    return a * d - b * c;
}


double ddet3( LWDMatrix3 m )
{
   return   m00 * ddet2( m11, m21, m12, m22 )
          - m01 * ddet2( m10, m20, m12, m22 )
          + m02 * ddet2( m10, m20, m11, m21 );
}


double ddet4( LWDMatrix4 m )
{
   LWDMatrix3 p;
   double a, b, c, d;

   dinitm3( p, m11, m21, m31, m12, m22, m32, m13, m23, m33 );  a = ddet3( p );
   dinitm3( p, m10, m20, m30, m12, m22, m32, m13, m23, m33 );  b = ddet3( p );
   dinitm3( p, m10, m20, m30, m11, m21, m31, m13, m23, m33 );  c = ddet3( p );
   dinitm3( p, m10, m20, m30, m11, m21, m31, m12, m22, m32 );  d = ddet3( p );

   return m00 * a - m01 * b + m02 * c - m03 * d;
}


void dadjoint3( LWDMatrix3 m, LWDMatrix3 adj )
{
   adj[ 0 ][ 0 ] =  ddet2( m11, m12, m21, m22 );
   adj[ 1 ][ 0 ] = -ddet2( m10, m12, m20, m22 );
   adj[ 2 ][ 0 ] =  ddet2( m10, m11, m20, m21 );
   adj[ 0 ][ 1 ] = -ddet2( m01, m02, m21, m22 );
   adj[ 1 ][ 1 ] =  ddet2( m00, m02, m20, m22 );
   adj[ 2 ][ 1 ] = -ddet2( m00, m01, m20, m21 );
   adj[ 0 ][ 2 ] =  ddet2( m01, m02, m11, m12 );
   adj[ 1 ][ 2 ] = -ddet2( m00, m02, m10, m12 );
   adj[ 2 ][ 2 ] =  ddet2( m00, m01, m10, m11 );
}


void dadjoint4( LWDMatrix4 m, LWDMatrix4 adj )
{
   LWDMatrix3 a;

   dinitm3( a, m11, m21, m31, m12, m22, m32, m13, m23, m33 );  adj[ 0 ][ 0 ] =  ddet3( a );
   dinitm3( a, m10, m20, m30, m12, m22, m32, m13, m23, m33 );  adj[ 1 ][ 0 ] = -ddet3( a );
   dinitm3( a, m10, m20, m30, m11, m21, m31, m13, m23, m33 );  adj[ 2 ][ 0 ] =  ddet3( a );
   dinitm3( a, m10, m20, m30, m11, m21, m31, m12, m22, m32 );  adj[ 3 ][ 0 ] = -ddet3( a );
   dinitm3( a, m01, m21, m31, m02, m22, m32, m03, m23, m33 );  adj[ 0 ][ 1 ] = -ddet3( a );
   dinitm3( a, m00, m20, m30, m02, m22, m32, m03, m23, m33 );  adj[ 1 ][ 1 ] =  ddet3( a );
   dinitm3( a, m00, m20, m30, m01, m21, m31, m03, m23, m33 );  adj[ 2 ][ 1 ] = -ddet3( a );
   dinitm3( a, m00, m20, m30, m01, m21, m31, m02, m22, m32 );  adj[ 3 ][ 1 ] =  ddet3( a );
   dinitm3( a, m01, m11, m31, m02, m12, m32, m03, m13, m33 );  adj[ 0 ][ 2 ] =  ddet3( a );
   dinitm3( a, m00, m10, m30, m02, m12, m32, m03, m13, m33 );  adj[ 1 ][ 2 ] = -ddet3( a );
   dinitm3( a, m00, m10, m30, m01, m11, m31, m03, m13, m33 );  adj[ 2 ][ 2 ] =  ddet3( a );
   dinitm3( a, m00, m10, m30, m01, m11, m31, m02, m12, m32 );  adj[ 3 ][ 2 ] = -ddet3( a );
   dinitm3( a, m01, m11, m21, m02, m12, m22, m03, m13, m23 );  adj[ 0 ][ 3 ] = -ddet3( a );
   dinitm3( a, m00, m10, m20, m02, m12, m22, m03, m13, m23 );  adj[ 1 ][ 3 ] =  ddet3( a );
   dinitm3( a, m00, m10, m20, m01, m11, m21, m03, m13, m23 );  adj[ 2 ][ 3 ] = -ddet3( a );
   dinitm3( a, m00, m10, m20, m01, m11, m21, m02, m12, m22 );  adj[ 3 ][ 3 ] =  ddet3( a );
}


void dinverse3( LWDMatrix3 m, LWDMatrix3 inv )
{
   double det;

   det = ddet3( m );
   if ( fabs( det ) < EPSILON_D ) return;

   dadjoint3( m, inv );
   dscalem3( inv, 1.0 / det );
}


void dinverse4( LWDMatrix4 m, LWDMatrix4 inv )
{
   double det;

   det = ddet4( m );
   if ( fabs( det ) < EPSILON_D ) return;

   dadjoint4( m, inv );
   dscalem4( inv, 1.0 / det );
}


void dmat4_quat( LWDMatrix4 m, double q[ 4 ] )
{
   double tr, s;
   int i, j, k;

   tr = m00 + m11 + m22;
   if ( tr > 0.0 ) {
      s = sqrt( tr + 1.0 );
      q[ 3 ] = s * 0.5;
      s = 0.5 / s;
      q[ 0 ] = ( m12 - m21 ) * s;
      q[ 1 ] = ( m20 - m02 ) * s;
      q[ 2 ] = ( m01 - m10 ) * s;
   }
   else {
      i = 0;
      if ( m11 > m00 ) i = 1;
      if ( m22 > m[ i ][ i ] ) i = 2;
      j = nxt[ i ];
      k = nxt[ j ];
      s = sqrt(( m[ i ][ i ] - ( m[ j ][ j ] + m[ k ][ k ] )) + 1.0 );
      q[ i ] = s * 0.5;
      s = 0.5 / s;
      q[ 3 ] = ( m[ j ][ k ] - m[ k ][ j ] ) * s;
      q[ j ] = ( m[ i ][ j ] + m[ j ][ i ] ) * s;
      q[ k ] = ( m[ i ][ k ] + m[ k ][ i ] ) * s;
   }
}


void dquat_mat4( double q[ 4 ], LWDMatrix4 m )
{
   double s, xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

   s = 2.0 / ( q[ 0 ] * q[ 0 ]
             + q[ 1 ] * q[ 1 ]
             + q[ 2 ] * q[ 2 ]
             + q[ 3 ] * q[ 3 ] );

   xs = q[ 0 ] * s;
   ys = q[ 1 ] * s;
   zs = q[ 2 ] * s;
   wx = q[ 3 ] * xs;
   wy = q[ 3 ] * ys;
   wz = q[ 3 ] * zs;
   xx = q[ 0 ] * xs;
   xy = q[ 0 ] * ys;
   xz = q[ 0 ] * zs;
   yy = q[ 1 ] * ys;
   yz = q[ 1 ] * zs;
   zz = q[ 2 ] * zs;

   m00 = 1.0 - ( yy + zz );
   m01 = xy + wz;
   m02 = xz - wy;

   m10 = xy - wz;
   m11 = 1.0 - ( xx + zz );
   m12 = yz + wx;

   m20 = xz + wy;
   m21 = yz - wx;
   m22 = 1.0 - ( xx + yy );

   m03 = m13 = m23 = m30 = m31 = m32 = 0.0;
   m33 = 1.0;
}


/* spherical linear interpolation between 2 quaternions */

void dslerp( double p[ 4 ], double q[ 4 ], double t, double qt[ 4 ] )
{
   double omega, cosom, sinom, sclp, sclq;
   int i;

   cosom = p[ 0 ] * q[ 0 ]
         + p[ 1 ] * q[ 1 ]
         + p[ 2 ] * q[ 2 ]
         + p[ 3 ] * q[ 3 ];

   if (( 1.0 + cosom ) > EPSILON_D ) {
      if (( 1.0 - cosom ) > EPSILON_D ) {
         omega = acos( cosom );
         sinom = sin( omega );
         sclp  = sin(( 1.0 - t ) * omega ) / sinom;
         sclq  = sin( t * omega ) / sinom;
      }
      else {
         sclp = 1.0 - t;
         sclq = t;
      }
      for ( i = 0; i < 4; i++ )
         qt[ i ] = sclp * p[ i ] + sclq * q[ i ];
   }
   else {
      qt[ 0 ] = -p[ 1 ];
      qt[ 1 ] =  p[ 0 ];
      qt[ 2 ] = -p[ 3 ];
      qt[ 3 ] =  p[ 2 ];
      sclp = sin(( 1.0 - t ) * HALFPI );
      sclq = sin( t * HALFPI );
      for ( i = 0; i < 3; i++ )
         qt[ i ] = sclp * p[ i ] + sclq * qt[ i ];
   }
}


LWFVecMathFuncs ffunc = {
   initv, initm3, initm4, identity3, identity4, copyv, copym3, copym4,
   dot, lenSquared, len, scalev, setlen, normalize, neg, add, sub, mul,
   lerp, combine, cross, polynorm, dist, angle, vec_hp, hp_vec, transform,
   transformp, matmul3, matmul4, scalem4, scalem3, det2, det3, det4,
   adjoint3, adjoint4, inverse3, inverse4, mat4_quat, quat_mat4, slerp
};

LWDVecMathFuncs dfunc = {
   dinitv, dinitm3, dinitm4, didentity3, didentity4, dcopyv, dcopym3, dcopym4,
   ddot, dlenSquared, dlen, dscalev, dsetlen, dnormalize, dneg, dadd, dsub, dmul,
   dlerp, dcombine, dcross, dpolynorm, ddist, dangle, dvec_hp, dhp_vec, dtransform,
   dtransformp, dmatmul3, dmatmul4, dscalem4, dscalem3, ddet2, ddet3, ddet4,
   dadjoint3, dadjoint4, dinverse3, dinverse4, dmat4_quat, dquat_mat4, dslerp
};


XCALL_( int )
VecMath( long version, GlobalFunc *global, LWGlobalService *local,
   void *serverData)
{
   if ( version != LWGLOBALSERVICE_VERSION ) return AFUNC_BADVERSION;

   if ( !strcmp( local->id, LWFVECMATHFUNCS_GLOBAL ))
      local->data = &ffunc;

   else if ( !strcmp( local->id, LWDVECMATHFUNCS_GLOBAL ))
      local->data = &dfunc;

   else
      local->data = NULL;

   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { LWGLOBALSERVICE_CLASS, LWFVECMATHFUNCS_GLOBAL, VecMath },
   { LWGLOBALSERVICE_CLASS, LWDVECMATHFUNCS_GLOBAL, VecMath },
   { NULL }
};
