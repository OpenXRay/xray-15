/*
======================================================================
blotchv.c

Stick a colored spot on a surface.

Allen Hastings, Arnie Cachelin, Stuart Ferguson, Ernie Wright
27 Nov 00

This version uses variant parameters and the preset shelf.
====================================================================== */

#include <lwserver.h>
#include <lwshader.h>
#include <lwsurf.h>
#include <lwhost.h>
#include <lwxpanel.h>
#include <lwvparm.h>
#include <lwshelf.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifndef PI
#define PI 3.1415926535897932384
#endif


/* our instance data */

typedef struct st_Blotch {
   double         color[ 3 ];
   double         center[ 3 ];
   double         radius;
   double         softness;
   double         r2, piOverR;
   char           desc[ 80 ];
   LWVParmID      vcolor, vcenter, vradius, vsoftness;
   LWChanGroupID  cgroup;
   LWShelfCltID   psclient;
   LWXPanelID     panel;
} Blotch;


LWEnvelopeFuncs *envf;
LWSurfaceFuncs  *surff;
LWVParmFuncs    *vparmf;
LWImageUtil     *imgutil;


/*
======================================================================
create_vpenv()

Create variant parameters for envelopeable settings.

This is called from our create() callback to create and initialize the
vparms in our instance data.  We only support envelopes for these, not
textures.
====================================================================== */

LWVParmID create_vpenv( int type, char *name, LWChanGroupID group,
   double *ival )
{
   LWVParmID vp;

   if ( vp = vparmf->create( type, LWVPDT_NOTXTR )) {
      vparmf->setup( vp, name, group, NULL, NULL, NULL, NULL );
      vparmf->setVal( vp, ival );
   }
   return vp;
}


/*
======================================================================
get_vpvalues()

Get the values of our instance parameters at a given time.
====================================================================== */

void get_vpvalues( Blotch *inst, LWTime t )
{
   double d[ 3 ];

   vparmf->getVal( inst->vcolor, t, NULL, d );
   inst->color[ 0 ] = d[ 0 ];
   inst->color[ 1 ] = d[ 1 ];
   inst->color[ 2 ] = d[ 2 ];

   vparmf->getVal( inst->vcenter, t, NULL, d );
   inst->center[ 0 ] = d[ 0 ];
   inst->center[ 1 ] = d[ 1 ];
   inst->center[ 2 ] = d[ 2 ];

   vparmf->getVal( inst->vradius, t, NULL, d );
   inst->radius = d[ 0 ];

   vparmf->getVal( inst->vsoftness, t, NULL, d );
   inst->softness = d[ 0 ];
}


/*
======================================================================
Create()

Handler callback.  Allocate and initialize instance data.

The create function allocates a Blotch struct and returns the pointer
as the instance.  Instance variables are initialized to some default
values.
====================================================================== */

XCALL_( static LWInstance )
Create( void *priv, LWSurfaceID surf, LWError *err )
{
   LWChanGroupID cparent;
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

   cparent = surff->chanGrp( surf );
   inst->cgroup = envf->createGroup( cparent, "Blotch" );

   inst->vcolor = create_vpenv( LWVP_COLOR, "Color",
      inst->cgroup, inst->color );
   inst->vcenter = create_vpenv( LWVP_FLOAT | LWVPF_VECTOR, "Center",
      inst->cgroup, inst->center );
   inst->vradius = create_vpenv( LWVP_FLOAT, "Radius",
      inst->cgroup, &inst->radius );
   inst->vsoftness = create_vpenv( LWVP_FLOAT, "Softness",
      inst->cgroup, &inst->softness );

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
   if ( inst ) {
      if ( inst->vcolor ) vparmf->destroy( inst->vcolor );
      if ( inst->vcenter ) vparmf->destroy( inst->vcenter );
      if ( inst->vradius ) vparmf->destroy( inst->vradius );
      if ( inst->vsoftness ) vparmf->destroy( inst->vsoftness );
      free( inst );
   }
}


/*
======================================================================
Copy()

Handler callback.  Copy instance data.

Create() has already been called for the to instance, so most of the
instance has been initialized, or will be later.  We just need to copy
the values of our vparms.
====================================================================== */

XCALL_( static LWError )
Copy( Blotch *to, Blotch *from )
{
   vparmf->copy( to->vcolor, from->vcolor );
   vparmf->copy( to->vcenter, from->vcenter );
   vparmf->copy( to->vradius, from->vradius );
   vparmf->copy( to->vsoftness, from->vsoftness );

   return NULL;
}


/*
======================================================================
Load()

Handler callback.  Read instance data.  This is used to read our data
both from the SURF chunks of object files and from preset shelf files.
In the latter case, the instance hasn't just been created by Create(),
so if there are any allocated resources in your instance, you may need
to free those before overwriting the data.  You can do that here, or
in a shelf load callback that then calls Load().
====================================================================== */

XCALL_( static LWError )
Load( Blotch *inst, const LWLoadState *ls )
{
   LWError err;

   if ( err = vparmf->load( inst->vcolor, ls )) return err;
   if ( err = vparmf->load( inst->vcenter, ls )) return err;
   if ( err = vparmf->load( inst->vradius, ls )) return err;
   if ( err = vparmf->load( inst->vsoftness, ls )) return err;

   get_vpvalues( inst, 0.0 );

   return NULL;
}


/*
======================================================================
Save()

Handler callback.  Write instance data.  Used for both objects and
presets.
====================================================================== */

XCALL_( static LWError )
Save( Blotch *inst, const LWSaveState *ss )
{
   LWError err;

   if ( err = vparmf->save( inst->vcolor, ss )) return err;
   if ( err = vparmf->save( inst->vcenter, ss )) return err;
   if ( err = vparmf->save( inst->vradius, ss )) return err;
   if ( err = vparmf->save( inst->vsoftness, ss )) return err;

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

Handler callback, called at the start of rendering.
====================================================================== */

XCALL_( static LWError )
Init( Blotch *inst, int mode )
{
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

Handler callback, called at the start of each sampling pass.  We grab
the parameter values for the current time and precompute a couple of
values that are constant for all evaluations at a given time.

Note that this is done in Init() in the non-vparm Blotch, since there
the parameters don't vary with time.
====================================================================== */

XCALL_( static LWError )
NewTime( Blotch *inst, LWFrame f, LWTime t )
{
   get_vpvalues( inst, t );
   inst->r2      = inst->radius * inst->radius;
   inst->piOverR = PI / inst->radius;

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
blotch()

The actual blotch calculation.  This is called by Evaluate() and by
ps_makethumb(), the function that draws the preset thumbnail.
====================================================================== */

void blotch( Blotch *inst, double d, double *bkcolor )
{
   double p, a;
   int i;

   /* Using the distance d, compute where this spot falls in the
      blotch's soft edge.  The blotch is given by a cosine density
      function scaled by the softness factor.  Where the density is
      greater than 1.0, it clips. */

   p = pow( 0.5 * ( 1.0 + cos( d * inst->piOverR )), inst->softness );

   /* Blend the blotch color into the existing color using the
      computed density. */

   a = 1.0 - p;
   for ( i = 0; i < 3; i++ )
      bkcolor[ i ] = bkcolor[ i ] * a + inst->color[ i ] * p;
}


/*
======================================================================
Evaluate()

Handler callback.  Compute the blotch effect at a spot on a surface.
We find the distance from the spot to the center of the blotch and
blend some of the blotch color with the color already computed for
that spot.
====================================================================== */

XCALL_( static void )
Evaluate( Blotch *inst, LWShaderAccess *sa )
{
   double d, r2;
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

   blotch( inst, d, sa->color );
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

   vparmf = global( LWVPARMFUNCS_GLOBAL,    GFUSE_TRANSIENT );
   surff  = global( LWSURFACEFUNCS_GLOBAL,  GFUSE_TRANSIENT );
   envf   = global( LWENVELOPEFUNCS_GLOBAL, GFUSE_TRANSIENT );
   if ( !vparmf || !surff || !envf ) return AFUNC_BADGLOBAL;

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
static LWInstUpdate  *lwupdate;
static LWShelfFuncs  *shelff;

enum { ID_COLOR = 0x8001, ID_CENTER, ID_RADIUS, ID_SOFTNESS, ID_SHOPEN, ID_SHADD };


/*
======================================================================
ps_loadok()

Shelf callback.  This is called to determine whether it's okay to load
a preset, and whether a dialog confirming the load should be displayed
to the user.
====================================================================== */

static int ps_loadok( Blotch *inst )
{
   return SHLC_DFLT;
}


/*
======================================================================
ps_load()

Shelf callback.  Called to load a preset.
====================================================================== */

static void ps_load( Blotch *inst, const LWLoadState *ls, const char *filename,
   LWShelfParmList parms )
{
   Load( inst, ls );
   if ( inst->panel )
      xpanf->viewRefresh( inst->panel );
}


/*
======================================================================
ps_save()

Shelf callback.  Called to save a preset.
====================================================================== */

static void ps_save( Blotch *inst, const LWSaveState *ss, const char *filename )
{
   Save( inst, ss );
}


/*
======================================================================
ps_makethumb()

Create a preset shelf thumbnail image.
====================================================================== */

static LWPixmapID ps_makethumb( Blotch *inst, int iw, int ih )
{
   LWPixmapID image = NULL;
   double d, x, y, y2, dx, dy;
   double bkcolor[ 3 ];
   unsigned char out[ 3 ];
   int sx, sy, i, v;

   if ( imgutil ) {
      if ( image = imgutil->create( iw, ih, LWIMTYP_RGB24 )) {
         inst->r2 = inst->radius * inst->radius;
         inst->piOverR = PI / inst->radius;
         dx = 2.0 * inst->radius / iw;
         dy = 2.0 * inst->radius / ih;

         for ( sy = 0; sy < ih; sy++ ) {
            y = -inst->radius + dy * sy;
            y2 = y * y;
            for ( sx = 0; sx < iw; sx++ ) {
               x = -inst->radius + dx * sx;
               d = sqrt( x * x + y2 );
               if ( d < 1.0 ) {
                  bkcolor[ 0 ] = bkcolor[ 1 ] = bkcolor[ 2 ] = 0.5;
                  blotch( inst, d, bkcolor );
                  for ( i = 0; i < 3; i++ ) {
                     v = ( int )( bkcolor[ i ] * 255.0 );
                     if ( v < 0 ) v = 0;
                     if ( v > 255 ) v = 255;
                     out[ i ] = v;
                  }
               }
               else
                  out[ 0 ] = out[ 1 ] = out[ 2 ] = 128;
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
   Blotch *inst = xpanf->getData( panel, 0 );

   if ( shelff && inst->psclient )
      shelff->open( inst->psclient );
}


/*
======================================================================
ui_addpreset()

Button callback.  Add a preset to the shelf.
====================================================================== */

static void ui_addpreset( LWXPanelID panel, int cid )
{
   LWPixmapID image;
   Blotch *inst = xpanf->getData( panel, 0 );

   if ( shelff && inst->psclient ) {
      if ( !shelff->isOpen( inst->psclient ))
         shelff->open( inst->psclient );
      get_vpvalues( inst, 0.0 );
      if ( image = ps_makethumb( inst, 80, 80 )) {
         shelff->addPreset( inst->psclient, image, NULL );
         imgutil->destroy( image );
      }
   }
}


/*
======================================================================
ui_get()

Xpanels callback for LWXP_VIEW panels.  Returns a pointer to the data
for a given control value.  Since get_panel() declares the value types
to be "-env" or "3-env", XPanels expects us to return LWVParmIDs for
these.
====================================================================== */

void *ui_get( Blotch *inst, unsigned long vid )
{
   switch ( vid ) {
      case ID_COLOR:     return inst->vcolor;
      case ID_CENTER:    return inst->vcenter;
      case ID_RADIUS:    return inst->vradius;
      case ID_SOFTNESS:  return inst->vsoftness;
      default:           return NULL;
   }
}


/*
======================================================================
ui_set()

Xpanels callback for LWXP_VIEW panels.  Store a value in our instance
data.  XPanels takes care of updating our vparms, so we don't have
much to do here.
====================================================================== */

int ui_set( Blotch *inst, unsigned long vid, void *value )
{
   switch ( vid ) {
      case ID_COLOR:
      case ID_CENTER:
      case ID_RADIUS:
      case ID_SOFTNESS:
         return 1;
      default:
         return 0;
   }
}


/*
======================================================================
ui_chgnotify()

XPanel callback.  XPanels calls this when an event occurs that affects
the value of one of your controls.  We use the instance update global
to tell Layout that our instance data has changed.  We also claim the
preset shelf when we receive the input focus.
====================================================================== */

void ui_chgnotify( LWXPanelID panel, unsigned long cid, unsigned long vid,
   int event )
{
   Blotch *inst = xpanf->getData( panel, 0 );

   if ( event == LWXPEVENT_FOCUS )
      if ( shelff && inst->psclient )
         shelff->setContext( inst->psclient );

   if ( event == LWXPEVENT_VALUE )
      lwupdate( LWSHADER_HCLASS, inst );
}


/*
======================================================================
ui_destroynotify()

Xpanels callback.  This is called after the interface is destroyed.
The argument is the pointer passed to LWXPanelFuncs->setData() for
control ID 0, in this case our instance data.
====================================================================== */

static void ui_destroynotify( Blotch *inst )
{
   if ( shelff && inst->psclient ) {
      shelff->unsubscribe( inst->psclient );
      inst->psclient = NULL;
   }
}


/*
======================================================================
get_panel()

Create and initialize an LWXP_VIEW panel.  Called by Interface().
====================================================================== */

LWXPanelID get_panel( Blotch *inst )
{
   static LWXPanelControl xctl[] = {
      { ID_COLOR,    "Color",       "color-env",    },
      { ID_CENTER,   "Center",      "distance3-env" },
      { ID_RADIUS,   "Radius",      "distance-env"  },
      { ID_SOFTNESS, "Softness",    "float-env"     },
      { ID_SHOPEN,   "Load Preset", "vButton"       },
      { ID_SHADD,    "Save Preset", "vButton"       },
      { 0 }
   };
   static LWXPanelDataDesc xdata[] = {
      { ID_COLOR,    "Color",    "color-env",    },
      { ID_CENTER,   "Center",   "distance3-env" },
      { ID_RADIUS,   "Radius",   "distance-env"  },
      { ID_SOFTNESS, "Softness", "float-env"     },
      { 0 }
   };
   static LWXPanelHint xhint[] = {
      XpLABEL( 0, "Blotch" ),
      XpCHGNOTIFY( ui_chgnotify ),
      XpDESTROYNOTIFY( ui_destroynotify ),
      XpBUTNOTIFY( ID_SHOPEN, ui_openshelf ),
      XpBUTNOTIFY( ID_SHADD, ui_addpreset ),
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


#define PLUGNAME "Demo_Blotch_Env"

/*
======================================================================
Interface()

The interface activation function.
====================================================================== */

XCALL_( int )
Interface( long version, GlobalFunc *global, LWInterface *local,
   void *serverData )
{
   Blotch *inst = ( Blotch * ) local->inst;

   if ( version != LWINTERFACE_VERSION ) return AFUNC_BADVERSION;

   lwupdate = global( LWINSTUPDATE_GLOBAL, GFUSE_TRANSIENT );
   xpanf = global( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT );
   if ( !lwupdate || !xpanf ) return AFUNC_BADGLOBAL;

   imgutil = global( LWIMAGEUTIL_GLOBAL, GFUSE_TRANSIENT );
   shelff = global( LWSHELFFUNCS_GLOBAL, GFUSE_TRANSIENT );

   local->panel = get_panel( local->inst );

   if ( shelff )
      inst->psclient = shelff->subscribe( LWSHADER_HCLASS, PLUGNAME,
         inst, SHLF_BIN, ps_loadok, ps_load, ps_save );

   local->options = NULL;
   local->command = NULL;

   return local->panel ? AFUNC_OK : AFUNC_BADGLOBAL;
}


ServerTagInfo ServerTags[] = {
   { "Demo: Blotch with Envelopes", SRVTAG_USERNAME },
   { NULL }
};

ServerRecord ServerDesc[] = {
   { LWSHADER_HCLASS, PLUGNAME, Handler, ServerTags },
   { LWSHADER_ICLASS, PLUGNAME, Interface, ServerTags },
   { NULL }
};
