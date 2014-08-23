/*
======================================================================
make.c

Ernie Wright  7 Jan 00

This module, part of the ModLib library, contains functions for making
Modeler's primitives.
====================================================================== */

#include <stdlib.h>
#include <assert.h>

#define CS_MACROS
#include "lwmodlib.h"


int csMakeBox( double *lowcorner, double *highcorner, int *nsegments )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 3 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_VFLOAT;
   argv[ 0 ].fvec.val[ 0 ] = lowcorner[ 0 ];
   argv[ 0 ].fvec.val[ 1 ] = lowcorner[ 1 ];
   argv[ 0 ].fvec.val[ 2 ] = lowcorner[ 2 ];

   argv[ 1 ].type = DY_VFLOAT;
   argv[ 1 ].fvec.val[ 0 ] = highcorner[ 0 ];
   argv[ 1 ].fvec.val[ 1 ] = highcorner[ 1 ];
   argv[ 1 ].fvec.val[ 2 ] = highcorner[ 2 ];

   if ( nsegments ) {
      argv[ 2 ].type = DY_VINT;
      argv[ 2 ].ivec.val[ 0 ] = nsegments[ 0 ];
      argv[ 2 ].ivec.val[ 1 ] = nsegments[ 1 ];
      argv[ 2 ].ivec.val[ 2 ] = nsegments[ 2 ];
   }
   else argv[ 2 ].type = DY_NULL;

   LOOKUP( "MAKEBOX" );
   EXECUTE( 3, argv );
   return OK;
}


int csMakeBall( double *radius, int nsides, int nsegments, double *center )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 4 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_VFLOAT;
   argv[ 0 ].fvec.val[ 0 ] = radius[ 0 ];
   argv[ 0 ].fvec.val[ 1 ] = radius[ 1 ];
   argv[ 0 ].fvec.val[ 2 ] = radius[ 2 ];

   argv[ 1 ].type = DY_INTEGER;
   argv[ 1 ].intv.value = nsides;

   argv[ 2 ].type = DY_INTEGER;
   argv[ 2 ].intv.value = nsegments;

   if ( center ) {
      argv[ 3 ].type = DY_VFLOAT;
      argv[ 3 ].fvec.val[ 0 ] = center[ 0 ];
      argv[ 3 ].fvec.val[ 1 ] = center[ 1 ];
      argv[ 3 ].fvec.val[ 2 ] = center[ 2 ];
   }
   else argv[ 3 ].type = DY_NULL;

   LOOKUP( "MAKEBALL" );
   EXECUTE( 4, argv );
   return OK;
}


int csMakeTesBall( double *radius, int level, double *center )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 3 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_VFLOAT;
   argv[ 0 ].fvec.val[ 0 ] = radius[ 0 ];
   argv[ 0 ].fvec.val[ 1 ] = radius[ 1 ];
   argv[ 0 ].fvec.val[ 2 ] = radius[ 2 ];

   argv[ 1 ].type = DY_INTEGER;
   argv[ 1 ].intv.value = level;

   if ( center ) {
      argv[ 2 ].type = DY_VFLOAT;
      argv[ 2 ].fvec.val[ 0 ] = center[ 0 ];
      argv[ 2 ].fvec.val[ 1 ] = center[ 1 ];
      argv[ 2 ].fvec.val[ 2 ] = center[ 2 ];
   }
   else argv[ 2 ].type = DY_NULL;

   LOOKUP( "MAKETESBALL" );
   EXECUTE( 3, argv );
   return OK;
}


int csMakeDisc( double *radius, double top, double bottom, char *axis,
   int nsides, int nsegments, double *center )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 7 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_VFLOAT;
   argv[ 0 ].fvec.val[ 0 ] = radius[ 0 ];
   argv[ 0 ].fvec.val[ 1 ] = radius[ 1 ];
   argv[ 0 ].fvec.val[ 2 ] = radius[ 2 ];

   argv[ 1 ].type = DY_FLOAT;
   argv[ 1 ].flt.value = top;

   argv[ 2 ].type = DY_FLOAT;
   argv[ 2 ].flt.value = bottom;

   argv[ 3 ].type = DY_STRING;
   argv[ 3 ].str.buf = axis;
   argv[ 3 ].str.bufLen = 0;

   argv[ 4 ].type = DY_INTEGER;
   argv[ 4 ].intv.value = nsides;

   if ( nsegments > 0 ) {
      argv[ 5 ].type = DY_INTEGER;
      argv[ 5 ].intv.value = nsegments;
   }
   else argv[ 5 ].type = DY_NULL;

   if ( center ) {
      argv[ 6 ].type = DY_VDIST;
      argv[ 6 ].fvec.val[ 0 ] = center[ 0 ];
      argv[ 6 ].fvec.val[ 1 ] = center[ 1 ];
      argv[ 6 ].fvec.val[ 2 ] = center[ 2 ];
   }
   else argv[ 6 ].type = DY_NULL;

   LOOKUP( "MAKEDISC" );
   EXECUTE( 7, argv );
   return OK;
}


int csMakeCone( double *radius, double top, double bottom,
   char *axis, int nsides, int nsegments, double *center )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 7 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_VFLOAT;
   argv[ 0 ].fvec.val[ 0 ] = radius[ 0 ];
   argv[ 0 ].fvec.val[ 1 ] = radius[ 1 ];
   argv[ 0 ].fvec.val[ 2 ] = radius[ 2 ];

   argv[ 1 ].type = DY_FLOAT;
   argv[ 1 ].flt.value = top;

   argv[ 2 ].type = DY_FLOAT;
   argv[ 2 ].flt.value = bottom;

   argv[ 3 ].type = DY_STRING;
   argv[ 3 ].str.buf = axis;
   argv[ 3 ].str.bufLen = 0;

   argv[ 4 ].type = DY_INTEGER;
   argv[ 4 ].intv.value = nsides;

   if ( nsegments > 0 ) {
      argv[ 5 ].type = DY_INTEGER;
      argv[ 5 ].intv.value = nsegments;
   }
   else argv[ 5 ].type = DY_NULL;

   if ( center ) {
      argv[ 6 ].type = DY_VFLOAT;
      argv[ 6 ].fvec.val[ 0 ] = center[ 0 ];
      argv[ 6 ].fvec.val[ 1 ] = center[ 1 ];
      argv[ 6 ].fvec.val[ 2 ] = center[ 2 ];
   }
   else argv[ 6 ].type = DY_NULL;

   LOOKUP( "MAKECONE" );
   EXECUTE( 7, argv );
   return OK;
}
