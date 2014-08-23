/*
======================================================================
slider.c

Test the LWPanels slider-like controls.

Ernie Wright  28 Jun 00
====================================================================== */

#include <lwserver.h>
#include <lwpanel.h>
#include <stdio.h>


static int slideval[ 5 ] = { 10, 10, 10, 10, 10 };
static double pct = 15.2;
static double angle = 1.0471976;   /* radians! */


static void mevent( LWPanelID panel, LWPanelFuncs *panf, int code, int x, int y )
{
   static int down = 0;
   const LWDisplayMetrics *dmet;
   LWValue
      ival = { LWT_INTEGER },
      ivecval = { LWT_VINT };


   if ( !( code & MOUSE_DOWN )) return;
   down = !down;

   dmet = panf->drawFuncs->dispMetrics();

   /* mouse drag coordinates */

   panf->drawFuncs->drawBox( panel, down ? COLOR_LT_YELLOW : COLOR_BLACK, 0, 0,
      10, 10 );
}


/*
======================================================================
devent()

Event callback for the DRAG button controls.  We draw the mouse
coordinates.
====================================================================== */

static void devent( LWControl *ctl, void *data )
{
   LWPanelFuncs *panf;
   const LWDisplayMetrics *dmet;
   LWPanelID panel;
   LWValue
      ival = { LWT_INTEGER },
      ivecval = { LWT_VINT };
   int x, y, md[ 3 ];
   char a[ 32 ];


   /* panel, panf, dmet */

   panel = ( LWPanelID ) CON_PAN( ctl );
   panf = ( LWPanelFuncs * ) CON_PANFUN( ctl );
   dmet = panf->drawFuncs->dispMetrics();

   /* control dimensions */

   x = CON_HOTX( ctl );
   y = CON_HOTY( ctl );

   /* mouse drag coordinates */

   GETV_IVEC( ctl, md );
   sprintf( a, "%d %d %d", md[ 0 ], md[ 1 ], md[ 2 ] );
   panf->drawFuncs->drawBox( panel, COLOR_LT_YELLOW, x - 90, y, 80,
      dmet->textHeight );
   panf->drawFuncs->drawText( panel, a, COLOR_BLACK, x - 88, y );
}


/*
======================================================================
sevent()

Event callback for the HSLIDER and VSLIDER controls.  We draw the
mouse coordinates.
====================================================================== */

static void sevent( LWControl *ctl, void *data )
{
   LWPanelFuncs *panf;
   const LWDisplayMetrics *dmet;
   LWPanelID panel;
   LWValue
      ival = { LWT_INTEGER },
      ivecval = { LWT_VINT };
   int x, y, w, h, m;
   char a[ 32 ];


   /* panel, panf, dmet */

   panel = ( LWPanelID ) CON_PAN( ctl );
   panf = ( LWPanelFuncs * ) CON_PANFUN( ctl );
   dmet = panf->drawFuncs->dispMetrics();

   /* control dimensions */

   x = CON_HOTX( ctl );
   y = CON_HOTY( ctl );
   w = CON_HOTW( ctl );
   h = CON_HOTH( ctl );
   x += w;
   y += ( h - dmet->textHeight ) / 2;

   /* mouse drag coordinates */

   GET_INT( ctl, m );
   sprintf( a, "%d", m );
   panf->drawFuncs->drawBox( panel, COLOR_LT_YELLOW, x + 10, y, 40,
      dmet->textHeight );
   panf->drawFuncs->drawText( panel, a, COLOR_BLACK, x + 12, y );
}


/*
======================================================================
open_sliderpan()

Open the panel.
====================================================================== */

int open_sliderpan( LWPanelFuncs *panf )
{
   LWPanControlDesc desc;
   LWValue
      ival = { LWT_INTEGER },
      fval = { LWT_FLOAT };
   LWPanelID panel;
   LWControl *ctl[ 10 ];
   int n, w, ok;

   if( !( panel = PAN_CREATE( panf, "Slider" )))
      return 0;

   panf->set( panel, PAN_USERDATA, panf );
   panf->set( panel, PAN_MOUSEBUTTON, mevent );
   panf->set( panel, PAN_MOUSEMOVE, mevent );

   ctl[ 0 ] = SLIDER_CTL( panf, panel, "Slider", 100, -20, 100 );
   ctl[ 1 ] = VSLIDER_CTL( panf, panel, "Vertical Slider", 60, 0, 100 );
   ctl[ 2 ] = HSLIDER_CTL( panf, panel, "Horizontal Slider", 100, -20, 100 );
   ctl[ 3 ] = UNSLIDER_CTL( panf, panel, "Unbounded Slider", 100, -20, 100 );
   ctl[ 4 ] = MINISLIDER_CTL( panf, panel, "Minislider", 100, -20, 100 );
   ctl[ 5 ] = PERCENT_CTL( panf, panel, "Percent" );
   ctl[ 6 ] = ANGLE_CTL( panf, panel, "Angle" );
   ctl[ 7 ] = DRAGBUT_CTL( panf, panel, "Drag Button", 40, 40 );
   ctl[ 8 ] = VDRAGBUT_CTL( panf, panel, "Vertical Drag Button" );
   ctl[ 9 ] = HDRAGBUT_CTL( panf, panel, "Horizontal Drag Button" );

   CON_SETEVENT( ctl[ 1 ], sevent, NULL );
   CON_SETEVENT( ctl[ 2 ], sevent, NULL );

   CON_SETEVENT( ctl[ 7 ], devent, NULL );
   CON_SETEVENT( ctl[ 8 ], devent, NULL );
   CON_SETEVENT( ctl[ 9 ], devent, NULL );

   /* align */

   for ( n = 0; n < 10; n++ ) {
      w = CON_LW( ctl[ n ] );
      ival.intv.value = 100 - w;
      ctl[ n ]->set( ctl[ n ], CTL_X, &ival );
   }

   for ( n = 0; n < 5; n++ )
      SET_INT( ctl[ n ], slideval[ n ] );

   SET_FLOAT( ctl[ 5 ], pct );
   SET_FLOAT( ctl[ 6 ], angle );

   ok = panf->open( panel, PANF_BLOCKING | PANF_CANCEL | PANF_MOUSETRAP );

   if ( ok ) {
      for ( n = 0; n < 5; n++ )
         GET_INT( ctl[ n ], slideval[ n ] );

      GET_FLOAT( ctl[ 5 ], pct );
      GET_FLOAT( ctl[ 6 ], angle );
   }

   PAN_KILL( panf, panel );

   return 1;
}
