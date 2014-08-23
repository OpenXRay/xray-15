/*
======================================================================
select.c

Ernie Wright  7 Jan 00

This module, part of the ModLib library, collects together functions
used to select items.
====================================================================== */

#include <stdlib.h>
#include <assert.h>

#define CS_MACROS
#include "lwmodlib.h"


int mePointSelect( LWPntID id, int select )
{
   ModData *md = getModData();

   assert(( md->edit != NULL ) && ( md->opsel & OPSEL_MODIFY ));
   return md->edit->pntSelect( md->edit->state, id, select );
}


int csSelPoint_ID( EDPointScanFunc *callback, void *userdata,
   EltOpLayer layers )
{
   int err = -1;
   ModData *md = getModData();

   assert( md->edit == NULL );

   md->opsel |= OPSEL_MODIFY;
   if ( csMeshBegin( 0, 0, md->opsel )) {
      err = mePointScan( callback, userdata, layers );
      csMeshDone( err, 0 );
   }
   md->opsel &= ~OPSEL_MODIFY;

   return err == 0;
}


int csSelPoint_Volume( char *action, double *lo, double *hi )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 4 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = action;
   argv[ 0 ].str.bufLen = 0;

   argv[ 1 ].type = DY_STRING;
   argv[ 1 ].str.buf = "VOLUME";
   argv[ 1 ].str.bufLen = 0;

   argv[ 2 ].type = DY_VFLOAT;
   argv[ 2 ].fvec.val[ 0 ] = lo[ 0 ];
   argv[ 2 ].fvec.val[ 1 ] = lo[ 1 ];
   argv[ 2 ].fvec.val[ 2 ] = lo[ 2 ];

   argv[ 3 ].type = DY_VFLOAT;
   argv[ 3 ].fvec.val[ 0 ] = hi[ 0 ];
   argv[ 3 ].fvec.val[ 1 ] = hi[ 1 ];
   argv[ 3 ].fvec.val[ 2 ] = hi[ 2 ];

   LOOKUP( "SEL_POINT" );
   EXECUTE( 4, argv );
   return OK;
}


int csSelPoint_Connect( char *action )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 2 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = action;
   argv[ 0 ].str.bufLen = 0;

   argv[ 1 ].type = DY_STRING;
   argv[ 1 ].str.buf = "CONNECT";
   argv[ 1 ].str.bufLen = 0;

   LOOKUP( "SEL_POINT" );
   EXECUTE( 2, argv );
   return OK;
}


int csSelPoint_Stat( char *action, char *stat, int npol )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 3 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = action;
   argv[ 0 ].str.bufLen = 0;

   argv[ 1 ].type = DY_STRING;
   argv[ 1 ].str.buf = stat;
   argv[ 1 ].str.bufLen = 0;

   argv[ 2 ].type = DY_INTEGER;
   argv[ 2 ].intv.value = npol;

   LOOKUP( "SEL_POINT" );
   EXECUTE( 3, argv );
   return OK;
}


int mePolygonSelect( LWPolID id, int select )
{
   ModData *md = getModData();

   assert(( md->edit != NULL ) && ( md->opsel & OPSEL_MODIFY ));
   return md->edit->polSelect( md->edit->state, id, select );
}


int csSelPolygon_ID( EDPolyScanFunc *callback, void *userdata,
   EltOpLayer layers )
{
   int err = -1;
   ModData *md = getModData();

   assert( md->edit == NULL );

   md->opsel |= OPSEL_MODIFY;
   if ( csMeshBegin( 0, 0, md->opsel )) {
      err = mePolyScan( callback, userdata, layers );
      csMeshDone( err, 0 );
   }
   md->opsel &= ~OPSEL_MODIFY;

   return err == 0;
}


int csSelPolygon_Volume( char *action, char *criterion, double *lo, double *hi )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 4 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = action;
   argv[ 0 ].str.bufLen = 0;

   argv[ 1 ].type = DY_STRING;
   argv[ 1 ].str.buf = criterion;
   argv[ 1 ].str.bufLen = 0;

   argv[ 2 ].type = DY_VFLOAT;
   argv[ 2 ].fvec.val[ 0 ] = lo[ 0 ];
   argv[ 2 ].fvec.val[ 1 ] = lo[ 1 ];
   argv[ 2 ].fvec.val[ 2 ] = lo[ 2 ];

   argv[ 3 ].type = DY_VFLOAT;
   argv[ 3 ].fvec.val[ 0 ] = hi[ 0 ];
   argv[ 3 ].fvec.val[ 1 ] = hi[ 1 ];
   argv[ 3 ].fvec.val[ 2 ] = hi[ 2 ];

   LOOKUP( "SEL_POLYGON" );
   EXECUTE( 4, argv );
   return OK;
}


int csSelPolygon_Connect( char *action )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 2 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = action;
   argv[ 0 ].str.bufLen = 0;

   argv[ 1 ].type = DY_STRING;
   argv[ 1 ].str.buf = "CONNECT";
   argv[ 1 ].str.bufLen = 0;

   LOOKUP( "SEL_POLYGON" );
   EXECUTE( 2, argv );
   return OK;
}


int csSelPolygon_Stat( char *action, char *stat, int nvert )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 3 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = action;
   argv[ 0 ].str.bufLen = 0;

   argv[ 1 ].type = DY_STRING;
   argv[ 1 ].str.buf = stat;
   argv[ 1 ].str.bufLen = 0;

   argv[ 2 ].type = DY_INTEGER;
   argv[ 2 ].intv.value = nvert;

   LOOKUP( "SEL_POLYGON" );
   EXECUTE( 3, argv );
   return OK;
}


int csSelPolygon_Surface( char *action, char *surf )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 3 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = action;
   argv[ 0 ].str.bufLen = 0;

   argv[ 1 ].type = DY_STRING;
   argv[ 1 ].str.buf = "SURFACE";
   argv[ 1 ].str.bufLen = 0;

   argv[ 2 ].type = DY_STRING;
   argv[ 2 ].str.buf = surf;
   argv[ 2 ].str.bufLen = 0;

   LOOKUP( "SEL_POLYGON" );
   EXECUTE( 3, argv );
   return OK;
}


int csSelPolygon_Faces( char *action )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 2 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = action;
   argv[ 0 ].str.bufLen = 0;

   argv[ 1 ].type = DY_STRING;
   argv[ 1 ].str.buf = "FACE";
   argv[ 1 ].str.bufLen = 0;

   LOOKUP( "SEL_POLYGON" );
   EXECUTE( 2, argv );
   return OK;
}


int csSelPolygon_Curves( char *action )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 2 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = action;
   argv[ 0 ].str.bufLen = 0;

   argv[ 1 ].type = DY_STRING;
   argv[ 1 ].str.buf = "CURVE";
   argv[ 1 ].str.bufLen = 0;

   LOOKUP( "SEL_POLYGON" );
   EXECUTE( 2, argv );
   return OK;
}


int csSelPolygon_Nonplanar( char *action, double limit )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 3 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = action;
   argv[ 0 ].str.bufLen = 0;

   argv[ 1 ].type = DY_STRING;
   argv[ 1 ].str.buf = "NONPLANAR";
   argv[ 1 ].str.bufLen = 0;

   if ( limit >= 0.0 ) {
      argv[ 2 ].type = DY_FLOAT;
      argv[ 2 ].flt.value = limit;
   }
   else argv[ 2 ].type = DY_NULL;

   LOOKUP( "SEL_POLYGON" );
   EXECUTE( 3, argv );
   return OK;
}


int csSelInvert( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );

   LOOKUP( "SEL_INVERT" );
   EXECUTE( 0, NULL );
   return OK;
}


int csSelHide( char *state )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv;

   assert( md->edit == NULL );

   argv.type = DY_STRING;
   argv.str.buf = state;
   argv.str.bufLen = 0;

   LOOKUP( "SEL_HIDE" );
   EXECUTE( 1, &argv );
   return OK;
}


int csSelUnhide( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );

   LOOKUP( "SEL_UNHIDE" );
   EXECUTE( 0, NULL );
   return OK;
}


int csInvertHide( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );

   LOOKUP( "INVERT_HIDE" );
   EXECUTE( 0, NULL );
   return OK;
}
