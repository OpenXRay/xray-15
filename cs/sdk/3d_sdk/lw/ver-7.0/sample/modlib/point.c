/*
======================================================================
point.c

Ernie Wright  7 Jan 00

This module, part of the ModLib library, collects together functions
that modify geometry by moving points.
====================================================================== */

#include <stdlib.h>
#include <assert.h>

#define CS_MACROS
#include "lwmodlib.h"


int csFixedFlex( char *axis, double start, double end,
   char *ease )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 4 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = axis;
   argv[ 0 ].str.bufLen = 0;

   argv[ 1 ].type = DY_FLOAT;
   argv[ 1 ].flt.value = start;

   argv[ 2 ].type = DY_FLOAT;
   argv[ 2 ].flt.value = end;

   if ( ease ) {
      argv[ 3 ].type = DY_STRING;
      argv[ 3 ].str.buf = ease;
      argv[ 3 ].str.bufLen = 0;
   }
   else argv[ 3 ].type = DY_NULL;

   LOOKUP( "FIXEDFLEX" );
   EXECUTE( 4, argv );
   return OK;
}


int csAutoFlex( char *axis, char *direction, char *ease )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 3 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = axis;
   argv[ 0 ].str.bufLen = 0;

   argv[ 1 ].type = DY_STRING;
   argv[ 1 ].str.buf = direction;
   argv[ 1 ].str.bufLen = 0;

   if ( ease ) {
      argv[ 2 ].type = DY_STRING;
      argv[ 2 ].str.buf = ease;
      argv[ 2 ].str.bufLen = 0;
   }
   else argv[ 2 ].type = DY_NULL;

   LOOKUP( "AUTOFLEX" );
   EXECUTE( 3, argv );
   return OK;
}


int csDeformRegion( double *radius, double *center, char *axis )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 3 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_VFLOAT;
   argv[ 0 ].fvec.val[ 0 ] = radius[ 0 ];
   argv[ 0 ].fvec.val[ 1 ] = radius[ 1 ];
   argv[ 0 ].fvec.val[ 2 ] = radius[ 2 ];

   if ( center ) {
      argv[ 1 ].type = DY_VFLOAT;
      argv[ 1 ].fvec.val[ 0 ] = center[ 0 ];
      argv[ 1 ].fvec.val[ 1 ] = center[ 1 ];
      argv[ 1 ].fvec.val[ 2 ] = center[ 2 ];
   }
   else argv[ 1 ].type = DY_NULL;

   if ( axis ) {
      argv[ 2 ].type = DY_STRING;
      argv[ 2 ].str.buf = axis;
      argv[ 2 ].str.bufLen = 0;
   }
   else argv[ 2 ].type = DY_NULL;

   LOOKUP( "DEFORMREGION" );
   EXECUTE( 3, argv );
   return OK;
}


int csMove( double *offset )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv;

   assert( md->edit == NULL );

   argv.type = DY_VFLOAT;
   argv.fvec.val[ 0 ] = offset[ 0 ];
   argv.fvec.val[ 1 ] = offset[ 1 ];
   argv.fvec.val[ 2 ] = offset[ 2 ];

   LOOKUP( "MOVE" );
   EXECUTE( 1, &argv );
   return OK;
}


int csShear( double *offset )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv;

   assert( md->edit == NULL );

   argv.type = DY_VFLOAT;
   argv.fvec.val[ 0 ] = offset[ 0 ];
   argv.fvec.val[ 1 ] = offset[ 1 ];
   argv.fvec.val[ 2 ] = offset[ 2 ];

   LOOKUP( "SHEAR" );
   EXECUTE( 1, &argv );
   return OK;
}


int csMagnet( double *offset )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv;

   assert( md->edit == NULL );

   argv.type = DY_VFLOAT;
   argv.fvec.val[ 0 ] = offset[ 0 ];
   argv.fvec.val[ 1 ] = offset[ 1 ];
   argv.fvec.val[ 2 ] = offset[ 2 ];

   LOOKUP( "MAGNET" );
   EXECUTE( 1, &argv );
   return OK;
}


int csRotate( double angle, char *axis, double *center )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 3 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_FLOAT;
   argv[ 0 ].flt.value = angle;

   argv[ 1 ].type = DY_STRING;
   argv[ 1 ].str.buf = axis;
   argv[ 1 ].str.bufLen = 0;

   if ( center ) {
      argv[ 2 ].type = DY_VFLOAT;
      argv[ 2 ].fvec.val[ 0 ] = center[ 0 ];
      argv[ 2 ].fvec.val[ 1 ] = center[ 1 ];
      argv[ 2 ].fvec.val[ 2 ] = center[ 2 ];
   }
   else argv[ 2 ].type = DY_NULL;

   LOOKUP( "ROTATE" );
   EXECUTE( 3, argv );
   return OK;
}


int csTwist( double angle, char *axis, double *center )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 3 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_FLOAT;
   argv[ 0 ].flt.value = angle;

   argv[ 1 ].type = DY_STRING;
   argv[ 1 ].str.buf = axis;
   argv[ 1 ].str.bufLen = 0;

   if ( center ) {
      argv[ 2 ].type = DY_VFLOAT;
      argv[ 2 ].fvec.val[ 0 ] = center[ 0 ];
      argv[ 2 ].fvec.val[ 1 ] = center[ 1 ];
      argv[ 2 ].fvec.val[ 2 ] = center[ 2 ];
   }
   else argv[ 2 ].type = DY_NULL;

   LOOKUP( "TWIST" );
   EXECUTE( 3, argv );
   return OK;
}


int csVortex( double angle, char *axis, double *center )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 3 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_FLOAT;
   argv[ 0 ].flt.value = angle;

   argv[ 1 ].type = DY_STRING;
   argv[ 1 ].str.buf = axis;
   argv[ 1 ].str.bufLen = 0;

   if ( center ) {
      argv[ 2 ].type = DY_VFLOAT;
      argv[ 2 ].fvec.val[ 0 ] = center[ 0 ];
      argv[ 2 ].fvec.val[ 1 ] = center[ 1 ];
      argv[ 2 ].fvec.val[ 2 ] = center[ 2 ];
   }
   else argv[ 2 ].type = DY_NULL;

   LOOKUP( "VORTEX" );
   EXECUTE( 3, argv );
   return OK;
}


int csScale( double *factor, double *center )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 2 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_VFLOAT;
   argv[ 0 ].fvec.val[ 0 ] = factor[ 0 ];
   argv[ 0 ].fvec.val[ 1 ] = factor[ 1 ];
   argv[ 0 ].fvec.val[ 2 ] = factor[ 2 ];

   if ( center ) {
      argv[ 1 ].type = DY_VFLOAT;
      argv[ 1 ].fvec.val[ 0 ] = center[ 0 ];
      argv[ 1 ].fvec.val[ 1 ] = center[ 1 ];
      argv[ 1 ].fvec.val[ 2 ] = center[ 2 ];
   }
   else argv[ 1 ].type = DY_NULL;

   LOOKUP( "SCALE" );
   EXECUTE( 2, argv );
   return OK;
}


int csTaper( double *factor, double *center )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 2 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_VFLOAT;
   argv[ 0 ].fvec.val[ 0 ] = factor[ 0 ];
   argv[ 0 ].fvec.val[ 1 ] = factor[ 1 ];
   argv[ 0 ].fvec.val[ 2 ] = factor[ 2 ];

   if ( center ) {
      argv[ 1 ].type = DY_VFLOAT;
      argv[ 1 ].fvec.val[ 0 ] = center[ 0 ];
      argv[ 1 ].fvec.val[ 1 ] = center[ 1 ];
      argv[ 1 ].fvec.val[ 2 ] = center[ 2 ];
   }
   else argv[ 1 ].type = DY_NULL;

   LOOKUP( "TAPER" );
   EXECUTE( 2, argv );
   return OK;
}


int csPole( double *factor, double *center )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 2 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_VFLOAT;
   argv[ 0 ].fvec.val[ 0 ] = factor[ 0 ];
   argv[ 0 ].fvec.val[ 1 ] = factor[ 1 ];
   argv[ 0 ].fvec.val[ 2 ] = factor[ 2 ];

   if ( center ) {
      argv[ 1 ].type = DY_VFLOAT;
      argv[ 1 ].fvec.val[ 0 ] = center[ 0 ];
      argv[ 1 ].fvec.val[ 1 ] = center[ 1 ];
      argv[ 1 ].fvec.val[ 2 ] = center[ 2 ];
   }
   else argv[ 1 ].type = DY_NULL;

   LOOKUP( "POLE" );
   EXECUTE( 2, argv );
   return OK;
}


int csBend( double angle, double direction, double *center )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 3 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_FLOAT;
   argv[ 0 ].flt.value = angle;

   argv[ 1 ].type = DY_FLOAT;
   argv[ 1 ].flt.value = direction;

   if ( center ) {
      argv[ 2 ].type = DY_VFLOAT;
      argv[ 2 ].fvec.val[ 0 ] = center[ 0 ];
      argv[ 2 ].fvec.val[ 1 ] = center[ 1 ];
      argv[ 2 ].fvec.val[ 2 ] = center[ 2 ];
   }
   else argv[ 2 ].type = DY_NULL;

   LOOKUP( "BEND" );
   EXECUTE( 3, argv );
   return OK;
}


int csJitter( double *radius, char *type, double *center )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 3 ];
   int argc = 1;

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_VFLOAT;
   argv[ 0 ].fvec.val[ 0 ] = radius[ 0 ];
   argv[ 0 ].fvec.val[ 1 ] = radius[ 1 ];
   argv[ 0 ].fvec.val[ 2 ] = radius[ 2 ];

   if ( type ) {
      argv[ 1 ].type = DY_STRING;
      argv[ 1 ].str.buf = type;
      argv[ 1 ].str.bufLen = 0;
   }
   else argv[ 1 ].type = DY_NULL;

   if ( center ) {
      argv[ 2 ].type = DY_VFLOAT;
      argv[ 2 ].fvec.val[ 0 ] = center[ 0 ];
      argv[ 2 ].fvec.val[ 1 ] = center[ 1 ];
      argv[ 2 ].fvec.val[ 2 ] = center[ 2 ];
   }
   else argv[ 2 ].type = DY_NULL;

   LOOKUP( "JITTER" );
   EXECUTE( 3, argv );
   return OK;
}


int csSmooth( int iterations, double strength )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 2 ];

   assert( md->edit == NULL );

   if ( iterations ) {
      argv[ 0 ].type = DY_INTEGER;
      argv[ 0 ].intv.value = iterations;
   }
   else argv[ 0 ].type = DY_NULL;

   if ( strength ) {
      argv[ 1 ].type = DY_FLOAT;
      argv[ 1 ].flt.value = strength;
   }
   else argv[ 1 ].type = DY_NULL;

   LOOKUP( "SMOOTH" );
   EXECUTE( 2, argv );
   return OK;
}


int csQuantize( double *size )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv;

   assert( md->edit == NULL );

   argv.type = DY_VFLOAT;
   argv.fvec.val[ 0 ] = size[ 0 ];
   argv.fvec.val[ 1 ] = size[ 1 ];
   argv.fvec.val[ 2 ] = size[ 2 ];

   LOOKUP( "QUANTIZE" );
   EXECUTE( 1, &argv );
   return OK;
}


int csMergePoints( double d )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv;

   assert( md->edit == NULL );

   if ( d >= 0.0 ) {
      argv.type = DY_FLOAT;
      argv.flt.value = d;
   }
   else argv.type = DY_NULL;

   LOOKUP( "MERGEPOINTS" );
   EXECUTE( 1, &argv );
   return OK;
}


int csWeldPoints( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );
   assert( md->opsel == OPSEL_USER || md->opsel == OPSEL_DIRECT );

   LOOKUP( "WELDPOINTS" );
   EXECUTE( 0, NULL );
   return OK;
}


int csWeldAverage( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );
   assert( md->opsel == OPSEL_USER || md->opsel == OPSEL_DIRECT );

   LOOKUP( "WELDAVERAGE" );
   EXECUTE( 0, NULL );
   return OK;
}
