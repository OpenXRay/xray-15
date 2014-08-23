/*
======================================================================
shtest.c

A master for testing the Shelf global.

Ernie Wright  27 Oct 01
====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lwserver.h>
#include <lwmaster.h>
#include <lwxpanel.h>
#include <lwimage.h>
#include <lwhost.h>
#include <lwio.h>
#include <lwshelf.h>


#define PLUGNAME "ShelfTest"


/* the instance data structure */

typedef struct {
   LWDVector      color;
   GlobalFunc    *global;
   char           desc[ 80 ];
   LWShelfCltID   psclient;
   LWXPanelID     panel;
} MyParam;


/* some globals */

static LWXPanelFuncs *xpanf;
static LWShelfFuncs  *shf;
static LWImageUtil   *imgutil;


/*
======================================================================
Create()
====================================================================== */

XCALL_( static LWInstance )
Create( void *userdata, void *context, LWError *err )
{
   MyParam *mp;

   if ( mp = calloc( 1, sizeof( MyParam ))) {
      mp->global = ( GlobalFunc * ) userdata;
      mp->color[ 0 ] = mp->color[ 1 ] = mp->color[ 2 ] = 0.8;
   }

   return mp;
}


/*
======================================================================
Destroy()
====================================================================== */

XCALL_( static void )
Destroy( MyParam *mp )
{
   if ( mp ) free( mp );
}


/*
======================================================================
Copy()
====================================================================== */

XCALL_( static LWError )
Copy( MyParam *to, MyParam *from )
{
   *to = *from;
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
Load( MyParam *mp, const LWLoadState *ls )
{
   float f[ 3 ];

   LWLOAD_FP( ls, f, 3 );
   mp->color[ 0 ] = f[ 0 ];
   mp->color[ 1 ] = f[ 1 ];
   mp->color[ 2 ] = f[ 2 ];

   return NULL;
}


/*
======================================================================
Save()

Store instance data in a scene file or as a preset.
====================================================================== */

XCALL_( static LWError )
Save( MyParam *mp, const LWSaveState *ss )
{
   float f[ 3 ];

   f[ 0 ] = ( float ) mp->color[ 0 ];
   f[ 1 ] = ( float ) mp->color[ 1 ];
   f[ 2 ] = ( float ) mp->color[ 2 ];
   LWSAVE_FP( ss, f, 3 );

   return NULL;
}


/*
======================================================================
DescLn()
====================================================================== */

XCALL_( static const char * )
DescLn( MyParam *mp )
{
   sprintf( mp->desc, "ShelfTest  (%g, %g, %g)",
      mp->color[ 0 ], mp->color[ 1 ], mp->color[ 2 ] );
   return mp->desc;
}


/*
======================================================================
Event()
====================================================================== */

XCALL_( static double )
Event( MyParam *mp, const LWMasterAccess *ma )
{
   return 0.0;
}


/*
======================================================================
Flags()
====================================================================== */

XCALL_( static int )
Flags( MyParam *mp )
{
   return 0;
}


/*
======================================================================
Handler()
====================================================================== */

XCALL_( static int )
Handler( long version, GlobalFunc *global, LWMasterHandler *local,
   void *serverData )
{
   if ( version != LWMASTER_VERSION ) return AFUNC_BADVERSION;

   local->inst->priv     = global;

   local->inst->create   = Create;
   local->inst->destroy  = Destroy;
   local->inst->copy     = Copy;
   local->inst->load     = Load;
   local->inst->save     = Save;
   local->inst->descln   = DescLn;

   local->type           = LWMAST_SCENE;
   local->event          = Event;
   local->flags          = Flags;

   return AFUNC_OK;
}


/* ----- interface stuff ----- */

enum { ID_COLOR = 0x8001, ID_SHOPEN, ID_SHADD, ID_SHLOAD, ID_SHSAVE };


/*
======================================================================
ps_loadok()

Shelf callback.  This is called to determine whether it's okay to load
a preset, and whether a dialog confirming the load should be displayed
to the user.
====================================================================== */

static int ps_loadok( MyParam *mp )
{
   return SHLC_DFLT;
}


/*
======================================================================
ps_load()

Shelf callback.  Called to load a preset.

We just call our handler Load() callback.

If the filename isn't NULL, ps_load() is being called because we
called LWShelfFuncs->load().  For that case, we create our own
LWLoadState and read the data from the file.

Otherwise, we're being called because the user double-clicked on a
preset thumbnail in the shelf window, and we use the LWLoadState the
shelf gives us to read the preset from the shelf.

If we wanted to support the loading of only a portion of the preset,
we would examine the parms array to see which named parameters the
user had chosen to load.  The list of parameter names is passed to
the shelf as an argument to LWShelfFuncs->save(), addPreset() and
addNamedPreset().  The parms argument to ps_load() contains only those
parameter names the user has selected from this list.
====================================================================== */

static void ps_load( MyParam *mp, const LWLoadState *cls, const char *filename,
   LWShelfParmList parms )
{
   LWFileIOFuncs *filef = NULL;
   LWLoadState *ls = NULL;

   if ( filename ) {
      filef = mp->global( LWFILEIOFUNCS_GLOBAL, GFUSE_ACQUIRE );
      if ( filef ) {
         ls = filef->openLoad( filename, LWIO_BINARY );
         if ( ls )
            Load( mp, ls );
      }
   }
   else
      Load( mp, cls );

   if ( filef && ls ) {
      filef->closeLoad( ls );
      mp->global( LWFILEIOFUNCS_GLOBAL, GFUSE_RELEASE );
   }

   if ( mp->panel )
      xpanf->viewRefresh( mp->panel );
}


/*
======================================================================
ps_save()

Shelf callback.  Called to save a preset.

We just call our handler Save() callback.

If the filename isn't NULL, ps_save() is being called because we
called LWShelfFuncs->save().  For that case, we create our own
LWSaveState, and the preset is saved to its own file.

Otherwise, we're here because we called LWShelfFuncs->addPreset() or
addNamedPreset(), and we use the LWSaveState the shelf gives us to
store our preset on the shelf.
====================================================================== */

static void ps_save( MyParam *mp, const LWSaveState *css, const char *filename )
{
   LWFileIOFuncs *filef;
   LWSaveState *ss;

   if ( filename ) {
      filef = mp->global( LWFILEIOFUNCS_GLOBAL, GFUSE_ACQUIRE );
      ss = filef->openSave( filename, LWIO_BINARY );
      Save( mp, ss );
   }
   else
      Save( mp, css );

   if ( filename ) {
      filef->closeSave( ss );
      mp->global( LWFILEIOFUNCS_GLOBAL, GFUSE_RELEASE );
   }
}


/*
======================================================================
ps_makethumb()

Create and draw a preset shelf thumbnail.
====================================================================== */

static LWPixmapID ps_makethumb( MyParam *mp, int iw, int ih )
{
   LWPixmapID image = NULL;
   float out[ 3 ];
   int sx, sy;

   if ( imgutil ) {
      if ( image = imgutil->create( iw, ih, LWIMTYP_RGBFP )) {
         out[ 0 ] = ( float ) mp->color[ 0 ];
         out[ 1 ] = ( float ) mp->color[ 1 ];
         out[ 2 ] = ( float ) mp->color[ 2 ];
         for ( sy = 0; sy < ih; sy++ )
            for ( sx = 0; sx < iw; sx++ )
               imgutil->setPixel( image, sx, sy, out );
      }
   }

   return image;
}


/*
======================================================================
ui_openshelf()

Button callback.  Open the preset shelf window.  If the window is
already open, we just set the context to our client ID.
====================================================================== */

static void ui_openshelf( LWXPanelID panel, int cid )
{
   MyParam *mp = xpanf->getData( panel, 0 );

   if ( !shf->isOpen( mp->psclient ))
      shf->open( mp->psclient );
   else
      shf->setContext( mp->psclient );
}


/*
======================================================================
ui_addpreset()

Button callback.  Add a preset to the shelf.
====================================================================== */

static void ui_addpreset( LWXPanelID panel, int cid )
{
   MyParam *mp = xpanf->getData( panel, 0 );
   LWPixmapID image = NULL;

   if ( !shf->isOpen( mp->psclient ))
      shf->open( mp->psclient );

   if ( image = ps_makethumb( mp, 128, 128 )) {
      shf->addPreset( mp->psclient, image, NULL );
      imgutil->destroy( image );
   }
}


/*
======================================================================
get_filename()

Open a file dialog and return a filename.  Called by ui_loadfile() and
ui_savefile().
====================================================================== */

static int get_filename( GlobalFunc *global, char *title, int type,
   char *filename )
{
   LWFileReqLocal frloc;
   LWFileActivateFunc *filereq;
   static char name[ 260 ], path[ 260 ], node[ 260 ];
   int result;

   filereq = global( LWFILEACTIVATEFUNC_GLOBAL, GFUSE_TRANSIENT );
   if ( !filereq ) return 0;

   frloc.reqType  = type;
   frloc.title    = title;
   frloc.bufLen   = sizeof( name );
   frloc.pickName = NULL;
   frloc.fileType = NULL;
   frloc.path     = path;
   frloc.baseName = node;
   frloc.fullName = name;

   result = filereq( LWFILEREQ_VERSION, &frloc );
   if ( result == AFUNC_OK && frloc.result ) {
      strcpy( filename, name );
      return 1;
   }
   return 0;
}


/*
======================================================================
ui_loadfile()

Load a preset from a separate file.
====================================================================== */

static void ui_loadfile( LWXPanelID panel, int cid )
{
   char filename[ 260 ];
   MyParam *mp = xpanf->getData( panel, 0 );

   if ( get_filename( mp->global, "Load Preset", FREQ_LOAD, filename ))
      shf->load( mp->psclient, filename, 1 );
}


/*
======================================================================
ui_savefile()

Save a preset to a separate file.

Just as a test, we call LWShelfFuncs->save() with a non-NULL parameter
list.  When this preset is reloaded, a dialog with our parameter names
will be presented to the user.  The parameter names selected by the
user will be passed to ps_load().
====================================================================== */

static void ui_savefile( LWXPanelID panel, int cid )
{
   char *param[] = { "Param 1", "Param 2", NULL };
   char filename[ 260 ];
   LWPixmapID image;
   MyParam *mp = xpanf->getData( panel, 0 );

   if ( get_filename( mp->global, "Save Preset", FREQ_SAVE, filename )) {
      image = ps_makethumb( mp, 128, 128 );
      shf->save( mp->psclient, filename, image, param );
      imgutil->destroy( image );
   }
}


/*
======================================================================
ui_dstrynotify()

Xpanels callback.  This is called after the interface is destroyed.
The argument is the pointer passed to LWXPanelFuncs->setData() for
control ID 0.
====================================================================== */

static void ui_dstrynotify( MyParam *mp )
{
   shf->unsubscribe( mp->psclient );
   mp->psclient = NULL;
   mp->panel = NULL;
}


/*
======================================================================
ui_get()

Give XPanels the value of a control.
====================================================================== */

static void *ui_get( MyParam *mp, unsigned long vid )
{
   switch ( vid ) {
      case ID_COLOR:  return &mp->color;
      default:        return NULL;
   }
}


/*
======================================================================
ui_set()

Store the value of a control.
====================================================================== */

static int ui_set( MyParam *mp, unsigned long vid, void *value )
{
   double *d;

   switch ( vid )
   {
      case ID_COLOR:
         d = ( double * ) value;
         mp->color[ 0 ] = d[ 0 ];
         mp->color[ 1 ] = d[ 1 ];
         mp->color[ 2 ] = d[ 2 ];
         break;

      default:
         return LWXPRC_NONE;
   }

   return LWXPRC_DRAW;
}


/*
======================================================================
get_xpanel()
====================================================================== */

LWXPanelID get_xpanel( MyParam *mp )
{
   LWXPanelID panel;

   static LWXPanelControl ctl[] = {
      { ID_COLOR,    "Color",          "color"   },
      { ID_SHOPEN,   "Open Shelf",     "vButton" },
      { ID_SHADD,    "Add Preset",     "vButton" },
      { ID_SHLOAD,   "Load From File", "vButton" },
      { ID_SHSAVE,   "Save To File",   "vButton" },
      { 0 }
   };
   static LWXPanelDataDesc cdata[] = {
      { ID_COLOR, "Color", "color"   },
      { 0 }
   };
   static LWXPanelHint hint[] = {
      XpLABEL( 0, "Shelf Test" ),
      XpDESTROYNOTIFY( ui_dstrynotify ),
      XpBUTNOTIFY( ID_SHOPEN, ui_openshelf ),
      XpBUTNOTIFY( ID_SHADD,  ui_addpreset ),
      XpBUTNOTIFY( ID_SHLOAD, ui_loadfile ),
      XpBUTNOTIFY( ID_SHSAVE, ui_savefile ),
      XpEND
   };

   panel = xpanf->create( LWXP_VIEW, ctl );
   if ( !panel ) return NULL;
   mp->panel = panel;

   xpanf->describe( panel, cdata, ui_get, ui_set );
   xpanf->hint( panel, 0, hint );
   xpanf->viewInst( panel, mp );
   xpanf->setData( panel, 0, mp );

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
   MyParam *mp = ( MyParam * ) local->inst;

   if ( version != LWINTERFACE_VERSION ) return AFUNC_BADVERSION;

   xpanf   = global( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT );
   shf     = global( LWSHELFFUNCS_GLOBAL, GFUSE_TRANSIENT );
   imgutil = global( LWIMAGEUTIL_GLOBAL, GFUSE_TRANSIENT );
   if ( !xpanf || !shf || !imgutil ) return AFUNC_BADGLOBAL;

   local->panel = get_xpanel( mp );
   if ( !local->panel ) return AFUNC_BADGLOBAL;

   mp->psclient = shf->subscribe( LWMASTER_HCLASS, PLUGNAME,
      mp, SHLF_BIN | SHLF_ASC | SHLF_SEP, ps_loadok, ps_load, ps_save );

   if ( !mp->psclient ) {
      xpanf->destroy( local->panel );
      local->panel = NULL;
      return AFUNC_BADGLOBAL;
   }

   local->options = NULL;
   local->command = NULL;

   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { LWMASTER_HCLASS, PLUGNAME, Handler },
   { LWMASTER_ICLASS, PLUGNAME, Interface },
   { NULL }
};
