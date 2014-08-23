/*
======================================================================
color.c

Test the LWPanels color selection controls.

Ernie Wright  28 Jun 00
====================================================================== */

#include <lwserver.h>
#include <lwpanel.h>
#include <stdio.h>
#include <stdlib.h>


static int
   rgb[ 3 ] = { 80, 160, 240 },
   minirgb[ 3 ] = { 80, 160, 240 },
   minihsv[ 3 ] = { 149, 170, 240 },
   rgbv[ 3 ] = { 80, 160, 240 };


static void draw_swatch( LWControl *ctl, int *i, DrMode mode )
{
   LWPanelFuncs *panf;
   LWPanelID panel;
   LWValue ival = { LWT_INTEGER };
   char a[ 10 ];
   int x, y, w, h;

   ctl->get( ctl, CTL_PANEL, &ival );
   panel = ( LWPanelID ) ival.ptr.ptr;

   ctl->get( ctl, CTL_PANFUN, &ival );
   panf = ( LWPanelFuncs * ) ival.ptr.ptr;

   x = CON_HOTX( ctl );
   y = CON_HOTY( ctl );
   w = CON_HOTW( ctl );
   h = CON_HOTH( ctl );

   panf->drawFuncs->drawBox( panel, SYSTEM_Ic( *i ), x, y, w, h );
   panf->drawFuncs->drawText( panel, itoa( *i, a, sizeof( a )),
      COLOR_BLACK, x + 2, y + 2 );
}


void make_swatches( LWPanelFuncs *panf, LWPanelID panel, int y )
{
   static char *label[] = { "LightWave", "Palette", NULL };
   static int index[ 48 ];
   LWPanControlDesc desc;
   LWValue ival = { LWT_INTEGER };
   LWControl *ctl;
   int i, w;

   ctl = TEXT_CTL( panf, panel, "", label );
   w = CON_W( ctl );
   MOVE_CON( ctl, 100 - w, y );

   for ( i = 0; i < 48; i++ ) {
      ctl = CANVAS_CTL( panf, panel, "", 24, 24 );
      MOVE_CON( ctl, 100 + 30 * ( i % 8 ), y + 30 * ( i / 8 ));
      ival.ptr.ptr = draw_swatch;
      ctl->set( ctl, CTL_USERDRAW, &ival );
      index[ i ] = i;
      ival.ptr.ptr = &index[ i ];
      ctl->set( ctl, CTL_USERDATA, &ival );
   }
}


int open_colorpan( LWPanelFuncs *panf )
{
   LWPanelID panel;
   LWValue
      ival    = { LWT_INTEGER },
      ivecval = { LWT_VINT };
   LWPanControlDesc desc;
   LWControl *ctl[ 5 ];
   int n, w, y, ok;

   if( !( panel = PAN_CREATE( panf, "Color" )))
      return 0;

   ctl[ 0 ] = RGB_CTL( panf, panel, "RGB" );
   ctl[ 1 ] = MINIRGB_CTL( panf, panel, "Mini RGB" );
   ctl[ 2 ] = MINIHSV_CTL( panf, panel, "Mini HSV" );
   ctl[ 3 ] = RGBVEC_CTL( panf, panel, "RGB Vector" );

   y = CON_Y( ctl[ 3 ] );
   y += CON_H( ctl[ 3 ] );
   make_swatches( panf, panel, y + 10 );

   /* align */

   for ( n = 0; n < 4; n++ ) {
      w = CON_LW( ctl[ n ] );
      ival.intv.value = 100 - w;
      ctl[ n ]->set( ctl[ n ], CTL_X, &ival );
   }

   SETV_IVEC( ctl[ 0 ], rgb );
   SETV_IVEC( ctl[ 1 ], minirgb );
   SETV_IVEC( ctl[ 2 ], minihsv );
   SETV_IVEC( ctl[ 3 ], rgbv );

   ok = panf->open( panel, PANF_BLOCKING | PANF_CANCEL );

   if ( ok ) {
      GETV_IVEC( ctl[ 0 ], rgb );
      GETV_IVEC( ctl[ 1 ], minirgb );
      GETV_IVEC( ctl[ 2 ], minihsv );
      GETV_IVEC( ctl[ 3 ], rgbv );
   }

   PAN_KILL( panf, panel );

   return 1;
}
