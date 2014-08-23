/*
======================================================================
sweep.c

Ernie Wright  7 Jan 00

This module, part of the ModLib library, contains functions that
create geometry by replication along a curve.
====================================================================== */

#include <stdlib.h>
#include <assert.h>

#define CS_MACROS
#include "lwmodlib.h"


int csLathe( char *axis, int nsides, double *center, double endangle,
   double startangle, double offset )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 6 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = axis;
   argv[ 0 ].str.bufLen = 0;

   argv[ 1 ].type = DY_INTEGER;
   argv[ 1 ].intv.value = nsides;

   if ( center ) {
      argv[ 2 ].type = DY_FLOAT;
      argv[ 2 ].fvec.val[ 0 ] = center[ 0 ];
      argv[ 2 ].fvec.val[ 1 ] = center[ 1 ];
      argv[ 2 ].fvec.val[ 2 ] = center[ 2 ];
   }
   else argv[ 2 ].type = DY_NULL;

   if ( endangle != startangle ) {
      argv[ 3 ].type = DY_FLOAT;
      argv[ 3 ].flt.value = endangle;

      argv[ 4 ].type = DY_FLOAT;
      argv[ 4 ].flt.value = startangle;
   }
   else argv[ 3 ].type = argv[ 4 ].type = DY_NULL;

   if ( offset != 0.0 ) {
      argv[ 5 ].type = DY_FLOAT;
      argv[ 5 ].flt.value = offset;
   }
   else argv[ 5 ].type = DY_NULL;

   LOOKUP( "LATHE" );
   EXECUTE( 6, argv );
   return OK;
}


int csExtrude( char *axis, double extent, int nsegments )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 3 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = axis;
   argv[ 0 ].str.bufLen = 0;

   argv[ 1 ].type = DY_FLOAT;
   argv[ 1 ].flt.value = extent;

   if ( nsegments > 0 ) {
      argv[ 2 ].type = DY_INTEGER;
      argv[ 2 ].intv.value = nsegments;
   }
   else argv[ 2 ].type = DY_NULL;

   LOOKUP( "EXTRUDE" );
   EXECUTE( 3, argv );
   return OK;
}


int csPathClone( char *filename, double step, double start, double end )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 4 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = filename;
   argv[ 0 ].str.bufLen = 0;

   if ( step != 0.0 ) {
      argv[ 1 ].type = DY_FLOAT;
      argv[ 1 ].flt.value = step;
   }
   else argv[ 1 ].type = DY_NULL;

   if ( start != end ) {
      argv[ 2 ].type = DY_FLOAT;
      argv[ 2 ].flt.value = start;

      argv[ 3 ].type = DY_FLOAT;
      argv[ 3 ].flt.value = end;
   }
   else argv[ 2 ].type = argv[ 3 ].type = DY_NULL;

   LOOKUP( "PATHCLONE" );
   EXECUTE( 4, argv );
   return OK;
}


int csPathExtrude( char *filename, double step, double start,
   double end )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 4 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = filename;
   argv[ 0 ].str.bufLen = 0;

   if ( step != 0.0 ) {
      argv[ 1 ].type = DY_FLOAT;
      argv[ 1 ].flt.value = step;
   }
   else argv[ 1 ].type = DY_NULL;

   if ( start != end ) {
      argv[ 2 ].type = DY_FLOAT;
      argv[ 2 ].flt.value = start;

      argv[ 3 ].type = DY_FLOAT;
      argv[ 3 ].flt.value = end;
   }
   else argv[ 2 ].type = argv[ 3 ].type = DY_NULL;

   LOOKUP( "PATHEXTRUDE" );
   EXECUTE( 4, argv );
   return OK;
}


int csRailClone( int nsegments, char *divs, char *flags, double strength )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 4 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_INTEGER;
   argv[ 0 ].intv.value = nsegments;

   if ( divs ) {
      argv[ 1 ].type = DY_STRING;
      argv[ 1 ].str.buf = divs;
      argv[ 1 ].str.bufLen = 0;
   }
   else argv[ 1 ].type = DY_NULL;

   if ( flags ) {
      argv[ 2 ].type = DY_STRING;
      argv[ 2 ].str.buf = flags;
      argv[ 2 ].str.bufLen = 0;
   }
   else argv[ 2 ].type = DY_NULL;

   if ( strength != 0.0 ) {
         argv[ 3 ].type = DY_FLOAT;
         argv[ 3 ].flt.value = strength;
   }
   else argv[ 3 ].type = DY_NULL;

   LOOKUP( "RAILCLONE" );
   EXECUTE( 4, argv );
   return OK;
}


int csRailExtrude( int nsegments, char *divs, char *flags, double strength )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 4 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_INTEGER;
   argv[ 0 ].intv.value = nsegments;

   if ( divs ) {
      argv[ 1 ].type = DY_STRING;
      argv[ 1 ].str.buf = divs;
      argv[ 1 ].str.bufLen = 0;
   }
   else argv[ 1 ].type = DY_NULL;

   if ( flags ) {
      argv[ 2 ].type = DY_STRING;
      argv[ 2 ].str.buf = flags;
      argv[ 2 ].str.bufLen = 0;
   }
   else argv[ 2 ].type = DY_NULL;

   if ( strength != 0.0 ) {
         argv[ 3 ].type = DY_FLOAT;
         argv[ 3 ].flt.value = strength;
   }
   else argv[ 3 ].type = DY_NULL;

   LOOKUP( "RAILEXTRUDE" );
   EXECUTE( 4, argv );
   return OK;
}
