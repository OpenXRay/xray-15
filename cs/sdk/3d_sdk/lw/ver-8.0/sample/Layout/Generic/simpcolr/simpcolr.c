/*
======================================================================
simpcolr.c

Ernie Wright  15 Mar 00

A simple example of a color picker plug-in.
====================================================================== */

#include <lwserver.h>
#include <lwhost.h>
#include <lwdialog.h>
#include <lwpanel.h>
#include <stdlib.h>


static LWHotColorFunc *hotFunc;     // host's color callback
static void *hotFuncData;           // host's color callback data

static LWPanelFuncs *panf;          // panel functions
static LWPanelID panel;             // color picker's panel
static LWControl *ctl[ 2 ];         // panel controls
static LWPanControlDesc desc;       // required by macros in lwpanel.h
static LWValue
   ival = { LWT_INTEGER },
   ivecval = { LWT_VINT };


/*
======================================================================
draw_swatch()

Draws original and modified color boxes in the swatch control.
====================================================================== */

static void draw_swatch( LWControl *ectl, int *rgb, DrMode mode )
{
   int x, y, w, h, w2;
   int r0, g0, b0, r1, g1, b1;

   x = CON_HOTX( ectl );
   y = CON_HOTY( ectl );
   w = CON_HOTW( ectl );
   h = CON_HOTH( ectl );

   w2 = w / 2;
   r0 = rgb[ 0 ];
   g0 = rgb[ 1 ];
   b0 = rgb[ 2 ];
   r1 = rgb[ 3 ];
   g1 = rgb[ 4 ];
   b1 = rgb[ 5 ];

   panf->drawFuncs->drawRGBBox( panel, r0, g0, b0, x, y, w2, h );
   panf->drawFuncs->drawRGBBox( panel, r1, g1, b1, x + w2, y, w - w2, h );
}


/*
======================================================================
ivec_event()

Called when the RGB values change.  Calls draw_swatch() to update the
color swatch, and calls the host's hotFunc(), if it exists.
====================================================================== */

static void ivec_event( LWControl *ectl, int *rgb )
{
   GET_IVEC( ectl, rgb[ 3 ], rgb[ 4 ], rgb[ 5 ] );

   draw_swatch( ctl[ 1 ], rgb, 0 );

   if ( hotFunc )
      hotFunc( hotFuncData, rgb[ 3 ] / 255.0f, rgb[ 4 ] / 255.0f,
         rgb[ 5 ] / 255.0f );
}


/*
======================================================================
show_panel()

Creates and displays the color picker's panel.  Returns 1 for OK and
0 for Cancel or if an error occurs.
====================================================================== */

int show_panel( GlobalFunc *global, char *title, int *rgb )
{
   int result;

   panf = global( LWPANELFUNCS_GLOBAL, GFUSE_TRANSIENT );
   if ( !panf ) return 0;

   panf->globalFun = global;

   if( !( panel = PAN_CREATE( panf, title ))) return 0;

   ctl[ 0 ] = MINIRGB_CTL( panf, panel, "" );
   ctl[ 1 ] = CANVAS_CTL( panf, panel, "", CON_HOTW( ctl[ 0 ] ), 30 );
   MOVE_CON( ctl[ 1 ], CON_HOTX( ctl[ 0 ] ), CON_Y( ctl[ 1 ] ));

   SET_IVEC( ctl[ 0 ], rgb[ 0 ], rgb[ 1 ], rgb[ 2 ] );
   CON_SETEVENT( ctl[ 0 ], ivec_event, rgb );

   ival.ptr.ptr = draw_swatch;
   ctl[ 1 ]->set( ctl[ 1 ], CTL_USERDRAW, &ival );

   ival.ptr.ptr = rgb;
   ctl[ 1 ]->set( ctl[ 1 ], CTL_USERDATA, &ival );

   result = panf->open( panel, PANF_BLOCKING | PANF_CANCEL );

   PAN_KILL( panf, panel );
   return result;
}


/*
======================================================================
SimpleColor()

The color picker's activation function.  We don't handle RGB levels
outside the 0.0 to 1.0 range, so the input values are clamped to that
range.
====================================================================== */

XCALL_( int )
SimpleColor( long version, GlobalFunc *global, LWColorPickLocal *local,
   void *serverData )
{
   int rgb[ 6 ], i;
   char *title;

   if ( version != LWCOLORPICK_VERSION ) return AFUNC_BADVERSION;

   hotFunc = local->hotFunc;
   hotFuncData = local->data;

   if ( local->title )
      title = local->title;
   else
      title = "Simple Color";

   rgb[ 0 ] = rgb[ 3 ] = ( int )( local->red   * 255 );
   rgb[ 1 ] = rgb[ 4 ] = ( int )( local->green * 255 );
   rgb[ 2 ] = rgb[ 5 ] = ( int )( local->blue  * 255 );

   for ( i = 0; i < 3; i++ ) {
      if ( rgb[ i ] > 255 ) rgb[ i ] = rgb[ i + 3 ] = 255;
      if ( rgb[ i ] <   0 ) rgb[ i ] = rgb[ i + 3 ] = 0;
   }

   local->result = show_panel( global, title, rgb );

   if ( local->result ) {
      local->red   = rgb[ 3 ] / 255.0f;
      local->green = rgb[ 4 ] / 255.0f;
      local->blue  = rgb[ 5 ] / 255.0f;
   }

   return AFUNC_OK;
}


/*
======================================================================
SimpleColorTest()

A LayoutGeneric activation.  We can use the generic version to test
the panel without installing this as LightWave's color picker.
====================================================================== */

#include <lwgeneric.h>

XCALL_( int )
SimpleColorTest( long version, GlobalFunc *global, LWLayoutGeneric *local,
   void *serverData )
{
   int rgb[ 6 ] = { 255, 128, 64, 255, 128, 64 };

   show_panel( global, "SimpleColor Test", rgb );

   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { LWCOLORPICK_CLASS, "SimpleColor", SimpleColor },
   { LWLAYOUTGENERIC_CLASS, "SimpleColorTest", SimpleColorTest },
   { NULL }
};
