/*
======================================================================
mandfilt.c

A pixel filter that draws the Mandelbrot set.

Ernie Wright  4 Apr 00
Ryan Mapes  1 Jun 00  preset shelf stuff

This plug-in turns LightWave into a Mandelbrot set renderer.  You'll
usually apply it in an empty scene.

The display is mapped onto the complex plane.  Each pixel represents
an initial value of c in the iterated function z = z^2 + c, and its
color depends on the number of iterations performed before the
magnitude of z exceeds 2.0.
====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <lwserver.h>
#include <lwfilter.h>
#include <lwxpanel.h>
#include <lwvparm.h>
#include <lwimage.h>
#include <lwshelf.h>

#define PLUGNAME "MandFilter"

#define MAX_ITERATIONS 512
#define COLOR_SPAN (MAX_ITERATIONS / 2)
#define NPALKEYS 7
#define lerp(a,b,t)  ((a) + (t) * ((b) - (a)))


/* the instance data structure */

typedef struct {
   int            iw, ih;           // pixel dimensions of the image
   double         aspect;           // pixel aspect ratio
   LWChanGroupID  cgroup;           // channel group for the parameters
   LWVParmID      vcx, vcy, vzoom;  // vparms for zoom and center x and y
   double         cx, cy, zoom;     // handy non-variant copies
   double         dx, dy;           // pixel size
   GlobalFunc    *global;           // used in NewTime()
   char           desc[ 80 ];       // description of the instance data
   LWShelfCltID   psclient;         // preset shelf client ID
   LWXPanelID     panel;            // refresh this after loading preset
} MandParam;


/* some globals */

static LWVParmFuncs    *vparmf;
static LWEnvelopeFuncs *envf;
static LWItemInfo      *iteminfo;
static LWChannelInfo   *chaninfo;
static LWXPanelFuncs   *xpanf;
static LWShelfFuncs    *shf;
static LWImageUtil     *imgutil;



/*
======================================================================
get_rgb()

A poor-man's color gradient.
====================================================================== */

static void get_rgb( float *rgb, int iterations )
{
   static LWFVector palkey[ NPALKEYS ] = {
      0.33333f, 0.0f, 0.5f,
      0.33333f, 0.0f, 0.0f,
      1.0f, 0.0f, 0.0f,
      1.0f, 1.0f, 0.0f,
      0.0f, 1.0f, 0.0f,
      0.0f, 1.0f, 1.0f,
      0.0f, 0.0f, 1.0f
   };
   float t;
   int i, p1, p2;

   t = ( float ) fmod(( double ) NPALKEYS * iterations / COLOR_SPAN,
      ( double ) NPALKEYS );
   p1 = ( int ) floor( t );
   p2 = ( p1 + 1 ) % NPALKEYS;
   t -= p1;

   for ( i = 0; i < 3; i++ )
      rgb[ i ] = lerp( palkey[ p1 ][ i ], palkey[ p2 ][ i ], t );
}


/*
======================================================================
iterate()

Calculates z = z^2 + c in a loop.
====================================================================== */

static int iterate( double cr, double ci )
{
   double zr, zi, x, y;
   int i;

   zr = zi = 0;
   for ( i = 1; i < MAX_ITERATIONS; i++ ) {
      x = cr + zr * zr - zi * zi;
      y = ci + 2 * zr * zi;
      if ( x * x + y * y >= 4.0 ) break;
      zr = x;
      zi = y;
   }
   return i;
}


/*
======================================================================
mand()

Assign a color to a pixel in a Mandelbrot set image.

INPUTS
   sx, sy      pixel coordinates
   iw, ih      image dimensions in pixels
   cx, cy      image center as a point on the complex plane
   dx, dy      pixel size on the plane
   rgb         receives the pixel color
====================================================================== */

static void mand( double sx, double sy, int iw, int ih, double cx, double cy,
   double dx, double dy, float *rgb )
{
   double cr, ci;
   int i;

   cr = cx + ( sx - ( iw >> 1 )) * dx;
   ci = cy + ( sy - ( ih >> 1 )) * dy;

   i = iterate( cr, ci );

   if ( i == MAX_ITERATIONS )
      rgb[ 0 ] = rgb[ 1 ] = rgb[ 2 ] = 0.0f;
   else
      get_rgb( rgb, i );
}


/*
======================================================================
Create()

Create and initialize an instance data structure.
====================================================================== */

XCALL_( static LWInstance )
Create( void *userdata, void *context, LWError *err )
{
   LWItemID id;
   LWChanGroupID cparent;
   MandParam *mp;

   if ( mp = calloc( 1, sizeof( MandParam ))) {
      mp->global = ( GlobalFunc * ) userdata;

      mp->cx = -0.222;
      mp->cy = 0.8;
      mp->zoom = 15.0;

      id = iteminfo->first( LWI_CAMERA, NULL );
      cparent = iteminfo->chanGroup( id );
      mp->cgroup = envf->createGroup( cparent, "MandFilter" );

      if ( mp->vcx = vparmf->create( LWVP_FLOAT, LWVPDT_NOTXTR )) {
         vparmf->setup( mp->vcx, "Center.X", mp->cgroup,
            NULL, NULL, NULL, NULL );
         vparmf->setVal( mp->vcx, &mp->cx );
      }
      if ( mp->vcy = vparmf->create( LWVP_FLOAT, LWVPDT_NOTXTR )) {
         vparmf->setup( mp->vcy, "Center.Y", mp->cgroup,
            NULL, NULL, NULL, NULL );
         vparmf->setVal( mp->vcy, &mp->cy );
      }
      if ( mp->vzoom = vparmf->create( LWVP_FLOAT, LWVPDT_NOTXTR )) {
         vparmf->setup( mp->vzoom, "Zoom", mp->cgroup,
            NULL, NULL, NULL, NULL );
         vparmf->setVal( mp->vzoom, &mp->zoom );
      }
   }

   return mp;
}


/*
======================================================================
Destroy()

Free instance data.
====================================================================== */

XCALL_( static void )
Destroy( MandParam *mp )
{
   if ( mp ) {
      if ( mp->vcx ) vparmf->destroy( mp->vcx );
      if ( mp->vcy ) vparmf->destroy( mp->vcy );
      if ( mp->vzoom ) vparmf->destroy( mp->vzoom );
      free( mp );
   }
}


/*
======================================================================
Copy()

Initialize a duplicate instance.  We can't just copy the VParmIDs.
We need to let the vparm copy() function do that for us.  We also
don't want to overwrite the channel group created in Create().
====================================================================== */

XCALL_( static LWError )
Copy( MandParam *to, MandParam *from )
{
   LWVParmID vcx, vcy, vzoom;
   LWChanGroupID cgroup;

   vcx = to->vcx;
   vcy = to->vcy;
   vzoom = to->vzoom;
   cgroup = to->cgroup;

   *to = *from;

   to->vcx = vcx;
   to->vcy = vcy;
   to->vzoom = vzoom;
   to->cgroup = cgroup;

   vparmf->copy( to->vcx, from->vcx );
   vparmf->copy( to->vcy, from->vcy );
   vparmf->copy( to->vzoom, from->vzoom );

   return NULL;
}


/*
======================================================================
Load()

Read instance data from a scene file or a preset.

In most cases you can use the same Load() to read settings from the
scene or the preset shelf, but unlike scene loading, the instance
passed for preset loading hasn't just been created by Create(), so you
may need to do some cleanup before overwriting the data.  You can do
that here, or in a shelf load callback that then calls your Load().
====================================================================== */

XCALL_( static LWError )
Load( MandParam *mp, const LWLoadState *ls )
{
   LWError err;
   double d[ 3 ];

   if ( err = vparmf->load( mp->vcx, ls )) return err;
   if ( err = vparmf->load( mp->vcy, ls )) return err;
   if ( err = vparmf->load( mp->vzoom, ls )) return err;

   vparmf->getVal( mp->vcx, 0.0, NULL, d );    mp->cx = d[ 0 ];
   vparmf->getVal( mp->vcy, 0.0, NULL, d );    mp->cy = d[ 0 ];
   vparmf->getVal( mp->vzoom, 0.0, NULL, d );  mp->zoom = d[ 0 ];

   return NULL;
}


/*
======================================================================
Save()

Store instance data in a scene file.
====================================================================== */

XCALL_( static LWError )
Save( MandParam *mp, const LWSaveState *ss )
{
   LWError err;

   if ( err = vparmf->save( mp->vcx, ss )) return err;
   if ( err = vparmf->save( mp->vcy, ss )) return err;
   if ( err = vparmf->save( mp->vzoom, ss )) return err;

   return NULL;
}


/*
======================================================================
DescLn()

Return a one-line text description of the instance data.
====================================================================== */

XCALL_( static const char * )
DescLn( MandParam *mp )
{
   sprintf( mp->desc, "MandFilt  (%g, %g)  %g", mp->cx, mp->cy, mp->zoom );
   return mp->desc;
}


/*
======================================================================
Init()

Called when rendering begins.
====================================================================== */

XCALL_( static LWError )
Init( MandParam *mp, int mode )
{
   LWSceneInfo *lws;

   if ( lws = mp->global( LWSCENEINFO_GLOBAL, GFUSE_TRANSIENT )) {
      mp->iw = lws->frameWidth;
      mp->ih = lws->frameHeight;
      mp->aspect = lws->pixelAspect;
      return NULL;
   }
   else
      return "Couldn't get image dimensions.";
}


/*
======================================================================
Cleanup()

Called when rendering ends.
====================================================================== */

XCALL_( static void )
Cleanup( MandParam *mp )
{
}


/*
======================================================================
NewTime()

Called at the start of each frame.
====================================================================== */

XCALL_( static LWError )
NewTime( MandParam *mp, LWFrame f, LWTime t )
{
   double d[ 3 ], w;

   vparmf->getVal( mp->vcx,   t, NULL, d );  mp->cx   = d[ 0 ];
   vparmf->getVal( mp->vcy,   t, NULL, d );  mp->cy   = d[ 0 ];
   vparmf->getVal( mp->vzoom, t, NULL, d );  mp->zoom = d[ 0 ];

   w = 3.2 / mp->zoom;
   mp->dx = w / mp->iw;
   mp->dy = -mp->dx / mp->aspect;

   return NULL;
}


/*
======================================================================
Evaluate()
====================================================================== */

XCALL_( static void )
Evaluate( MandParam *mp, const LWPixelAccess *pa )
{
   float out[ 4 ] = { 0.0f };

   mand( pa->sx, pa->sy, mp->iw, mp->ih, mp->cx, mp->cy, mp->dx, mp->dy, out );
   pa->setRGBA( out );
}


/*
======================================================================
Flags()

Request buffers.  We don't need anything other than the RGB.
====================================================================== */

XCALL_( static int )
Flags( MandParam *mp )
{
   return 0;
}


/*
======================================================================
Handler()

The activation function.
====================================================================== */

XCALL_( static int )
Handler( long version, GlobalFunc *global, LWPixelFilterHandler *local,
   void *serverData )
{
   if ( version != LWPIXELFILTER_VERSION ) return AFUNC_BADVERSION;

   iteminfo = global( LWITEMINFO_GLOBAL,      GFUSE_TRANSIENT );
   envf     = global( LWENVELOPEFUNCS_GLOBAL, GFUSE_TRANSIENT );
   chaninfo = global( LWCHANNELINFO_GLOBAL,   GFUSE_TRANSIENT );
   vparmf   = global( LWVPARMFUNCS_GLOBAL,    GFUSE_TRANSIENT );

   if ( !iteminfo || !envf || !chaninfo || !vparmf )
      return AFUNC_BADGLOBAL;

   local->inst->priv     = global;

   local->inst->create   = Create;
   local->inst->destroy  = Destroy;
   local->inst->copy     = Copy;
   local->inst->load     = Load;
   local->inst->save     = Save;
   local->inst->descln   = DescLn;

   local->rend->init     = Init;
   local->rend->cleanup  = Cleanup;
   local->rend->newTime  = NewTime;

   local->evaluate       = Evaluate;
   local->flags          = Flags;

   return AFUNC_OK;
}


/* ----- interface stuff ----- */


/* panel control IDs */

enum { IDCX = 0x8001, IDCY, IDZOOM, IDTHUM, IDGRP, IDSHOPEN, IDSHADD };


/*
======================================================================
ps_loadok()

Shelf callback.  This is called to determine whether it's okay to load
a preset, and whether a dialog confirming the load should be displayed
to the user.
====================================================================== */

static int ps_loadok( MandParam *mp )
{
   return SHLC_DFLT;
}


/*
======================================================================
ps_load()

Shelf callback.  Called to load a preset.
====================================================================== */

static void ps_load( MandParam *mp, const LWLoadState *ls, const char *filename,
   LWShelfParmList parms )
{
   Load( mp, ls );
   if ( mp->panel )
      xpanf->viewRefresh( mp->panel );
}


/*
======================================================================
ps_save()

Shelf callback.  Called to save a preset.
====================================================================== */

static void ps_save( MandParam *mp, const LWSaveState *ss, const char *filename )
{
   Save( mp, ss );
}


/*
======================================================================
ps_makethumb()

Create and draw a preset shelf thumbnail.
====================================================================== */

static LWPixmapID ps_makethumb( MandParam *mp, int iw, int ih )
{
   LWPixmapID image = NULL;
   double w;
   float out[ 3 ];
   int sx, sy;

   if ( imgutil ) {
      if ( image = imgutil->create( iw, ih, LWIMTYP_RGBFP )) {
         w = 3.2 / mp->zoom;
         mp->dx = w / iw;
         mp->dy = -mp->dx;

         for ( sy = 0; sy < ih; sy++ ) {
            for ( sx = 0; sx < iw; sx++ ) {
               mand( sx, sy, iw, ih, mp->cx, mp->cy, mp->dx, mp->dy, out );
               imgutil->setPixel( image, sx, sy, out );
            }
         }
      }
   }

   return image;
}


/*
======================================================================
ui_openshelf()

Button callback.  Open the preset shelf.
====================================================================== */

static void ui_openshelf( LWXPanelID panel, int cid )
{
   MandParam *mp = xpanf->getData( panel, 0 );

   if ( shf && mp->psclient )
      shf->open( mp->psclient );
}


/*
======================================================================
ui_addpreset()

Button callback.  Add a preset to the shelf.
====================================================================== */

static void ui_addpreset( LWXPanelID panel, int cid )
{
   MandParam *mp = xpanf->getData( panel, 0 );
   LWPixmapID image = NULL;

   if ( shf && mp->psclient ) {
      if ( !shf->isOpen( mp->psclient ))
         shf->open( mp->psclient );
      if ( image = ps_makethumb( mp, 128, 128 )) {
         shf->addPreset( mp->psclient, image, NULL );
         imgutil->destroy( image );
      }
   }
}


/*
======================================================================
ui_chgnotify()

Xpanels callback.  This is called for user interface events.  FORM
panels use this to update their instances.
====================================================================== */

static int ui_chgnotify( LWXPanelID panel, unsigned long cid, unsigned long vid,
   int event )
{
   int rc = LWXPRC_NONE;
   MandParam *mp = xpanf->getData( panel, 0 );
   double d[ 3 ];


   if ( event == LWXPEVENT_FOCUS ) {
      if ( shf && mp->psclient )
         shf->setContext( mp->psclient );
   }
   else if ( event == LWXPEVENT_TRACK || event == LWXPEVENT_VALUE ) {
      switch ( vid ) {
         case IDCX:
            vparmf->getVal( mp->vcx, 0, NULL, d );  mp->cx = d[ 0 ];
            rc = LWXPRC_DRAW;
            break;
         case IDCY:
            vparmf->getVal( mp->vcy, 0, NULL, d );  mp->cy = d[ 0 ];
            rc = LWXPRC_DRAW;
            break;
         case IDZOOM:
            vparmf->getVal( mp->vzoom, 0, NULL, d );  mp->zoom = d[ 0 ];
            rc = LWXPRC_DRAW;
            break;
         default:
            rc = LWXPRC_NONE;
      }
   }

   return rc;
}


/*
======================================================================
ui_dstrynotify()

Xpanels callback.  This is called after the interface is destroyed.
The argument is the pointer passed to LWXPanelFuncs->setData() for
control ID 0.
====================================================================== */

static void ui_dstrynotify( MandParam *mp )
{
   if ( shf && mp->psclient ) {
      shf->unsubscribe( mp->psclient );
      mp->psclient = NULL;
   }
}


/*
======================================================================
ui_drawthumb()

Refresh the thumbnail display.
====================================================================== */

static void ui_drawthumb( LWXPanelID panel, unsigned long cid, LWXPDrAreaID reg,
   int iw, int ih )
{
   MandParam *mp = xpanf->getData( panel, 0 );
   float out[ 3 ];
   double w;
   int sx, sy;

   mp->iw = iw;
   mp->ih = ih;
   w = 3.2 / mp->zoom;
   mp->dx = w / iw;
   mp->dy = -mp->dx;

   for ( sy = 0; sy < ih; sy++ ) {
      for ( sx = 0; sx < iw; sx++ ) {
         mand( sx, sy, iw, ih, mp->cx, mp->cy, mp->dx, mp->dy, out );
         xpanf->drawf->drawRGBPixel( reg,
            ( int )( out[ 0 ] * 255 ),
            ( int )( out[ 1 ] * 255 ),
            ( int )( out[ 2 ] * 255 ), sx, sy );
      }
   }
}


/*
======================================================================
ui_mouse()

XPanels callback.  Respond to mouse clicks and drags in the thumbnail
rectangle.

If the user clicks without dragging, the mouse point is made the new
center and the zoom is unchanged.  If the user drags, the larger of
the two rectangle dimensions determines the new zoom, and the center
of the rectangle is made the new view center.
====================================================================== */

static void ui_mouse( LWXPanelID panel, unsigned long cid, int x, int y,
   int *rect, int clickcount )
{
   MandParam *mp = xpanf->getData( panel, 0 );
   double w, d[ 3 ] = { 0.0 };
   int rw, rh;

   rw = abs( rect[ 2 ] - rect[ 0 ] ) + 1;
   rh = abs( rect[ 3 ] - rect[ 1 ] ) + 1;

   if ( rw > 2 && rh > 2 ) {
      if ( rw > rh )
         w = rw * mp->dx;
      else
         w = -rh * mp->dy;

      mp->zoom = 3.2 / w;
      mp->dx = w / mp->iw;
      mp->dy = -mp->dx;
      x = ( rect[ 2 ] > rect[ 0 ] ? rect[ 0 ] : rect[ 2 ] ) + rw / 2;
      y = ( rect[ 3 ] > rect[ 1 ] ? rect[ 1 ] : rect[ 3 ] ) + rh / 2;

      d[ 0 ] = mp->zoom;  vparmf->setVal( mp->vzoom, d );
   }

   mp->cx = mp->cx + ( x - ( mp->iw >> 1 )) * mp->dx;
   mp->cy = mp->cy + ( y - ( mp->ih >> 1 )) * mp->dy;

   d[ 0 ] = mp->cx;  vparmf->setVal( mp->vcx, d );
   d[ 0 ] = mp->cy;  vparmf->setVal( mp->vcy, d );

   xpanf->viewRefresh( panel );
}


/*
======================================================================
get_xpanel()

Create and initialize an xpanel.
====================================================================== */

static LWXPanelID get_xpanel( MandParam *mp )
{
   LWXPanelID panel;

   static LWXPanelControl ctrl_list[] = {
      { IDTHUM,   "Preview",           "dThumbnail" },
      { IDCX,     "Center X",          "float-env"  },
      { IDCY,     "Center Y",          "float-env"  },
      { IDZOOM,   "Zoom",              "float-env"  },
      { IDSHOPEN, "Open Preset Shelf", "vButton"    },
      { IDSHADD,  "Add Preset",        "vButton"    },
      { 0 }
   };

   static LWXPanelDataDesc data_descrip[] = {
      { IDCX,   "Center X", "float-env" },
      { IDCY,   "Center Y", "float-env" },
      { IDZOOM, "Zoom",     "float-env" },
      { 0 }
   };

   static LWXPanelHint hint[] = {
      XpLABEL( 0, "Mandelbrot Filter" ),
      XpCHGNOTIFY( ui_chgnotify ),
      XpDESTROYNOTIFY( ui_dstrynotify ),

      XpBUTNOTIFY( IDSHOPEN, ui_openshelf ),
      XpBUTNOTIFY( IDSHADD, ui_addpreset ),

      XpDRAWCBFUNC( IDTHUM, ui_drawthumb ),
      XpZOOMCBFUNC( IDTHUM, ui_mouse, 2 ),

      XpGROUP_( IDGRP ),
         XpH( IDTHUM ), XpH( IDCX ), XpH( IDCY ),
         XpH( IDZOOM ), XpH( IDSHOPEN ), XpH( IDSHADD ), XpEND,
      XpLEFT_(),
         XpH( IDTHUM ), XpEND,
      XpNARROW_(),
         XpH( IDCX ), XpH( IDCY ), XpH( IDZOOM ), XpEND,

      XpMIN( IDCX, -2 ),  XpMAX( IDCX, 1 ),       XpSTEP( IDCX, 1 ),
      XpMIN( IDCY, -2 ),  XpMAX( IDCY, 2 ),       XpSTEP( IDCY, 1 ),
      XpMIN( IDZOOM, 1 ), XpMAX( IDZOOM, 65536 ), XpSTEP( IDZOOM, 5 ),

      XpEND
   };


   panel = xpanf->create( LWXP_FORM, ctrl_list );
   if ( !panel ) return NULL;

   xpanf->hint( panel, 0, hint );
   xpanf->describe( panel, data_descrip, NULL, NULL );
   xpanf->setData( panel, 0, mp );

   xpanf->formSet( panel, IDCX, mp->vcx );
   xpanf->formSet( panel, IDCY, mp->vcy );
   xpanf->formSet( panel, IDZOOM, mp->vzoom );

   mp->panel = panel;

   return panel;
}


/*
======================================================================
Interface()

Interface activation function.  Create an xpanel and fill in the
fields of the LWInterface structure.
====================================================================== */

XCALL_( static int )
Interface( long version, GlobalFunc *global, LWInterface *local,
   void *serverData )
{
   MandParam *mp = ( MandParam * ) local->inst;

   if ( version != LWINTERFACE_VERSION ) return AFUNC_BADVERSION;

   xpanf = global( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT );
   if ( !xpanf ) return AFUNC_BADGLOBAL;

   shf = global( LWSHELFFUNCS_GLOBAL, GFUSE_TRANSIENT );
   imgutil = global( LWIMAGEUTIL_GLOBAL, GFUSE_TRANSIENT );

   local->panel = get_xpanel( mp );
   if ( !local->panel ) return AFUNC_BADGLOBAL;

   if ( shf )
      mp->psclient = shf->subscribe( LWPIXELFILTER_HCLASS, PLUGNAME,
         mp, SHLF_BIN, ps_loadok, ps_load, ps_save );

   local->options = NULL;
   local->command = NULL;

   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { LWPIXELFILTER_HCLASS, PLUGNAME, Handler },
   { LWPIXELFILTER_ICLASS, PLUGNAME, Interface },
   { NULL }
};
