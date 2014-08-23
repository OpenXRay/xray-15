/*
======================================================================
superq.c

Mesh edits for creating superquadrics.

Ernie Wright  28 Mar 00
Arnie Cachelin  21 Sep 00
====================================================================== */

#include <lwserver.h>
#include <lwmeshedt.h>
#include <lwmath.h>
#include <lwmodtool.h>
#include <stdlib.h>
#include <math.h>
#include "superq.h"

#define SIGN(x) ((x > 0) - (x < 0))

static int
   zax[] = { 1, 2, 0 },
   xax[] = { 2, 0, 1 },
   yax[] = { 0, 1, 2 };


/*
======================================================================
ellipsoid()

Create an ellipsoidal superquadric based on the sqData settings.
====================================================================== */

int ellipsoid( MeshEditOp *edit, sqData *dat )
{
   LWPntID *id;
   int i, j, k, npts, x, y, z;
   double u, v, su, sv, cu, cv, d, xv, yv, pt[ 3 ];
   float uv[ 2 ], puv[ 8 ];

   y = dat->axis;
   x = xax[ dat->axis ];
   z = zax[ dat->axis ];

   npts = 2 + dat->nsides * ( dat->nsegments - 1 );
   id = calloc( npts, sizeof( LWPntID ));
   if ( !id ) return 0;

   for ( j = 0, k = 0; j <= dat->nsegments; j++ ) {
      uv[ 1 ] = ( float ) j / dat->nsegments;
      v       = -PI / 2 + uv[ 1 ] * PI;
      sv      = sin( v );
      cv      = cos( v );
      d       = SIGN( cv ) * pow( fabs( cv ), 2 / dat->bf1 );
      xv      = dat->rad[ x ] * d;
      yv      = dat->rad[ z ] * d;
      pt[ y ] = dat->rad[ y ] * SIGN( sv ) * pow( fabs( sv ), 2 / dat->bf1 )
              + dat->org[ y ];

      for ( i = 0; i < dat->nsides; i++ ) {
         uv[ 0 ] = ( float ) i / dat->nsides;
         u       = -PI + 2 * PI * uv[ 0 ];
         su      = sin( u );
         cu      = cos( u );
         pt[ x ] = xv * SIGN( cu ) * pow( fabs( cu ), 2 / dat->bf2 ) + dat->org[ x ];
         pt[ z ] = yv * SIGN( su ) * pow( fabs( su ), 2 / dat->bf2 ) + dat->org[ z ];

         if ( dat->uvs )
            edit->initUV( edit->state, uv );

         id[ k++ ] = edit->addPoint( edit->state, pt );
         if ( j == 0 || j == dat->nsegments ) break;
      }
   }

   if ( dat->uvs ) {
      su = 1.0 / dat->nsides;
      sv = 1.0 / dat->nsegments;
      puv[ 0 ] = ( float ) su * 0.5f;
      puv[ 1 ] = 0.0f;
      puv[ 2 ] = 0.0f;
      puv[ 3 ] = ( float ) sv;
      puv[ 4 ] = ( float ) su;
      puv[ 5 ] = ( float ) sv;
   }

   for ( i = 1; i <= dat->nsides; i++ ) {
      if( dat->uvs )
         edit->initUV( edit->state, puv );
      edit->addTri( edit->state,
         id[ 0 ],
         id[ i ],
         id[ ( i % dat->nsides ) + 1 ] );
      if ( dat->uvs ) {
         puv[ 0 ] += ( float ) su;
         puv[ 2 ] += ( float ) su;
         puv[ 4 ] += ( float ) su;
      }
   }

   for ( j = 1, k = 1; j < dat->nsegments - 1; k += dat->nsides, j++ ) {
      for ( i = 0; i < dat->nsides; i++ ) {
         if ( dat->uvs ) {
            puv[ 0 ] = puv[ 2 ] = ( float ) su * i;
            puv[ 1 ] = puv[ 7 ] = ( float ) sv * j;
            puv[ 4 ] = puv[ 6 ] = puv[ 0 ] + ( float ) su;
            puv[ 3 ] = puv[ 5 ] = puv[ 1 ] + ( float ) sv;
            edit->initUV( edit->state, puv );
         }
         edit->addQuad( edit->state,
            id[ k + i ],
            id[ k + dat->nsides + i ],
            id[ k + dat->nsides + (( i + 1 ) % dat->nsides ) ],
            id[ k + (( i + 1 ) % dat->nsides ) ] );
      }
   }

   if ( dat->uvs ) {
      puv[ 0 ] = ( float ) su * 0.5f;
      puv[ 1 ] = 1.0f;
      puv[ 2 ] = 0.0f;
      puv[ 3 ] = 1.0f - ( float ) sv;
      puv[ 4 ] = ( float ) su;
      puv[ 5 ] = 1.0f - ( float ) sv;
   }

   for ( i = 0, k = npts - dat->nsides - 1; i < dat->nsides; i++ ) {
      if ( dat->uvs )
         edit->initUV( edit->state, puv );
      edit->addTri( edit->state,
         id[ npts - 1 ],
         id[ k + (( i + 1 ) % dat->nsides ) ],
         id[ k + i ] );
      if ( dat->uvs ) {
         puv[ 0 ] += ( float ) su;
         puv[ 2 ] += ( float ) su;
         puv[ 4 ] += ( float ) su;
      }
   }

   free( id );
   return npts;
}


/*
======================================================================
toroid()

Create a toroidal superquadric based on the sqData settings.
====================================================================== */

int toroid( MeshEditOp *edit, sqData *dat )
{
   LWPntID *id;
   int i, j, k, npts, x, y, z;
   double u, v, su, sv, cu, cv, d, xv, yv, pt[ 3 ];
   float uv[ 2 ], puv[ 8 ];

   y = dat->axis;
   x = xax[ dat->axis ];
   z = zax[ dat->axis ];

   npts = dat->nsides * dat->nsegments;
   id = calloc( npts, sizeof( LWPntID ));
   if ( !id ) return 0;

   for ( j = 0, k = 0; j < dat->nsegments; j++ ) {
      uv[ 1 ] = ( float ) j / dat->nsegments;
      v       = -PI + 2 * PI * uv[ 1 ];
      sv      = sin( v );
      cv      = cos( v );
      d       = dat->diam + SIGN( cv ) * pow( fabs( cv ), 2 / dat->bf1 );
      xv      = dat->rad[ x ] * d;
      yv      = dat->rad[ z ] * d;
      pt[ y ] = dat->rad[ y ] * SIGN( sv ) * pow( fabs( sv ), 2 / dat->bf1 )
              + dat->org[ y ];

      for ( i = 0; i < dat->nsides; i++ ) {
         uv[ 0 ] = ( float ) i / dat->nsides;
         u       = -PI + 2 * PI * uv[ 0 ];
         su      = sin( u );
         cu      = cos( u );
         pt[ x ] = xv * SIGN( cu ) * pow( fabs( cu ), 2 / dat->bf2 )
                 + dat->org[ x ];
         pt[ z ] = yv * SIGN( su ) * pow( fabs( su ), 2 / dat->bf2 )
                 + dat->org[ z ];

         if ( dat->uvs )
            edit->initUV( edit->state, uv );

         id[ k++ ] = edit->addPoint( edit->state, pt );
      }
   }

   if ( dat->uvs ) {
      su = 1.0 / dat->nsides;
      sv = 1.0 / dat->nsegments;
   }

   for ( j = 0, k = 0; j < dat->nsegments; j++, k += dat->nsides ) {
      for ( i = 0; i < dat->nsides; i++ ) {
         if ( dat->uvs ) {
            puv[ 0 ] = puv[ 2 ] = ( float ) su * i;
            puv[ 1 ] = puv[ 7 ] = ( float ) sv * j;
            puv[ 4 ] = puv[ 6 ] = puv[ 0 ] + ( float ) su;
            puv[ 3 ] = puv[ 5 ] = puv[ 1 ] + ( float ) sv;
            edit->initUV( edit->state, puv );
         }
         edit->addQuad( edit->state,
            id[ k + i ],
            id[ ( k + dat->nsides + i ) % npts ],
            id[ ( k + dat->nsides + (( i + 1 ) % dat->nsides )) % npts ],
            id[ k + (( i + 1 ) % dat->nsides ) ] );
      }
   }

   free( id );
   return npts;
}
