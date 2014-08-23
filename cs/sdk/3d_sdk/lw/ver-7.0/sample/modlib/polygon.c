/*
======================================================================
polygon.c

Ernie Wright  7 Jan 00

This module, part of the ModLib library, contains functions that
operate on polygons.
====================================================================== */

#include <stdlib.h>
#include <assert.h>

#define CS_MACROS
#include "lwmodlib.h"


int csFlip( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );

   LOOKUP( "FLIP" );
   EXECUTE( 0, NULL );
   return OK;
}


int csTriple( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );

   LOOKUP( "TRIPLE" );
   EXECUTE( 0, NULL );
   return OK;
}


int csFreezeCurves( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );

   LOOKUP( "FREEZECURVES" );
   EXECUTE( 0, NULL );
   return OK;
}


int csAlignPols( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );

   LOOKUP( "ALIGNPOLS" );
   EXECUTE( 0, NULL );
   return OK;
}


int csRemovePols( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );

   LOOKUP( "REMOVEPOLS" );
   EXECUTE( 0, NULL );
   return OK;
}


int csUnifyPols( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );

   LOOKUP( "UNIFYPOLS" );
   EXECUTE( 0, NULL );
   return OK;
}


int csChangePart( char *part )
{
   static LWCommandCode ccode;
   DynaValue argv;
   ModData *md = getModData();

   assert( md->edit == NULL );

   argv.type = DY_STRING;
   argv.str.buf = part;
   argv.str.bufLen = 0;

   LOOKUP( "CHANGEPART" );
   EXECUTE( 1, &argv );
   return OK;
}


int csSubdivide( char *mode, double maxangle )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 2 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = mode;
   argv[ 0 ].str.bufLen = 0;

   if ( maxangle > 0.0 ) {
      argv[ 1 ].type = DY_FLOAT;
      argv[ 1 ].flt.value = maxangle;
   }
   else argv[ 1 ].type = DY_NULL;

   LOOKUP( "SUBDIVIDE" );
   EXECUTE( 2, argv );
   return OK;
}


int csFracSubdivide( char *mode, double fractal, double maxangle )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 3 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = mode;
   argv[ 0 ].str.bufLen = 0;

   argv[ 1 ].type = DY_FLOAT;
   argv[ 1 ].flt.value = fractal;

   if ( maxangle > 0.0 ) {
      argv[ 2 ].type = DY_FLOAT;
      argv[ 2 ].flt.value = maxangle;
   }
   else argv[ 2 ].type = DY_NULL;

   LOOKUP( "FRACSUBDIVIDE" );
   EXECUTE( 3, argv );
   return OK;
}


int csMorphPols( int nsegments )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv;

   assert( md->edit == NULL );
   assert( md->opsel == OPSEL_USER || md->opsel == OPSEL_DIRECT );

   argv.type = DY_INTEGER;
   argv.intv.value = nsegments;

   LOOKUP( "MORPHPOLS" );
   EXECUTE( 1, &argv );
   return OK;
}


int csMergePols( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );
   assert( md->opsel == OPSEL_USER || md->opsel == OPSEL_DIRECT );

   LOOKUP( "MERGEPOLS" );
   EXECUTE( 0, NULL );
   return OK;
}


int csSplitPols( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );
   assert( md->opsel == OPSEL_USER || md->opsel == OPSEL_DIRECT );

   LOOKUP( "SPLITPOLS" );
   EXECUTE( 0, NULL );
   return OK;
}


int csSkinPols( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );
   assert( md->opsel == OPSEL_USER || md->opsel == OPSEL_DIRECT );

   LOOKUP( "SKINPOLS" );
   EXECUTE( 0, NULL );
   return OK;
}


int csSmoothCurves( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );
   assert( md->opsel == OPSEL_USER || md->opsel == OPSEL_DIRECT );

   LOOKUP( "SMOOTHCURVES" );
   EXECUTE( 0, NULL );
   return OK;
}


int csToggleCCStart( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );

   LOOKUP( "TOGGLECCSTART" );
   EXECUTE( 0, NULL );
   return OK;
}


int csToggleCCEnd( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );

   LOOKUP( "TOGGLECCEND" );
   EXECUTE( 0, NULL );
   return OK;
}


int csTogglePatches( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );

   LOOKUP( "TOGGLEPATCHES" );
   EXECUTE( 0, NULL );
   return OK;
}


int csMake4Patch( double perpendicular, double parallel )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 2 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_FLOAT;
   argv[ 0 ].flt.value = perpendicular;

   argv[ 1 ].type = DY_FLOAT;
   argv[ 1 ].flt.value = parallel;

   LOOKUP( "MAKE4PATCH" );
   EXECUTE( 2, argv );
   return OK;
}
