/*
======================================================================
text.c

Ernie Wright  7 Jan 00

This module, part of the ModLib library, contains functions for making
text.
====================================================================== */

#include <stdlib.h>
#include <assert.h>

#define CS_MACROS
#include "lwmodlib.h"


int csMakeText( char *text, int index, char *cornertype, double spacing,
   double scale, char *axis, double *pos )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 7 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = text;
   argv[ 0 ].str.bufLen = 0;

   argv[ 1 ].type = DY_INTEGER;
   argv[ 1 ].intv.value = index;

   if ( cornertype ) {
      argv[ 2 ].type = DY_STRING;
      argv[ 2 ].str.buf = cornertype;
      argv[ 2 ].str.bufLen = 0;
   }
   else argv[ 2 ].type = DY_NULL;

   if ( spacing != 0.0 ) {
      argv[ 3 ].type = DY_FLOAT;
      argv[ 3 ].flt.value = spacing;
   }
   else argv[ 3 ].type = DY_NULL;

   if ( scale != 0.0 ) {
      argv[ 4 ].type = DY_FLOAT;
      argv[ 4 ].flt.value = scale;
   }
   else argv[ 4 ].type = DY_NULL;

   if ( axis ) {
      argv[ 5 ].type = DY_STRING;
      argv[ 5 ].str.buf = axis;
      argv[ 5 ].str.bufLen = 0;
   }
   else argv[ 5 ].type = DY_NULL;

   if ( pos ) {
      argv[ 6 ].type = DY_VFLOAT;
      argv[ 6 ].fvec.val[ 0 ] = pos[ 0 ];
      argv[ 6 ].fvec.val[ 1 ] = pos[ 1 ];
      argv[ 6 ].fvec.val[ 2 ] = pos[ 2 ];
   }
   else argv[ 6 ].type = DY_NULL;

   LOOKUP( "MAKETEXT" );
   EXECUTE( 7, argv );
   return OK;
}


int mgGetFontCount( void )
{
   ModData *md = getModData();
   return md->font->count();
}


int mgGetFontIndex( char *name )
{
   ModData *md = getModData();
   return md->font->index( name );
}


const char *mgGetFontName( int index )
{
   ModData *md = getModData();
   return md->font->name( index );
}


int mgLoadFont( char *filename )
{
   ModData *md = getModData();
   return md->font->load( filename );
}


void mgClearFont( int index )
{
   ModData *md = getModData();
   md->font->clear( index );
}
