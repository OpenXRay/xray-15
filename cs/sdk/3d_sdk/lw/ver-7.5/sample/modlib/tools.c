/*
======================================================================
tools.c

Ernie Wright  3 May 99

This module, part of the ModLib library, contains functions that
Modeler has historically called "tools."
====================================================================== */

#include <stdlib.h>
#include <assert.h>

#define CS_MACROS
#include "lwmodlib.h"


int csAxisDrill( char *operation, char *axis, char *surface )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 3 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = operation;
   argv[ 0 ].str.bufLen = 0;

   argv[ 1 ].type = DY_STRING;
   argv[ 1 ].str.buf = axis;
   argv[ 1 ].str.bufLen = 0;

   if ( surface ) {
      argv[ 2 ].type = DY_STRING;
      argv[ 2 ].str.buf = surface;
      argv[ 2 ].str.bufLen = 0;
   }
   else argv[ 2 ].type = DY_NULL;

   LOOKUP( "AXISDRILL" );
   EXECUTE( 3, argv );
   return OK;
}


int csSolidDrill( char *operation, char *surface )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 2 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = operation;
   argv[ 0 ].str.bufLen = 0;

   if ( surface ) {
      argv[ 1 ].type = DY_STRING;
      argv[ 1 ].str.buf = surface;
      argv[ 1 ].str.bufLen = 0;
   }
   else argv[ 1 ].type = DY_NULL;

   LOOKUP( "SOLIDDRILL" );
   EXECUTE( 2, argv );
   return OK;
}


int csBoolean( char *operation )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv;

   assert( md->edit == NULL );

   argv.type = DY_STRING;
   argv.str.buf = operation;
   argv.str.bufLen = 0;

   LOOKUP( "BOOLEAN" );
   EXECUTE( 1, &argv );
   return OK;
}


int csBevel( double inset, double shift )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 2 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_DISTANCE;
   argv[ 0 ].flt.value = inset;

   argv[ 1 ].type = DY_DISTANCE;
   argv[ 1 ].flt.value = shift;

   LOOKUP( "BEVEL" );
   EXECUTE( 2, argv );
   return OK;
}


int csShapeBevel( int npairs, double *insh )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv;

   assert( md->edit == NULL );

   argv.type = DY_CUSTOM;
   argv.cust.val[ 0 ] = npairs;
   argv.cust.val[ 1 ] = ( int ) insh;

   LOOKUP( "SHAPEBEVEL" );
   EXECUTE( 1, &argv );
   return OK;
}


int csSmoothShift( double offset, double maxangle )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 2 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_FLOAT;
   argv[ 0 ].flt.value = offset;

   if ( maxangle >= 0.0 ) {
      argv[ 1 ].type = DY_FLOAT;
      argv[ 1 ].flt.value = maxangle;
   }
   else argv[ 1 ].type = DY_NULL;

   LOOKUP( "SMOOTHSHIFT" );
   EXECUTE( 2, argv );
   return OK;
}


int csSmoothScale( double offset )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv;

   assert( md->edit == NULL );

   argv.type = DY_FLOAT;
   argv.flt.value = offset;

   LOOKUP( "SMOOTHSCALE" );
   EXECUTE( 1, &argv );
   return OK;
}


int csMirror( char *axis, double plane )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 2 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = axis;
   argv[ 0 ].str.bufLen = 0;

   argv[ 1 ].type = DY_FLOAT;
   argv[ 1 ].flt.value = plane;

   LOOKUP( "MIRROR" );
   EXECUTE( 2, argv );
   return OK;
}
