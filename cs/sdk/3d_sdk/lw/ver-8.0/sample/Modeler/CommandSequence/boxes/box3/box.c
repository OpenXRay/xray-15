/*
======================================================================
box.c

A Modeler plug-in that makes a box.

Ernie Wright  2 Jun 01
====================================================================== */

#include <lwserver.h>
#include <lwcmdseq.h>
#include <lwxpanel.h>
#include <lwsurf.h>
#include <lwdyna.h>
#include "box.h"


double vert[ 8 ][ 3 ] = {   /* a unit cube */
   -0.5, -0.5, -0.5,
    0.5, -0.5, -0.5,
    0.5, -0.5,  0.5,
   -0.5, -0.5,  0.5,
   -0.5,  0.5, -0.5,
    0.5,  0.5, -0.5,
    0.5,  0.5,  0.5,
   -0.5,  0.5,  0.5
};

int face[ 6 ][ 4 ] = {     /* vertex indexes */
   0, 1, 2, 3,
   0, 4, 5, 1,
   1, 5, 6, 2,
   3, 2, 6, 7,
   0, 3, 7, 4,
   4, 7, 6, 5
};

float cuv[ 8 ][ 2 ] = {    /* continuous UVs (spherical mapping) */
   .125f, .304f,
   .375f, .304f,
   .625f, .304f,
   .875f, .304f,
   .125f, .696f,
   .375f, .696f,
   .625f, .696f,
   .875f, .696f
};

float duv[ 2 ][ 2 ] = {    /* discontinuous UVs */
   -0.125f, 0.304f,
   -0.125f, 0.696f
};


/*
======================================================================
makebox()

Create a box using mesh edit functions.
====================================================================== */

void makebox( MeshEditOp *edit, double *size, double *center,
   char *surfname, char *vmapname )
{
   LWDVector pos;
   LWPntID pt[ 8 ], vt[ 4 ];
   LWPolID pol[ 6 ];
   int i, j;

   for ( i = 0; i < 8; i++ ) {
      for ( j = 0; j < 3; j++ )
         pos[ j ] = size[ j ] * vert[ i ][ j ] + center[ j ];
      pt[ i ] = edit->addPoint( edit->state, pos );
      edit->pntVMap( edit->state, pt[ i ], LWVMAP_TXUV, vmapname, 2,
         cuv[ i ] );
   }

   for ( i = 0; i < 6; i++ ) {
      for ( j = 0; j < 4; j++ )
         vt[ j ] = pt[ face[ 0 ][ j ]];
      pol[ i ] = edit->addFace( edit->state, surfname, 4, vt );
   }

   edit->pntVPMap( edit->state, pt[ 3 ], pol[ 4 ],
      LWVMAP_TXUV, vmapname, 2, duv[ 0 ] );
   edit->pntVPMap( edit->state, pt[ 7 ], pol[ 4 ],
      LWVMAP_TXUV, vmapname, 2, duv[ 1 ] );
}


/*
======================================================================
Activate()

Our activation function.
====================================================================== */

XCALL_( int )
Activate( long version, GlobalFunc *global, LWModCommand *local,
   void *serverData )
{
   DynaConvertFunc *dynaf;
   LWXPanelFuncs *xpanf;
   LWSurfaceFuncs *surff;
   MeshEditOp *edit;
   double size[ 3 ]   = { 1.0, 1.0, 1.0 };
   double center[ 3 ] = { 0.0, 0.0, 0.0 };
   char surfname[ 128 ];
   char vmapname[ 128 ] = "MyUVs";
   int ok = 0;

   if ( version != LWMODCOMMAND_VERSION )
      return AFUNC_BADVERSION;

   if ( local->argument[ 0 ] ) {
      dynaf = global( LWDYNACONVERTFUNC_GLOBAL, GFUSE_TRANSIENT );
      if ( !dynaf ) return AFUNC_BADGLOBAL;
      ok = parse_cmdline( dynaf, local->argument,
         size, center, surfname, vmapname );
      if ( !ok ) return AFUNC_BADLOCAL;
   }
   else {
      xpanf = global( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT );
      surff = global( LWSURFACEFUNCS_GLOBAL, GFUSE_TRANSIENT );
      if ( !xpanf || !surff ) return AFUNC_BADGLOBAL;
      if ( !init_surflist( surff )) return AFUNC_BADGLOBAL;
      ok = get_user( xpanf, size, center, surfname, vmapname );
      free_surflist();
   }

   if ( ok ) {
      edit = local->editBegin( 0, 0, OPSEL_GLOBAL );
      if ( edit ) {
         makebox( edit, size, center, surfname, vmapname );
         edit->done( edit->state, EDERR_NONE, 0 );
      }
   }

   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { LWMODCOMMAND_CLASS, "Tutorial_Box3", Activate },
   { NULL }
};
