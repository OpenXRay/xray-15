/*
======================================================================
doftoy.c

The entry point and interface routines for the Depth of Field Toy
LightWave plug-in, used to explore the interaction of depth-of-field
parameters.

Ernie Wright  10 Jul 00

For information about the math involved, see the comments in func.c.
====================================================================== */

#include <lwserver.h>
#include <lwgeneric.h>
#include <lwrender.h>
#include <lwhost.h>
#include <lwpanel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "doftoy.h"


/* the seven depth of field parameters */

static double param[ 7 ] = {
   0.0,                          // near
   1.0,                          // focal distance
   0.0,                          // far
   4.0,                          // f-stop
   3.2,                          // zoom
   0.00005,                      // circle of confusion diameter
   0.015                         // aperture height
};

/* currently locked parameters */

static int
   locka = 1,
   lockb = 4;


/* scene info */

static LWLayoutGeneric *generic;
static LWItemID camid = LWITEM_NULL;
static LWTime curTime = 0.0;
static double frameAspect = 1.333;


/* LWPanels stuff */

static LWPanelFuncs *panf;       // panel functions
static LWPanelID panel;          // panel
static LWControl *pctl[ 7 ];     // param edit controls
static LWControl *lctl[ 5 ];     // param lock controls
static LWControl *dctl[ 5 ];     // param drag controls
static LWControl *actl[ 4 ];     // auxilliary controls

static LWPanControlDesc desc;    // required by macros in lwpanel.h
static LWValue
   ival    = { LWT_INTEGER },
   ivecval = { LWT_VINT    },
   sval    = { LWT_STRING  };


/*
======================================================================
get_edit()

Get the value of an edit field, given a parameter index.
====================================================================== */

static double get_edit( int i )
{
   char a[ 40 ];
   double d;
   int j;

   GET_STR( pctl[ i ], a, sizeof( a ));

   switch ( i ) {
      case 3:  d = sget_double( a );  break;
      case 4:
         GET_INT( actl[ 2 ], j );
         switch ( i ) {
            case 0:  d = sget_double( a );  break;
            case 1:  d = z_fp( sget_dist( a ), param[ 6 ] );  break;
            case 2:  d = z_hfov( sget_angle( a ), frameAspect );  break;
            case 3:  d = z_vfov( sget_angle( a ));  break;
         }
         break;
      default:  d = sget_dist( a );
   }

   return d;
}


/*
======================================================================
put_edit()

Write a parameter to its edit field, given a parameter index.
====================================================================== */

static void put_edit( int i )
{
   char a[ 40 ];
   int j;

   switch ( i ) {
      case 3:  sput_double( a, param[ 3 ] );  break;
      case 4:
         GET_INT( actl[ 2 ], j );
         switch ( j ) {
            case 0:  sput_double( a, param[ 4 ] );  break;
            case 1:  sput_dist( a, f_pz( param[ 6 ], param[ 4 ] ));  break;
            case 2:  sput_angle( a, hfov_z( param[ 4 ], frameAspect ));  break;
            case 3:  sput_angle( a, vfov_z( param[ 4 ] ));  break;
         }
         break;
      default:  sput_dist( a, param[ i ] );
   }

   SET_STR( pctl[ i ], a, sizeof( a ));
}


/*
======================================================================
update_ui()

Updates controls, usually after parameter recalculation.

The update argument contains bit fields indicating which controls
need to be updated.
====================================================================== */

static void update_ui( int update )
{
   int i;

   for ( i = 0; i < 7; i++ )
      if ( update & ( 1 << i ))
         put_edit( i );
}


/*
======================================================================
handle_drag()

Callback for the horizontal drag (slider) controls.

Updates the edit field associated with the slider and recalculates the
other parameters.

Drag event callbacks get an accumulated mouse move (the amount the
user has moved the mouse since pressing the left mouse button), but
what we want is the amount the mouse has moved only since the last
event call, so we need to do some subtracting.

dx stores the accumulated mouse move reported by the previous event.
vec[ 0 ] is the current net accumulated mouse move, and vec[ 2 ] is
the current gross (absolute value) accumulated mouse move.  The amount
moved since the last event is vec[ 0 ] - dx.

As of this writing, we encounter two problems at this point.  First,
we need to reset dx when a new mouse down occurs, but we don't get an
event for that.  If we did, we could just look for vec[ 2 ] == 0, like
we can with DRAGAREA controls.  Instead we store the previous vec[ 2 ]
in tx and reset dx when the new vec[ 2 ] is <= the old vec[ 2 ].

Second, vec[ 0 ] appears to contain a random offset.  Fortunately
vec[ 2 ] isn't similarly afflicted, so we find this bogus offset as
vec[ 0 ] - vec[ 2 ] and store it in bx whenever dx is reset.

Both of these strategies can fail in certain situations, but they
seem to work pretty well in practice.  If drag buttons are better
behaved in the future, both of these kludges will be harmless.

We use log10() and pow() to make the slider increments logarithmic.
The steps are bigger for bigger numbers and smaller for smaller ones.
We also round to the nearest step, and we don't allow the user to
slide away from invalid values.
====================================================================== */

static void handle_drag( LWControl *ctl, int *i )
{
   static int dx = 0, tx = 0, bx = 0;
   int vec[ 3 ], mx;
   double step, d, p1, p2;

   if ( param[ *i ] <= 0.0 || param[ *i ] == INVALID_NUMBER )
      return;

   GETV_IVEC( ctl, vec );
   if ( vec[ 2 ] <= tx ) {
      dx = 0;
      bx = vec[ 0 ] - vec[ 2 ];
   }

   tx = vec[ 2 ];
   vec[ 0 ] -= bx;
   mx = ( vec[ 0 ] - dx ) / 2;
   if ( mx == 0 ) return;

   p1 = log10( param[ *i ] );
   step = pow( 10.0, floor( p1 ) - 1.0 );
   d = param[ *i ] + step * mx;

   p2 = log10( d );
   if ( floor( p2 ) < floor( p1 )) {
      step /= 10.0;
      d = param[ *i ] + step * mx;
   }

   d = floor( d / step + 0.5 ) * step;
   param[ *i ] = d;
   put_edit( *i );
   update_ui( update_params( *i, param, locka, lockb ));

   dx = vec[ 0 ];
}


/*
======================================================================
handle_lock()

Callback for the boolean lock buttons.

This implements mutual exclusion in two groups.  Exactly one of the
Near/Focal Distance/Far locks and one of the F-Stop/Zoom locks can be
set at any given time.  If a button other than the current lock is
selected, the current lock is turned off and the new one made current.
If the current lock is selected, it gets turned back on (LWPanels has
just turned it off).
====================================================================== */

static void handle_lock( LWControl *ctl, int *i )
{
   int *lock;

   lock = ( *i < 3 ) ? &locka : &lockb;
   if ( *i != *lock ) {
      SET_INT( lctl[ *lock ], 0 );
      *lock = *i;
   }
   else
      SET_INT( lctl[ *i ], 1 );
}


/*
======================================================================
handle_itemlist()

Callback for the item list.  Replace the current focal distance with
the distance from the camera to the selected item.
====================================================================== */

static void handle_itemlist( LWControl *ctl, void *data )
{
   LWItemInfo *iteminfo;
   LWItemID id;
   LWDVector cpos, ipos;
   double x, y, z;

   ctl->get( ctl, CTL_VALUE, &ival );
   id = ( LWItemID ) ival.intv.value;

   if ( id != LWITEM_NULL && camid != LWITEM_NULL && id != camid ) {
      iteminfo = panf->globalFun( LWITEMINFO_GLOBAL, GFUSE_TRANSIENT );
      if ( iteminfo ) {
         iteminfo->param( id, LWIP_W_POSITION, curTime, ipos );
         iteminfo->param( camid, LWIP_W_POSITION, curTime, cpos );
         x = cpos[ 0 ] - ipos[ 0 ];
         y = cpos[ 1 ] - ipos[ 1 ];
         z = cpos[ 2 ] - ipos[ 2 ];
         param[ 1 ] = sqrt( x * x + y * y + z * z );
         update_ui( update_params( 1, param, locka, lockb ));
      }
   }
}


/*
======================================================================
handle_aphlist()

Callback for the aperture height preset list.
====================================================================== */

static void handle_aphlist( LWControl *ctl, void *data )
{
   static double aph[] = {
       4.1,  7.0, 15.0, 13.0, 13.5, 36.1, 52.0,
      13.0, 26.7, 45.0, 60.0,  3.2,  4.8,  6.4 };  /* mm */
   int i;

   GET_INT( ctl, i );

   param[ 6 ] = aph[ i ] * 0.001;
   put_edit( 6 );
   put_edit( 4 );  /* in case we're displaying the focal length */
   update_ui( update_params( 6, param, locka, lockb ));
}


/*
======================================================================
handle_zoomlist()

Callback for the zoom list, which lets the user choose how the zoom is
represented.  We just need to rewrite the edit field with the current
representation.
====================================================================== */

static void handle_zoomlist( LWControl *ctl, void *data )
{
   put_edit( 4 );
}


/*
======================================================================
handle_edit()

Callback for the parameter edit fields.
====================================================================== */

static void handle_edit( LWControl *ctl, int *i )
{
   double d;

   d = get_edit( *i );
   if ( d > 0.0 ) {
      param[ *i ] = d;
      put_edit( *i );
      update_ui( update_params( *i, param, locka, lockb ));
   }
   else
      put_edit( *i );
}


/*
======================================================================
handle_set()

Callback for the Set button.  The parameters are written back to the
scene at the current time.  The focal distance and f-stop can only be
written if Depth of Field rendering is enabled.
====================================================================== */

static void handle_set( LWControl *ctl, void *data )
{
   LWSceneInfo *sceneinfo;
   char a[ 80 ];

   sprintf( a, "ApertureHeight %g", param[ 6 ] );
   generic->evaluate( generic->data, a );
   sprintf( a, "ZoomFactor %g", param[ 4 ] );
   generic->evaluate( generic->data, a );

   sceneinfo = panf->globalFun( LWSCENEINFO_GLOBAL, GFUSE_TRANSIENT );
   if ( sceneinfo ) {
      if ( sceneinfo->renderOpts & LWROPT_DEPTHOFFIELD ) {
         sprintf( a, "FocalDistance %g", param[ 1 ] );
         generic->evaluate( generic->data, a );
         sprintf( a, "LensFStop %g", param[ 3 ] );
         generic->evaluate( generic->data, a );
      }
   }
}


/*
======================================================================
panel_draw()

Callback for drawing panel decorations.  We draw some dividers to
logically group controls.  The y coordinate for each divider is set by
get_panel() as it positions the controls.
====================================================================== */

static int divy[ 3 ];

static void panel_draw( LWPanelID panel, void *data, DrMode mode )
{
   int w, i;

   w = PAN_GETW( panf, panel );
   for ( i = 0; i < 3; i++ )
      panf->drawFuncs->drawBorder( panel, 1, 0, divy[ i ], w, 0 );
}


/*
======================================================================
get_scene()

Copy current camera's parameters for the current time.  To find the
current camera, we have to set the edit mode to Camera before calling
the Interface Info global to get the selItems[] array.
====================================================================== */

static void get_scene( GlobalFunc *global )
{
   LWSceneInfo *sceneinfo;
   LWCameraInfo *caminfo;
   LWInterfaceInfo *intinfo;
   double fl;

   generic->evaluate( generic->data, "EditCameras" );

   sceneinfo = global( LWSCENEINFO_GLOBAL, GFUSE_TRANSIENT );
   caminfo = global( LWCAMERAINFO_GLOBAL, GFUSE_TRANSIENT );
   intinfo = global( LWINTERFACEINFO_GLOBAL, GFUSE_TRANSIENT );

   if ( sceneinfo )
      frameAspect = sceneinfo->pixelAspect * sceneinfo->frameWidth
         / sceneinfo->frameHeight;

   if ( intinfo ) {
      curTime = intinfo->curTime;
      camid = intinfo->selItems[ 0 ];
   }

   if ( camid != LWITEM_NULL ) {
      param[ 1 ] = caminfo->focalDistance( camid, curTime );
      param[ 3 ] = caminfo->fStop( camid, curTime );
      param[ 4 ] = caminfo->zoomFactor( camid, curTime );
      fl = caminfo->focalLength( camid, curTime ) * 0.001;
      param[ 6 ] = p_fz( fl, param[ 4 ] );
   }

   update_ui( 0xFA );
   update_ui( update_params( 1, param, 1, lockb ));
}


/*
======================================================================
align_control()

Left-justify the control's hot rectangle.
====================================================================== */

static void align_control( LWControl *ctl, int x, int y )
{
   int lw, lh;

   lw  = CON_HOTX( ctl );
   lw -= CON_X( ctl );
   lh  = CON_HOTY( ctl );
   lh -= CON_Y( ctl );
   MOVE_CON( ctl, x - lw, y - lh );
}


/*
======================================================================
get_panel()

Create the user interface.
====================================================================== */

#define VSPACE0 10
#define VSPACE1 3
#define VSPACE2 15
#define HSPACE0 10
#define HSPACE1 3

static int get_panel( void )
{
   static char *label[] = { "Near", "Focal Distance", "Far", "F-Stop", "" };
   static char *zoomlist[] = {
      "Zoom Factor",
      "Lens Focal Length",
      "Horizontal FOV",
      "Vertical FOV",
      NULL
   };
   static char *camlist[] = {
      "Super 8 mm motion picture",
      "16 mm motion picture",
      "35 mm motion picture",
      "Super 35 1.78:1 (3 perf)",
      "Super 35 1.78:1 (4 perf)",
      "65 mm Super Panavision motion picture",
      "65 mm Imax motion picture",
      "Size 110 (pocket camera)",
      "Size 135 (35 mm SLR)",
      "Size 120 (60 x 45 mm rollfilm)",
      "Size 120 (90 x 60 mm rollfilm)",
      "1/3\" CCD video camera",
      "1/2\" CCD video camera",
      "2/3\" CCD video camera",
      NULL
   };
   static int pindex[ 7 ] = { 0, 1, 2, 3, 4, 5, 6 };
   int i, pw, x, y, dx, dx2, dy;


   /* create the controls */

   pctl[ 5 ] = STR_CTL( panf, panel, "Circle of Confusion", 17 );
   if ( !pctl[ 5 ] ) return 0;

   dx = CON_HOTW( pctl[ 5 ] );
   dy = CON_HOTH( pctl[ 5 ] );
   divy[ 0 ] = VSPACE0 + dy + VSPACE2 / 2;

   for ( i = 0; i < 5; i++ ) {
      pctl[ i ] = STR_CTL( panf, panel, label[ i ], 17 );
      lctl[ i ] = BOOL_CTL( panf, panel, "" );
      dctl[ i ] = HDRAGBUT_CTL( panf, panel, "" );
   }

   pctl[ 6 ] = STR_CTL( panf, panel, "Aperture Height", 17 );
   actl[ 0 ] = POPDOWN_CTL( panf, panel, "Presets", camlist );
   actl[ 1 ] = PIKITEM_CTL( panf, panel, "To", panf->globalFun, LWI_ANY, 56 );
   actl[ 2 ] = WPOPUP_CTL( panf, panel, "", zoomlist, 140 );
   actl[ 3 ] = WBUTTON_CTL( panf, panel, "Set", dx );


   /* see whether we got 'em all */

   for ( i = 0; i < 5; i++ )
      if ( !pctl[ i ] || !lctl[ i ] || !dctl[ i ] ) return 0;
   if ( !pctl[ 6 ] ) return 0;
   for ( i = 0; i < 4; i++ )
      if ( !actl[ i ] ) return 0;


   /* position the controls */

   dx2 = CON_HOTW( lctl[ 0 ] );
   pw = CON_HOTW( actl[ 2 ] ) + dx + dx2;
   pw += CON_HOTW( dctl[ 0 ] ) + 2 * HSPACE0 + 3 * HSPACE1;
   x = CON_HOTW( actl[ 2 ] ) + HSPACE0 + HSPACE1;
   y = VSPACE0;

   align_control( pctl[ 5 ], x, y );
   y += dy + VSPACE2;

   for ( i = 0; i < 5; i++ ) {
      align_control( pctl[ i ], x, y );
      align_control( lctl[ i ], x + dx + HSPACE1, y );
      align_control( dctl[ i ], x + dx + dx2 + HSPACE1, y );
      if ( i == 1 )
         align_control( actl[ 1 ], HSPACE0, y );
      if ( i == 4 )
         align_control( actl[ 2 ], HSPACE0, y );
      if ( i == 2 ) {
         divy[ 1 ] = y + dy + VSPACE2 / 2;
         y += dy + VSPACE2;
      }
      else
         y += dy + VSPACE1;
   }

   align_control( pctl[ 6 ], x, y );

   y += dy + VSPACE1;
   align_control( actl[ 0 ], x, y );

   divy[ 2 ] = y + dy + VSPACE2 / 2;
   y += dy + VSPACE2;
   align_control( actl[ 3 ], x, y );


   /* size the panel, set its draw callback */

   PAN_SETH( panf, panel, y + dy + 3 * VSPACE2 );
   PAN_SETDRAW( panf, panel, panel_draw );


   /* set the control callbacks */

   for ( i = 0; i < 5; i++ ) {
      CON_SETEVENT( pctl[ i ], handle_edit, &pindex[ i ] );
      CON_SETEVENT( lctl[ i ], handle_lock, &pindex[ i ] );
      CON_SETEVENT( dctl[ i ], handle_drag, &pindex[ i ] );
   }
   CON_SETEVENT( pctl[ 5 ], handle_edit, &pindex[ 5 ] );
   CON_SETEVENT( pctl[ 6 ], handle_edit, &pindex[ 6 ] );

   CON_SETEVENT( actl[ 0 ], handle_aphlist, NULL );
   CON_SETEVENT( actl[ 1 ], handle_itemlist, NULL );
   CON_SETEVENT( actl[ 2 ], handle_zoomlist, NULL );
   CON_SETEVENT( actl[ 3 ], handle_set, NULL );


   /* initialize the control values */

   SET_INT( lctl[ locka ], 1 );
   SET_INT( lctl[ lockb ], 1 );

   get_scene( panf->globalFun );


   /* done */

   return 1;
}


/*
======================================================================
DOFToy()

Plug-in activation function.
====================================================================== */

XCALL_( int )
DOFToy( long version, GlobalFunc *global, LWLayoutGeneric *local,
   void *serverData )
{
   if ( version != LWLAYOUTGENERIC_VERSION )
      return AFUNC_BADVERSION;

   generic = local;
   panf = global( LWPANELFUNCS_GLOBAL, GFUSE_TRANSIENT );

   if ( !panf ) return AFUNC_BADGLOBAL;
   panf->globalFun = global;
   if( !( panel = PAN_CREATE( panf, "Depth of Field Toy" )))
      return AFUNC_BADGLOBAL;

   if ( get_panel() )
      panf->open( panel, PANF_BLOCKING );
   PAN_KILL( panf, panel );

   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { "LayoutGeneric", "DOF_Toy", DOFToy },
   { NULL }
};
