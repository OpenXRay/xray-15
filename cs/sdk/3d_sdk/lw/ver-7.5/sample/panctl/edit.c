/*
======================================================================
edit.c

Test the LWPanels edit field controls.

Ernie Wright  28 Jun 00
====================================================================== */

#include <lwserver.h>
#include <lwpanel.h>
#include <stdio.h>


static int
   i = 123,
   ivec[ 3 ] = { 1, 2, 3 };

static double
   f = 1.23,
   fro = 2.345,
   fvec[ 3 ] = { 1.0, 2.0, 3.0 },
   d = 1.23,
   dvec[ 3 ] = { 0.1, 10.0, 1000.0 };

static char
   str[ 80 ] = "Edit me",
   strro[ 80 ] = "I'm read-only";


int open_editpan( LWPanelFuncs *panf )
{
   LWPanControlDesc desc;
   LWValue
      ival    = { LWT_INTEGER },
      ivecval = { LWT_VINT },
      fval    = { LWT_FLOAT },
      fvecval = { LWT_VFLOAT },
      sval    = { LWT_STRING };
   LWPanelID panel;
   LWControl *ctl[ 9 ];
   int n, w, ok;

   if( !( panel = PAN_CREATE( panf, "Edit" )))
      return 0;

   ctl[ 0 ] = INT_CTL( panf, panel, "Integer" );
   ctl[ 1 ] = FLOAT_CTL( panf, panel, "Float" );
   ctl[ 2 ] = FLOATRO_CTL( panf, panel, "Read-only Float" );
   ctl[ 3 ] = DIST_CTL( panf, panel, "Distance" );
   ctl[ 4 ] = IVEC_CTL( panf, panel, "Integer vector" );
   ctl[ 5 ] = FVEC_CTL( panf, panel, "Float Vector" );
   ctl[ 6 ] = DVEC_CTL( panf, panel, "Distance Vector" );
   ctl[ 7 ] = STR_CTL( panf, panel, "String", 30 );
   ctl[ 8 ] = STRRO_CTL( panf, panel, "Read-only String", 30 );

   /* align */

   for ( n = 0; n < 9; n++ ) {
      w = CON_LW( ctl[ n ] );
      ival.intv.value = 100 - w;
      ctl[ n ]->set( ctl[ n ], CTL_X, &ival );
   }

   SET_INT( ctl[ 0 ], i );
   SET_FLOAT( ctl[ 1 ], f );
   SET_FLOAT( ctl[ 2 ], fro );
   SET_FLOAT( ctl[ 3 ], d );
   SETV_IVEC( ctl[ 4 ], ivec );
   SETV_FVEC( ctl[ 5 ], fvec );
   SETV_FVEC( ctl[ 6 ], dvec );
   SET_STR( ctl[ 7 ], str, sizeof( str ));
   SET_STR( ctl[ 8 ], strro, sizeof( strro ));

   ok = panf->open( panel, PANF_BLOCKING | PANF_CANCEL );

   if ( ok ) {
      GET_INT( ctl[ 0 ], i );
      GET_FLOAT( ctl[ 1 ], f );
      GET_FLOAT( ctl[ 2 ], fro );
      GET_FLOAT( ctl[ 3 ], d );
      GETV_IVEC( ctl[ 4 ], ivec );
      GETV_FVEC( ctl[ 5 ], fvec );
      GETV_FVEC( ctl[ 6 ], dvec );
      GET_STR( ctl[ 7 ], str, sizeof( str ));
      GET_STR( ctl[ 8 ], strro, sizeof( strro ));
   }

   PAN_KILL( panf, panel );

   return 1;
}
