/*
======================================================================
box.c

A function that makes a box.

Ernie Wright  30 Jun 01
====================================================================== */

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

void makebox( MeshEditOp *edit, BoxData *box )
{
   LWDVector pos;
   LWPntID pt[ 8 ], vt[ 4 ];
   LWPolID pol[ 6 ];
   int i, j;

   for ( i = 0; i < 8; i++ ) {
      for ( j = 0; j < 3; j++ )
         pos[ j ] = box->size[ j ] * vert[ i ][ j ] + box->center[ j ];
      pt[ i ] = edit->addPoint( edit->state, pos );
      edit->pntVMap( edit->state, pt[ i ], LWVMAP_TXUV, box->vmapname, 2,
         cuv[ i ] );
   }

   for ( i = 0; i < 6; i++ ) {
      for ( j = 0; j < 4; j++ )
         vt[ j ] = pt[ face[ i ][ j ]];
      pol[ i ] = edit->addFace( edit->state, box->surfname, 4, vt );
   }

   edit->pntVPMap( edit->state, pt[ 3 ], pol[ 4 ],
      LWVMAP_TXUV, box->vmapname, 2, duv[ 0 ] );
   edit->pntVPMap( edit->state, pt[ 7 ], pol[ 4 ],
      LWVMAP_TXUV, box->vmapname, 2, duv[ 1 ] );
}
