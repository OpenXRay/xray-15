/*
======================================================================
unwrap.c

Draw polygons in the UV coordinates of an image mapped texture layer.

Ernie Wright  21 Aug 01

The image produced by Unwrap can be used as a template for painting
the texture layer's image map.  The polygons of the selected surface
are drawn in wireframe, in the 2D coordinates defined by the mapping
parameters.
====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <lwserver.h>
#include <lwrender.h>
#include <lwsurf.h>
#include <lwtxtr.h>
#include <lwimage.h>
#include <lwhost.h>
#include <lwpanel.h>
#include <lwgeneric.h>
#include "unwrap.h"


LWItemInfo     *iteminfo;
LWObjectInfo   *objinfo;
LWImageList    *imglist;
LWImageUtil    *imgutil;
LWMessageFuncs *msgf;
LWSurfaceFuncs *surff;
LWTextureFuncs *txtrf;
LWPanelFuncs   *panf;

static UnwrapParams uwp;
static LWMeshInfo *mesh;
static LWPixmapID pixmap;


/*
======================================================================
draw_pixel()

Set a pixel in the Unwrap image.
====================================================================== */

static void draw_pixel( int x, int y )
{
   unsigned char c[ 4 ];
   int i;

   y = ( uwp.height - 1 ) - y;

   switch ( uwp.fgoptions ) {
      case FG_COLOR:
         imgutil->setPixel( pixmap, x, y, uwp.fgcolor );
         break;

      case FG_INVERT:
         imgutil->getPixel( pixmap, x, y, c );
         for ( i = 0; i < 3; i++ )
            c[ i ] = c[ i ] > 127 ?
               c[ i ] >> 1 :
               c[ i ] + (( 255 - c[ i ] ) >> 1 );
         imgutil->setPixel( pixmap, x, y, c );
         break;

      case FG_LIGHTEN:
         imgutil->getPixel( pixmap, x, y, c );
         for ( i = 0; i < 3; i++ )
            c[ i ] += (( 255 - c[ i ] ) >> 1 );
         imgutil->setPixel( pixmap, x, y, c );
         break;

      case FG_DARKEN:
         imgutil->getPixel( pixmap, x, y, c );
         for ( i = 0; i < 3; i++ )
            c[ i ] >>= 1;
         imgutil->setPixel( pixmap, x, y, c );
         break;
   }
}


/*
======================================================================
frac()

Returns the value of x in the principal interval [0, 1].
====================================================================== */

static double frac( double x )
{
   if ( x > 1.0 || x < 0.0 )
      x -= floor( x );
   return x;
}


/*
======================================================================
draw_line3()

Draws a polygon edge, given the 3D coordinates of the vertices at each
end.  The axis is the dominant axis used by cubic mapping.  Called by
scan_polygons().

We call LWTextureFuncs->evaluateUV() for some number of points lying
between p0 and p1.  The number of points is the Manhattan distance in
pixel coordinates.
====================================================================== */

static void draw_line3( LWFVector p0, LWFVector p1, int axis )
{
   double dp0[ 3 ], dp1[ 3 ], pos[ 3 ], dif[ 3 ];
   double uv0[ 2 ], uv1[ 2 ];
   int i, j, x, y, d;

   for ( i = 0; i < 3; i++ ) {
      dp0[ i ] = ( double ) p0[ i ];
      dp1[ i ] = ( double ) p1[ i ];
      dif[ i ] = dp1[ i ] - dp0[ i ];
   }

   txtrf->evaluateUV( uwp.tlayer, axis, axis, dp0, dp0, uv0 );
   txtrf->evaluateUV( uwp.tlayer, axis, axis, dp1, dp1, uv1 );
   x = ( int )(( uv1[ 0 ] - uv0[ 0 ] ) * uwp.width );
   y = ( int )(( uv1[ 1 ] - uv0[ 1 ] ) * uwp.height );
   d = abs( x ) + abs( y );

   for ( i = 0; i <= d; i++ ) {
      for ( j = 0; j < 3; j++ )
         pos[ j ] = dp0[ j ] + i * dif[ j ] / d;
      txtrf->evaluateUV( uwp.tlayer, axis, axis, pos, pos, uv0 );
      x = ( int )( frac( uv0[ 0 ] ) * ( uwp.width - 1 ));
      y = ( int )( frac( uv0[ 1 ] ) * ( uwp.height - 1 ));
      draw_pixel( x, y );
   }
}


/*
======================================================================
draw_line2()

Draws a polygon edge, given the 2D UV coordinates of the vertices at
each end.  This is used for explicitly UV-mapped textures.  Called by
scan_polygons_vmap().

LWTextureFuncs->evaluateUV() doesn't work for UV-mapped textures, but
scan_polygons_vmap() can read the explicit UVs stored in the vertex
map.  We draw the edge by linearly interpolating between the endpoint
UVs (which is also how LW renders the texture).
====================================================================== */

static void draw_line2( float *uv0, float *uv1 )
{
   float dif[ 2 ];
   int i, x, y, d;

   dif[ 0 ] = uv1[ 0 ] - uv0[ 0 ];
   dif[ 1 ] = uv1[ 1 ] - uv0[ 1 ];

   x = ( int )( dif[ 0 ] * uwp.width );
   y = ( int )( dif[ 1 ] * uwp.height );
   d = abs( x ) + abs( y );

   for ( i = 0; i <= d; i++ ) {
      x = ( int )( frac( uv0[ 0 ] + i * dif[ 0 ] / d ) * ( uwp.width - 1 ));
      y = ( int )( frac( uv0[ 1 ] + i * dif[ 1 ] / d ) * ( uwp.height - 1 ));
      draw_pixel( x, y );
   }
}


/*
======================================================================
normalize()

Set a vector's length to 1.0.  Called by poly_normal().
====================================================================== */

static void normalize( LWFVector a )
{
   float d;

   d = ( float ) sqrt( a[ 0 ] * a[ 0 ] + a[ 1 ] * a[ 1 ] + a[ 2 ] * a[ 2 ] );
   if ( d != 0.0f ) {
      a[ 0 ] /= d;
      a[ 1 ] /= d;
      a[ 2 ] /= d;
   }
}


/*
======================================================================
cross()

Form the cross product of two vectors.  Called by poly_normal().
====================================================================== */

static void cross( LWFVector a, LWFVector b, LWFVector c )
{
   c[ 0 ] = a[ 1 ] * b[ 2 ] - a[ 2 ] * b[ 1 ];
   c[ 1 ] = a[ 2 ] * b[ 0 ] - a[ 0 ] * b[ 2 ];
   c[ 2 ] = a[ 0 ] * b[ 1 ] - a[ 1 ] * b[ 0 ];
}


/*
======================================================================
poly_normal()

Calculate the normal of a polygon.  LW forms the normal from the
first, second and last points in the vertex list.  Called by
dominant_axis() to find the axis for cubic mapping.
====================================================================== */

static void poly_normal( LWPolID pol, LWFVector norm )
{
   LWFVector p0, p1, pn, v0, v1;
   int i, nv;

   nv = mesh->polSize( mesh, pol );
   mesh->pntBasePos( mesh, mesh->polVertex( mesh, pol, 0 ), p0 );
   mesh->pntBasePos( mesh, mesh->polVertex( mesh, pol, 1 ), p1 );
   mesh->pntBasePos( mesh, mesh->polVertex( mesh, pol, nv - 1 ), pn );

   for ( i = 0; i < 3; i++ ) {
      v0[ i ] = p1[ i ] - p0[ i ];
      v1[ i ] = pn[ i ] - p1[ i ];
   }

   cross( v0, v1, norm );
   normalize( norm );
}


/*
======================================================================
dominant_axis()

Find the dominant axis for cubic mapping.  This is just the component
of the polygon normal with the largest magnitude.
====================================================================== */

static int dominant_axis( LWPolID pol )
{
   LWFVector norm;

   poly_normal( pol, norm );
   if ( fabs( norm[ 0 ] ) > fabs( norm[ 1 ] ))
      return fabs( norm[ 0 ] ) > fabs( norm[ 2 ] ) ? 0 : 2;
   else
      return fabs( norm[ 1 ] ) > fabs( norm[ 2 ] ) ? 1 : 2;
}


/*
======================================================================
scan_polygons()

Callback for the LWMeshInfo->scanPolys() call.  This is used for the
implicit projections (everything except UV-mapped textures).  The
polygon is drawn only if it's a FACE and if its surface is the
selected surface.
====================================================================== */

static int scan_polygons( const char *surfname, LWPolID pol )
{
   LWFVector p0, p1;
   LWID type;
   const char *tag;
   int i, axis, nv;

   type = mesh->polType( mesh, pol );
   if ( type != LWPOLTYPE_FACE ) return 0;

   tag = mesh->polTag( mesh, pol, LWPTAG_SURF );
   if ( strcmp( surfname, tag )) return 0;

   nv = mesh->polSize( mesh, pol );
   axis = dominant_axis( pol );
   mesh->pntBasePos( mesh, mesh->polVertex( mesh, pol, 0 ), p1 );

   for ( i = 0; i < nv; i++ ) {
      p0[ 0 ] = p1[ 0 ];  p0[ 1 ] = p1[ 1 ];  p0[ 2 ] = p1[ 2 ];
      mesh->pntBasePos( mesh, mesh->polVertex( mesh, pol, ( i + 1 ) % nv ), p1 );
      draw_line3( p0, p1, axis );
   }

   return 0;
}


/*
======================================================================
scan_polygons_vmap()

Callback for the LWMeshInfo->scanPolys() call.  This is used when the
texture is UV mapped.  The polygon is drawn only if it's a FACE and if
its surface is the selected surface.

UVs for the endpoints of each polygon edge are read from the UV vertex
map.  If a discontinuous UV exists (LWMeshInfo->pntVPGet() returns
TRUE), that UV is used.  Otherwise, the UV returned by pntVGet() is
used.
====================================================================== */

static int scan_polygons_vmap( const char *surfname, LWPolID pol )
{
   LWPntID pt;
   LWID type;
   float uv0[ 2 ], uv1[ 2 ];
   const char *tag;
   int i, nv, ismap0, ismap1;

   type = mesh->polType( mesh, pol );
   if ( type != LWPOLTYPE_FACE ) return 0;

   tag = mesh->polTag( mesh, pol, LWPTAG_SURF );
   if ( strcmp( surfname, tag )) return 0;

   nv = mesh->polSize( mesh, pol );
   pt = mesh->polVertex( mesh, pol, 0 );
   ismap1 = mesh->pntVPGet( mesh, pt, pol, uv1 );
   if ( !ismap1 )
      ismap1 = mesh->pntVGet( mesh, pt, uv1 );

   for ( i = 0; i < nv; i++ ) {
      ismap0 = ismap1;
      uv0[ 0 ] = uv1[ 0 ];
      uv0[ 1 ] = uv1[ 1 ];

      pt = mesh->polVertex( mesh, pol, ( i + 1 ) % nv );
      ismap1 = mesh->pntVPGet( mesh, pt, pol, uv1 );
      if ( !ismap1 )
         ismap1 = mesh->pntVGet( mesh, pt, uv1 );
      if ( ismap0 && ismap1 )
         draw_line2( uv0, uv1 );
   }

   return 0;
}


/*
======================================================================
create_pixmap()

Allocate the Unwrap image.  If the Unwrap image will use the image map
as the background, we copy the image map and resize it.  If that fails
for some reason, or if the Unwrap image doesn't use the image map, we
create a blank image filled with the selected background color.
====================================================================== */

static int create_pixmap( GlobalFunc *global )
{
   LWPixmapID pm;
   LWBufferValue rgbfp[ 3 ];
   unsigned char rgb[ 3 ];
   int x, y, w, h, i, c, ok = 0;

   if (( uwp.bgoptions == BG_IMAGE ) && uwp.image ) {
      imglist->size( uwp.image, &w, &h );
      if ( pm = imgutil->create( w, h, LWIMTYP_RGB24 )) {
         for ( y = 0; y < h; y++ ) {
            for ( x = 0; x < w; x++ ) {
               ( *imglist->RGB )( uwp.image, x, y, rgbfp );
               for ( i = 0; i < 3; i++ ) {
                  c = ( int )( rgbfp[ i ] * 255.0f );
                  if ( c > 255 ) c = 255;
                  if ( c < 0 ) c = 0;
                  rgb[ i ] = c;
               }
               imgutil->setPixel( pm, x, y, rgb );
            }
         }
         pixmap = imgutil->resample( pm, uwp.width, uwp.height, LWISM_BICUBIC );
         imgutil->destroy( pm );
         ok = ( pixmap != NULL );
      }
   }

   if ( !ok ) {
      pixmap = imgutil->create( uwp.width, uwp.height, LWIMTYP_RGB24 );
      if ( pixmap ) {
         ok = 1;
         for ( y = 0; y < uwp.height; y++ )
            for ( x = 0; x < uwp.width; x++ )
               imgutil->setPixel( pixmap, x, y, uwp.bgcolor );
      }
   }

   return ok;
}


/*
======================================================================
Unwrap()

The activation function.

1.  Get some globals.
2.  Display the user interface.
3.  Get the LWMeshInfo for the selected object.
4.  Create the Unwrap output image.
5.  Scan the polygons.  The polygons are drawn as each one is sent to
    the scan callback.
6.  Save the Unwrap image.
7.  Free the image.
====================================================================== */

XCALL_( int )
Unwrap( long version, GlobalFunc *global, LWLayoutGeneric *local,
   void *serverData )
{
   const char *surfname;

   iteminfo = global( LWITEMINFO_GLOBAL,     GFUSE_TRANSIENT );
   objinfo  = global( LWOBJECTINFO_GLOBAL,   GFUSE_TRANSIENT );
   imglist  = global( LWIMAGELIST_GLOBAL,    GFUSE_TRANSIENT );
   imgutil  = global( LWIMAGEUTIL_GLOBAL,    GFUSE_TRANSIENT );
   msgf     = global( LWMESSAGEFUNCS_GLOBAL, GFUSE_TRANSIENT );
   surff    = global( LWSURFACEFUNCS_GLOBAL, GFUSE_TRANSIENT );
   txtrf    = global( LWTEXTUREFUNCS_GLOBAL, GFUSE_TRANSIENT );
   panf     = global( LWPANELFUNCS_GLOBAL,   GFUSE_TRANSIENT );

   if ( !iteminfo || !objinfo || !imglist || !msgf || !surff || !txtrf || !panf )
      return AFUNC_BADGLOBAL;

   panf->globalFun = global;

   if ( !uwp.width ) uwp.width = uwp.height = 512;
   if ( !get_user( &uwp )) return AFUNC_OK;

   mesh = objinfo->meshInfo( uwp.object, 1 );
   if ( !mesh ) return AFUNC_BADGLOBAL;

   surfname = surff->name( uwp.surface );
   if ( !surfname ) return AFUNC_BADGLOBAL;

   if ( !create_pixmap( global )) return AFUNC_BADGLOBAL;

   if ( uwp.proj == TXPRJ_UVMAP ) {
      mesh->pntVSelect( mesh, uwp.vmap );
      mesh->scanPolys( mesh, scan_polygons_vmap, surfname );
   }
   else
      mesh->scanPolys( mesh, scan_polygons, surfname );

   imgutil->save( pixmap, uwp.saver, uwp.filename );
   imgutil->destroy( pixmap );

   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { LWLAYOUTGENERIC_CLASS, "Unwrap", Unwrap },
   { NULL }
};
