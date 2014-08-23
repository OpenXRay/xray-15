/*
======================================================================
vecmath.h

Header file for the vector math Global class sample plug-in.

Ernie Wright  19 Mar 00

Include this header in code that uses the vector math global.  See the
comments in vecmath.c for details.
====================================================================== */

#ifndef VECMATH_H
#define VECMATH_H

#include <lwtypes.h>


#define LWFVECMATHFUNCS_GLOBAL "LWFVecMathFunctions"

typedef float LWFMatrix3[ 3 ][ 3 ];
typedef float LWFMatrix4[ 4 ][ 4 ];

typedef struct st_LWFVecMathFuncs {
   void   ( *initv )     ( LWFVector a, float x, float y, float z );
   void   ( *initm3 )    ( LWFMatrix3 m, float, float, float, float,
                             float, float, float, float, float );
   void   ( *initm4 )    ( LWFMatrix4 m, float, float, float, float,
                             float, float, float, float, float, float,
                             float, float, float, float, float, float );
   void   ( *identity3 ) ( LWFMatrix3 m );
   void   ( *identity4 ) ( LWFMatrix4 m );
   void   ( *copyv )     ( LWFVector to, LWFVector from );
   void   ( *copym3 )    ( LWFMatrix3 to, LWFMatrix3 from );
   void   ( *copym4 )    ( LWFMatrix4 to, LWFMatrix4 from );
   float  ( *dot )       ( LWFVector a, LWFVector b );
   float  ( *lenSquared )( LWFVector a );
   float  ( *len )       ( LWFVector a );
   void   ( *scalev )    ( LWFVector a, float s );
   void   ( *setlen )    ( LWFVector a, float d );
   void   ( *normalize ) ( LWFVector a );
   void   ( *neg )       ( LWFVector a );
   void   ( *add )       ( LWFVector a, LWFVector b, LWFVector c );
   void   ( *sub )       ( LWFVector a, LWFVector b, LWFVector c );
   void   ( *mul )       ( LWFVector a, LWFVector b, LWFVector c );
   void   ( *lerp )      ( LWFVector a, LWFVector b, float t, LWFVector c );
   void   ( *combine )   ( LWFVector a, LWFVector b, LWFVector c, float sa, float sb );
   void   ( *cross )     ( LWFVector a, LWFVector b, LWFVector c );
   void   ( *polynorm )  ( LWFVector v1, LWFVector v2, LWFVector vlast, LWFVector norm );
   float  ( *dist )      ( LWFVector a, LWFVector b );
   float  ( *angle )     ( LWFVector a, LWFVector b );
   void   ( *vec_hp )    ( LWFVector a, float *h, float *p );
   void   ( *hp_vec )    ( float h, float p, LWFVector a );
   void   ( *transform ) ( LWFVector a, LWFMatrix3 m, LWFVector b );
   void   ( *transformp )( LWFVector a, LWFMatrix4 m, LWFVector b );
   void   ( *matmul3 )   ( LWFMatrix3 a, LWFMatrix3 b, LWFMatrix3 c );
   void   ( *matmul4 )   ( LWFMatrix4 a, LWFMatrix4 b, LWFMatrix4 c );
   void   ( *scalem4 )   ( LWFMatrix4 a, float s );
   void   ( *scalem3 )   ( LWFMatrix3 a, float s );
   float  ( *det2 )      ( float a, float b, float c, float d );
   float  ( *det3 )      ( LWFMatrix3 m );
   float  ( *det4 )      ( LWFMatrix4 m );
   void   ( *adjoint3 )  ( LWFMatrix3 m, LWFMatrix3 adj );
   void   ( *adjoint4 )  ( LWFMatrix4 m, LWFMatrix4 adj );
   void   ( *inverse3 )  ( LWFMatrix3 m, LWFMatrix3 inv );
   void   ( *inverse4 )  ( LWFMatrix4 m, LWFMatrix4 inv );
   void   ( *mat4_quat ) ( LWFMatrix4 m, float q[ 4 ] );
   void   ( *quat_mat4 ) ( float q[ 4 ], LWFMatrix4 m );
   void   ( *slerp )     ( float p[ 4 ], float q[ 4 ], float t, float qt[ 4 ] );
} LWFVecMathFuncs;


#define LWDVECMATHFUNCS_GLOBAL "LWDVecMathFunctions"

typedef double LWDMatrix3[ 3 ][ 3 ];
typedef double LWDMatrix4[ 4 ][ 4 ];

typedef struct st_LWDVecMathFuncs {
   void   ( *initv )     ( LWDVector a, double x, double y, double z );
   void   ( *initm3 )    ( LWDMatrix3 m, double, double, double, double,
                                 double, double, double, double, double );
   void   ( *initm4 )    ( LWDMatrix4 m, double, double, double, double,
                                 double, double, double, double, double, double,
                                 double, double, double, double, double, double );
   void   ( *identity3 ) ( LWDMatrix3 m );
   void   ( *identity4 ) ( LWDMatrix4 m );
   void   ( *copyv )     ( LWDVector to, LWDVector from );
   void   ( *copym3 )    ( LWDMatrix3 to, LWDMatrix3 from );
   void   ( *copym4 )    ( LWDMatrix4 to, LWDMatrix4 from );
   double ( *dot )       ( LWDVector a, LWDVector b );
   double ( *lenSquared )( LWDVector a );
   double ( *len )       ( LWDVector a );
   void   ( *scalev )    ( LWDVector a, double s );
   void   ( *setlen )    ( LWDVector a, double d );
   void   ( *normalize ) ( LWDVector a );
   void   ( *neg )       ( LWDVector a );
   void   ( *add )       ( LWDVector a, LWDVector b, LWDVector c );
   void   ( *sub )       ( LWDVector a, LWDVector b, LWDVector c );
   void   ( *mul )       ( LWDVector a, LWDVector b, LWDVector c );
   void   ( *lerp )      ( LWDVector a, LWDVector b, double t, LWDVector c );
   void   ( *combine )   ( LWDVector a, LWDVector b, LWDVector c, double sa, double sb );
   void   ( *cross )     ( LWDVector a, LWDVector b, LWDVector c );
   void   ( *polynorm )  ( LWDVector v1, LWDVector v2, LWDVector vlast, LWDVector norm );
   double ( *dist )      ( LWDVector a, LWDVector b );
   double ( *angle )     ( LWDVector a, LWDVector b );
   void   ( *vec_hp )    ( LWDVector a, double *h, double *p );
   void   ( *hp_vec )    ( double h, double p, LWDVector a );
   void   ( *transform ) ( LWDVector a, LWDMatrix3 m, LWDVector b );
   void   ( *transformp )( LWDVector a, LWDMatrix4 m, LWDVector b );
   void   ( *matmul3 )   ( LWDMatrix3 a, LWDMatrix3 b, LWDMatrix3 c );
   void   ( *matmul4 )   ( LWDMatrix4 a, LWDMatrix4 b, LWDMatrix4 c );
   void   ( *scalem4 )   ( LWDMatrix4 a, double s );
   void   ( *scalem3 )   ( LWDMatrix3 a, double s );
   double ( *det2 )      ( double a, double b, double c, double d );
   double ( *det3 )      ( LWDMatrix3 m );
   double ( *det4 )      ( LWDMatrix4 m );
   void   ( *adjoint3 )  ( LWDMatrix3 m, LWDMatrix3 adj );
   void   ( *adjoint4 )  ( LWDMatrix4 m, LWDMatrix4 adj );
   void   ( *inverse3 )  ( LWDMatrix3 m, LWDMatrix3 inv );
   void   ( *inverse4 )  ( LWDMatrix4 m, LWDMatrix4 inv );
   void   ( *mat4_quat ) ( LWDMatrix4 m, double q[ 4 ] );
   void   ( *quat_mat4 ) ( double q[ 4 ], LWDMatrix4 m );
   void   ( *slerp )     ( double p[ 4 ], double q[ 4 ], double t, double qt[ 4 ] );
} LWDVecMathFuncs;


#ifndef PI
#define PI     3.14159265358979323846264338328
#define HALFPI 1.57079632679489661923132169164
#endif

#define RADIANS( deg ) ((deg) * PI / 180.0)
#define DEGREES( rad ) ((rad) * 180.0 / PI)

#endif