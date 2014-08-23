/*
======================================================================
framebuf.c

Demonstrate the FrameBufferHandler class.

Ernie Wright  18 Mar 00
====================================================================== */

#include <lwserver.h>
#include <lwhandler.h>
#include <lwrender.h>
#include <lwhost.h>
#include <lwframbuf.h>
#include <lwpanel.h>
#include <stdlib.h>


typedef struct {
   int         w, h;
   int         y;
   LWRasterID  ras;
   LWPanelID   panel;
   LWControl  *ctl;
   GlobalFunc *global;
} FrameBuf;


static LWPanelFuncs *panf;                // panel functions
static LWRasterFuncs *rasf;               // raster functions
static LWPanControlDesc desc;             // used by macros in lwpanel.h
static LWValue ival = { LWT_INTEGER };


/*
======================================================================
draw_canvas()

Draw callback for a panel control.  Open() installs this as the draw
callback for a canvas control.  This is how our frame buffer image is
drawn on the panel.
====================================================================== */

static void draw_canvas( LWControl *ctl, FrameBuf *fbuf, DrMode mode )
{
   int x, y;

   x = CON_HOTX( ctl );
   y = CON_HOTY( ctl );
   rasf->blitPanel( fbuf->ras, 0, 0, fbuf->panel, x, y, fbuf->w, fbuf->h );
}


/*
======================================================================
Create()

Handler callback.  Allocate our instance data.
====================================================================== */

XCALL_( static LWInstance )
Create( void *priv, void *context, LWError *err )
{
   FrameBuf *fbuf;

   fbuf = calloc( 1, sizeof( FrameBuf ));
   if ( !fbuf )
      *err = "Couldn't allocate instance data.";

   return fbuf;
}


/*
======================================================================
Destroy()

Handler callback.  Free memory allocated by Create().
====================================================================== */

XCALL_( static void )
Destroy( FrameBuf *fbuf )
{
   if ( fbuf ) free( fbuf );
}


/*
======================================================================
Copy()

Handler callback.  Make a copy of an existing instance.  Make sure the
copy creates its own raster and panel.
====================================================================== */

XCALL_( static LWError )
Copy( FrameBuf *to, FrameBuf *from )
{
   *to = *from;

   to->ras = NULL;
   to->panel = NULL;

   return NULL;
}


/*
======================================================================
Close()

FrameBuffer callback.  Free memory allocated by Open().
====================================================================== */

XCALL_( static void )
Close( FrameBuf *fbuf )
{
   if ( fbuf->ras ) {
      rasf->destroy( fbuf->ras );
      fbuf->ras = NULL;
   }
   if ( fbuf->panel ) {
      PAN_KILL( panf, fbuf->panel );
      fbuf->panel = NULL;
   }
}


/*
======================================================================
Open()

FrameBuffer callback.  Allocate the raster and create the panel.
====================================================================== */

XCALL_( static LWError )
Open( FrameBuf *fbuf, int w, int h )
{
   fbuf->ras = rasf->create( w, h, 0 );
   if ( !fbuf->ras )
      return "Couldn't allocate image buffer.";

   fbuf->w = w;
   fbuf->h = h;

   fbuf->panel = PAN_CREATE( panf, "FrameBuffer Demo" );
   if ( !fbuf->panel ) {
      Close( fbuf );
      return "Couldn't create frame buffer panel.";
   }

   fbuf->ctl = CANVAS_CTL( panf, fbuf->panel, "", w, h );
   if ( !fbuf->ctl ) {
      Close( fbuf );
      return "Couldn't create frame buffer canvas.";
   }

   ival.ptr.ptr = draw_canvas;
   fbuf->ctl->set( fbuf->ctl, CTL_USERDRAW, &ival );

   ival.ptr.ptr = fbuf;
   fbuf->ctl->set( fbuf->ctl, CTL_USERDATA, &ival );

   return NULL;
}


/*
======================================================================
Begin()

FrameBuffer callback.  Initialize the scanline index.
====================================================================== */

XCALL_( static LWError )
Begin( FrameBuf *fbuf )
{
   fbuf->y = 0;
   return NULL;
}


/*
======================================================================
Write()

FrameBuffer callback.  Copy a scanline into our image buffer.
====================================================================== */

XCALL_( static LWError )
Write( FrameBuf *fbuf, const void *rb, const void *gb, const void *bb,
   const void *a )
{
   int x;
   const unsigned char *r = rb, *g = gb, *b = bb;

   for ( x = 0; x < fbuf->w; x++, r++, g++, b++ )
      rasf->drawRGBPixel( fbuf->ras, *r, *g, *b, x, fbuf->y );

   fbuf->y++;

   return NULL;
}


/*
======================================================================
Pause()

FrameBuffer callback.  Display the panel.  Our draw_canvas() control
callback takes care of drawing the image onto the panel.
====================================================================== */

XCALL_( static void )
Pause( FrameBuf *fbuf )
{
   panf->open( fbuf->panel, PANF_BLOCKING );
}


/*
======================================================================
Handler()

The activation function.  Get the panel and raster globals and fill in
the callback fields.
====================================================================== */

XCALL_( int )
Handler( long version, GlobalFunc *global, LWFrameBufferHandler *local,
   void *serverData )
{
   if ( version != LWFRAMEBUFFER_VERSION )
      return AFUNC_BADVERSION;

   panf = global( LWPANELFUNCS_GLOBAL, GFUSE_TRANSIENT );
   rasf = global( LWRASTERFUNCS_GLOBAL, GFUSE_TRANSIENT );
   if ( !panf || !rasf )
      return AFUNC_BADGLOBAL;

   local->type          = LWFBT_UBYTE;

   local->inst->create  = Create;
   local->inst->destroy = Destroy;
   local->inst->copy    = Copy;

   local->open          = Open;
   local->close         = Close;
   local->begin         = Begin;
   local->write         = Write;
   local->pause         = Pause;

   return AFUNC_OK;
}


/*
======================================================================
Options()

Interface callback.
====================================================================== */

XCALL_( static LWError )
Options( FrameBuf *inst )
{
   static char *t[ 2 ] = {
      "We don't have any real options yet.",
      "This is mostly just a demo for programmers."
   };
   LWMessageFuncs *msg;

   if ( msg = inst->global( LWMESSAGEFUNCS_GLOBAL, GFUSE_TRANSIENT ))
      msg->info( t[ 0 ], t[ 1 ] );

   return NULL;
}


/*
======================================================================
Interface()

Interface activation function.
====================================================================== */

XCALL_( int )
Interface( long version, GlobalFunc *global, LWInterface *local,
   void *serverData )
{
   FrameBuf *inst = ( FrameBuf * ) local->inst;

   if ( version != LWINTERFACE_VERSION )
      return AFUNC_BADVERSION;

   local->panel   = NULL;
   local->options = Options;
   local->command = NULL;

   inst->global = global;

   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { LWFRAMEBUFFER_HCLASS, "Demo_FrameBuffer", Handler },
   { LWFRAMEBUFFER_ICLASS, "Demo_FrameBuffer", Interface },
   { NULL }
};
