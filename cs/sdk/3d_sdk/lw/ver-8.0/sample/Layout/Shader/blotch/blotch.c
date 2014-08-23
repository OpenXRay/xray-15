/*
======================================================================
blotch.c

Stick a colored spot on a surface.

Allen Hastings, Arnie Cachelin, Stuart Ferguson, Ernie Wright
6 April 00

** Note:  The first release of LW 6.0 will crash on exit if this
plug-in's interface has been opened during that session.  The crash
is related to xpanel destroy processing.  It shouldn't cause loss of
data, and it should be resolved in later builds.
====================================================================== */

#include <lwserver.h>
#include <lwshader.h>
#include <lwsurf.h>
#include <lwhost.h>
#include <lwxpanel.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifndef PI
#define PI 3.1415926535897932384
#endif


/* our instance data */

typedef struct st_Blotch {
   double color[ 3 ];
   double center[ 3 ];
   double radius;
   double softness;
   double r2, piOverR;
   char   desc[ 80 ];
} Blotch;


/*
======================================================================
Create()

Handler callback.  Allocate and initialize instance data.

The create function allocates a blotch struct and returns the pointer
as the instance.  Note that "Blotch *" is used throughout instead of
"LWInstance".  This works since a LWInstance type is a generic pointer
and can safely be replaced with any specific pointer type.  Instance
variables are initialized to some default values.
====================================================================== */

XCALL_( static LWInstance )
Create( void *priv, LWSurfaceID surf, LWError *err )
{
   Blotch *inst;

   inst = calloc( 1, sizeof( Blotch ));
   if ( !inst ) {
      *err = "Couldn't allocate memory for instance.";
      return NULL;
   }

   inst->color[ 0 ] = 0.4;
   inst->color[ 1 ] = 0.0;
   inst->color[ 2 ] = 0.8;
   inst->radius = 1.0;
   inst->softness = 0.5;

   return inst;
}


/*
======================================================================
Destroy()

Handler callback.  Free resources allocated by Create().
====================================================================== */

XCALL_( static void )
Destroy( Blotch *inst )
{
   free( inst );
}


/*
======================================================================
Copy()

Handler callback.  Copy instance data.
====================================================================== */

XCALL_( static LWError )
Copy( Blotch *to, Blotch *from )
{
   XCALL_INIT;

   *to = *from;
   return NULL;
}


/*
======================================================================
Load()

Handler callback.  Read instance data.  Shader instance data is stored
in the SURF chunks of object files, but it isn't necessary to know
that to read and write the data.
====================================================================== */

XCALL_( static LWError )
Load( Blotch *inst, const LWLoadState *ls )
{
   float f[ 3 ];

   LWLOAD_FP( ls, f, 3 );
   inst->color[ 0 ] = f[ 0 ];
   inst->color[ 1 ] = f[ 1 ];
   inst->color[ 2 ] = f[ 2 ];

   LWLOAD_FP( ls, f, 3 );
   inst->center[ 0 ] = f[ 0 ];
   inst->center[ 1 ] = f[ 1 ];
   inst->center[ 2 ] = f[ 2 ];

   LWLOAD_FP( ls, f, 1 );
   inst->radius = f[ 0 ];

   LWLOAD_FP( ls, f, 1 );
   inst->softness = f[ 0 ];

   return NULL;
}


/*
======================================================================
Save()

Handler callback.  Write instance data.  The I/O functions in lwio.h
include one for reading and writing floats, but not doubles.  We just
transfer our double-precision data to a float variable before calling
the LWSAVE_FP() macro.
====================================================================== */

XCALL_( static LWError )
Save( Blotch *inst, const LWSaveState *ss )
{
   float f[ 3 ];

   f[ 0 ] = ( float ) inst->color[ 0 ];
   f[ 1 ] = ( float ) inst->color[ 1 ];
   f[ 2 ] = ( float ) inst->color[ 2 ];
   LWSAVE_FP( ss, f, 3 );

   f[ 0 ] = ( float ) inst->center[ 0 ];
   f[ 1 ] = ( float ) inst->center[ 1 ];
   f[ 2 ] = ( float ) inst->center[ 2 ];
   LWSAVE_FP( ss, f, 3 );

   f[ 0 ] = ( float ) inst->radius;
   LWSAVE_FP( ss, f, 1 );

   f[ 0 ] = ( float ) inst->softness;
   LWSAVE_FP( ss, f, 1 );

   return NULL;
}


/*
======================================================================
DescLn()

Handler callback.  Write a one-line text description of the instance
data.  Since the string must persist after this is called, it's part
of the instance.
====================================================================== */

XCALL_( static const char * )
DescLn( Blotch *inst )
{
   sprintf( inst->desc, "Blotch  (%0.2f %.2f %.02f)",
      inst->color[ 0 ],
      inst->color[ 1 ],
      inst->color[ 2 ] );

   return inst->desc;
}


/*
======================================================================
Init()

Handler callback, called at the start of rendering.  We do a little
precalculation here.
====================================================================== */

XCALL_( static LWError )
Init( Blotch *inst, int mode )
{
   inst->r2      = inst->radius * inst->radius;
   inst->piOverR = PI / inst->radius;

   return NULL;
}


/*
======================================================================
Cleanup()

Handler callback, called at the end of rendering.  We don't have
anything to do, but it's here in case we want to add something later.
====================================================================== */

XCALL_( static void )
Cleanup( Blotch *inst )
{
   return;
}


/*
======================================================================
NewTime()

Handler callback, called at the start of each sampling pass.
====================================================================== */

XCALL_( static LWError )
NewTime( Blotch *inst, LWFrame f, LWTime t )
{
   return NULL;
}


/*
======================================================================
Flags()

Handler callback.  Blotch alters the color of the surface, but nothing
else, so we return just the color bit.
====================================================================== */

XCALL_( static unsigned int )
Flags( Blotch *inst )
{
   return LWSHF_COLOR;
}


/*
======================================================================
Evaluate()

Handler callback.  This is where the blotchiness actually happens.  We
compute the distance from the spot to be shaded to the center of the
blotch and blend some of the blotch color with the color already
computed for that spot.
====================================================================== */

XCALL_( static void )
Evaluate( Blotch *inst, LWShaderAccess *sa )
{
   double d, r2, a;
   int i;

   /* Compute the distance from the center of the blotch to the spot
      in object coordinates.  Exit early if the spot is clearly
      outside the blotch radius. */

   r2 = 0;
   for ( i = 0; i < 3; i++ ) {
      d = sa->oPos[ i ] - inst->center[ i ];
      d = d * d;
      if ( d > inst->r2 ) return;
      r2 += d;
   }
   if ( r2 > inst->r2 ) return;

   d = sqrt( r2 );
   if ( d > inst->radius ) return;

   /* Using the distance in 'd', compute where this spot falls in the
      blotch's soft edge.  The blotch is given by a cosine density
      function scaled by the softness factor.  Where the density is
      greater than 1.0, it clips. */

   d = pow( 0.5 * ( 1.0 + cos( d * inst->piOverR )), inst->softness );

   /* Finally, blend the blotch color into the existing color using
      the computed density. */

   a = 1.0 - d;
   for ( i = 0; i < 3; i++ )
      sa->color[ i ] = sa->color[ i ] * a + inst->color[ i ] * d;
}


/*
======================================================================
Handler()

Handler activation function.  Check the version and fill in the
callback fields of the handler structure.
====================================================================== */

XCALL_( static int )
Handler( long version, GlobalFunc *global, LWShaderHandler *local,
   void *serverData)
{
   if ( version != LWSHADER_VERSION ) return AFUNC_BADVERSION;

   local->inst->create   = Create;
   local->inst->destroy  = Destroy;
   local->inst->load     = Load;
   local->inst->save     = Save;
   local->inst->copy     = Copy;
   local->inst->descln   = DescLn;
   local->rend->init     = Init;
   local->rend->cleanup  = Cleanup;
   local->rend->newTime  = NewTime;
   local->evaluate       = Evaluate;
   local->flags          = Flags;

   return AFUNC_OK;
}


/* interface stuff ----- */

static LWXPanelFuncs *xpanf;
static LWColorActivateFunc *colorpick;
static LWInstUpdate *lwupdate;

enum { ID_COLOR = 0x8001, ID_CENTER, ID_RADIUS, ID_SOFTNESS };


/*
======================================================================
handle_color()

Event callback for the color button.  Called by LWXPanels.  Opens the
user's installed color picker.
====================================================================== */

static void handle_color( LWXPanelID panel, int cid )
{
   LWColorPickLocal local;
   Blotch *inst;
   int result;

   inst = xpanf->getData( panel, 0 );

   local.result  = 0;
   local.title   = "Blotch Color";
   local.red     = ( float ) inst->color[ 0 ];
   local.green   = ( float ) inst->color[ 1 ];
   local.blue    = ( float ) inst->color[ 2 ];
   local.data    = NULL;
   local.hotFunc = NULL;

   result = colorpick( LWCOLORPICK_VERSION, &local );
   if ( result == AFUNC_OK && local.result > 0 ) {
      inst->color[ 0 ] = local.red;
      inst->color[ 1 ] = local.green;
      inst->color[ 2 ] = local.blue;
      lwupdate( LWSHADER_HCLASS, inst );
   }
}


/*
======================================================================
ui_get()

Xpanels callback for LWXP_VIEW panels.  Returns a pointer to the data
for a given control value.
====================================================================== */

void *ui_get( Blotch *inst, unsigned long vid )
{
   switch ( vid ) {
      case ID_CENTER:    return inst->center;
      case ID_RADIUS:    return &inst->radius;
      case ID_SOFTNESS:  return &inst->softness;
      default:           return NULL;
   }
}


/*
======================================================================
ui_set()

Xpanels callback for LWXP_VIEW panels.  Store a value in our instance
data.
====================================================================== */

int ui_set( Blotch *inst, unsigned long vid, void *value )
{
   double *d = ( double * ) value;

   switch ( vid ) {
      case ID_CENTER:
         inst->center[ 0 ] = d[ 0 ];
         inst->center[ 1 ] = d[ 1 ];
         inst->center[ 2 ] = d[ 2 ];
         break;

      case ID_RADIUS:
         inst->radius = *d;
         break;

      case ID_SOFTNESS:
         inst->softness = *d;
         break;

      default:
         return 0;
   }

   return 1;
}


/*
======================================================================
ui_chgnotify()

XPanel callback.  XPanels calls this when an event occurs that affects
the value of one of your controls.  We use the instance update global
to tell Layout that our instance data has changed.
====================================================================== */

void ui_chgnotify( LWXPanelID panel, unsigned long cid, unsigned long vid,
   int event )
{
   void *dat;

   if ( event == LWXPEVENT_VALUE )
      if ( dat = xpanf->getData( panel, 0 ))
         lwupdate( LWSHADER_HCLASS, dat );
}


/*
======================================================================
get_panel()

Create and initialize an LWXP_VIEW panel.  Called by Interface().
====================================================================== */

LWXPanelID get_panel( Blotch *inst )
{
   static LWXPanelControl xctl[] = {
      { ID_COLOR,    "Color",    "vButton",  },
      { ID_CENTER,   "Center",   "distance3" },
      { ID_RADIUS,   "Radius",   "distance"  },
      { ID_SOFTNESS, "Softness", "float"     },
      { 0 }
   };
   static LWXPanelDataDesc xdata[] = {
      { ID_CENTER,   "Center",   "distance3" },
      { ID_RADIUS,   "Radius",   "distance"  },
      { ID_SOFTNESS, "Softness", "float"     },
      { 0 }
   };
   static LWXPanelHint xhint[] = {
      XpLABEL( 0, "Blotch" ),
      XpBUTNOTIFY( ID_COLOR, handle_color ),
      XpCHGNOTIFY( ui_chgnotify ),
      XpEND
   };

   LWXPanelID panel;

   if ( panel = xpanf->create( LWXP_VIEW, xctl )) {
      xpanf->hint( panel, 0, xhint );
      xpanf->describe( panel, xdata, ui_get, ui_set );
      xpanf->viewInst( panel, inst );
      xpanf->setData( panel, 0, inst );
   }

   return panel;
}


/*
======================================================================
Interface()

The interface activation function.
====================================================================== */

XCALL_( int )
Interface( long version, GlobalFunc *global, LWInterface *local,
   void *serverData )
{
   if ( version != LWINTERFACE_VERSION ) return AFUNC_BADVERSION;

   colorpick = global( LWCOLORACTIVATEFUNC_GLOBAL, GFUSE_TRANSIENT );
   lwupdate  = global( LWINSTUPDATE_GLOBAL,        GFUSE_TRANSIENT );
   xpanf     = global( LWXPANELFUNCS_GLOBAL,       GFUSE_TRANSIENT );
   if ( !colorpick || !lwupdate || !xpanf ) return AFUNC_BADGLOBAL;

   local->panel   = get_panel( local->inst );
   local->options = NULL;
   local->command = NULL;

   return local->panel ? AFUNC_OK : AFUNC_BADGLOBAL;
}


ServerRecord ServerDesc[] = {
   { LWSHADER_HCLASS, "Demo_Blotch", Handler },
   { LWSHADER_ICLASS, "Demo_Blotch", Interface },
   { NULL }
};
