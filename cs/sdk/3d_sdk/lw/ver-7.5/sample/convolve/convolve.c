/*
======================================================================
convolve.c

Apply a 3 x 3 convolution to an image.

Ernie Wright  22 Mar 00

This is yet another convolution image filter.  Unlike the ones that
have shipped with LightWave, this one's only 3 x 3, but it supports
asymmetrical kernels and has an interface with presets.
====================================================================== */

#include <lwserver.h>
#include <lwfilter.h>
#include <lwmonitor.h>
#include <lwpanel.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


typedef struct {
   float kern[ 3 ][ 3 ];
} ConvolveInst;


/*
======================================================================
Create()

Allocate instance data.
====================================================================== */

XCALL_( LWInstance )
Create( void *priv, void *context, LWError *err )
{
   ConvolveInst *inst;

   inst = calloc( 1, sizeof( ConvolveInst ));
   if ( !inst )
      *err = "Convolve couldn't allocate its instance data.";
   else
      inst->kern[ 1 ][ 1 ] = 1.0f;

   return ( LWInstance ) inst;
}


/*
======================================================================
Destroy()

Free instance data.
====================================================================== */

XCALL_( void )
Destroy ( ConvolveInst *inst )
{
   free( inst );
}


/*
======================================================================
Copy()

Duplicate instance data.
====================================================================== */

XCALL_( LWError )
Copy ( ConvolveInst *to, ConvolveInst *from )
{
   int i;

   for ( i = 0; i < 9; i++ )
      to->kern[ i / 3 ][ i % 3 ] = from->kern[ i / 3 ][ i % 3 ];

   return NULL;
}


/*
======================================================================
Load()

Load instance data.
====================================================================== */

XCALL_( LWError )
Load( ConvolveInst *inst, const LWLoadState *ls )
{
   ls->readFP( ls->readData, &inst->kern[ 0 ][ 0 ], 9 );
   return NULL;
}


/*
======================================================================
Save()

Save instance data.
====================================================================== */

XCALL_( LWError )
Save( ConvolveInst *inst, const LWSaveState *ss )
{
   ss->writeFP( ss->writeData, &inst->kern[ 0 ][ 0 ], 9 );
   return NULL;
}


/*
======================================================================
Flags()

Returns bit flags for buffers we need (other than RGBA, which we
always get).
====================================================================== */

XCALL_( static unsigned int )
Flags( LWInstance inst )
{
   return 0;
}


/*
======================================================================
Process()

Apply the convolution.  See for example Dale Schumacher, "Image
Smoothing and Sharpening by Discrete Convolution" in GRAPHICS GEMS II,
James Arvo ed., Academic Press, 1991, ISBN 0-12-064480-0.

We multiply each of the 9 pixels in a 3 x 3 pixel neighborhood by the
corresponding value in the 3 x 3 filter kernel, then add the resulting
9 values and assign the sum to the center pixel.

We create a 3-scanline window on the image and move this window down
as we process.  getLine() is only called once per scanline (okay, 3
times, once each for red, green and blue), and at the end of each y
iteration, the bottom 2 scanlines are moved up 1 and the next scanline
is read into the bottom of our window.
====================================================================== */

XCALL_( static void )
Process( ConvolveInst *inst, const LWFilterAccess *fa )
{
   LWFVector out;
   float *r[ 3 ], *g[ 3 ], *b[ 3 ], *a;
   float s, k[ 3 ][ 3 ];
   int x, y, i;


   /* make a copy of the kernel, sum the elements */

   for ( i = 0, s = 0.0f; i < 9; i++ ) {
      k[ i / 3 ][ i % 3 ] = inst->kern[ i / 3 ][ i % 3 ];
      s += k[ i / 3 ][ i % 3 ];
   }

   /* scale the kernel so that the elements sum to 1.0 */

   if ( s != 0.0f )
      for ( i = 0; i < 9; i++ )
         k[ i / 3 ][ i % 3 ] /= s;

   /* get the first two scanlines */

   for ( i = 0; i < 2; i++ ) {
      r[ i ] = fa->getLine( LWBUF_RED, i );
      g[ i ] = fa->getLine( LWBUF_GREEN, i );
      b[ i ] = fa->getLine( LWBUF_BLUE, i );
   }

   /* fire up the monitor */

   MON_INIT( fa->monitor, fa->height / 8 );

   for ( y = 1; y < fa->height - 1; y++ ) {

      /* get the next scanline */

      r[ 2 ] = fa->getLine( LWBUF_RED, y + 1 );
      g[ 2 ] = fa->getLine( LWBUF_GREEN, y + 1 );
      b[ 2 ] = fa->getLine( LWBUF_BLUE, y + 1 );

      /* apply the kernel */

      for ( x = 1; x < fa->width - 1; x++ ) {
         out[ 0 ] = r[ 1 ][ x ] * k[ 1 ][ 1 ]
            + r[ 0 ][ x - 1 ] * k[ 0 ][ 0 ]
            + r[ 0 ][ x     ] * k[ 0 ][ 1 ]
            + r[ 0 ][ x + 1 ] * k[ 0 ][ 2 ]
            + r[ 1 ][ x - 1 ] * k[ 1 ][ 0 ]
            + r[ 1 ][ x + 1 ] * k[ 1 ][ 2 ]
            + r[ 2 ][ x - 1 ] * k[ 2 ][ 0 ]
            + r[ 2 ][ x     ] * k[ 2 ][ 1 ]
            + r[ 2 ][ x + 1 ] * k[ 2 ][ 2 ];

         out[ 1 ] = g[ 1 ][ x ] * k[ 1 ][ 1 ]
            + g[ 0 ][ x - 1 ] * k[ 0 ][ 0 ]
            + g[ 0 ][ x     ] * k[ 0 ][ 1 ]
            + g[ 0 ][ x + 1 ] * k[ 0 ][ 2 ]
            + g[ 1 ][ x - 1 ] * k[ 1 ][ 0 ]
            + g[ 1 ][ x + 1 ] * k[ 1 ][ 2 ]
            + g[ 2 ][ x - 1 ] * k[ 2 ][ 0 ]
            + g[ 2 ][ x     ] * k[ 2 ][ 1 ]
            + g[ 2 ][ x + 1 ] * k[ 2 ][ 2 ];

         out[ 2 ] = b[ 1 ][ x ] * k[ 1 ][ 1 ]
            + b[ 0 ][ x - 1 ] * k[ 0 ][ 0 ]
            + b[ 0 ][ x     ] * k[ 0 ][ 1 ]
            + b[ 0 ][ x + 1 ] * k[ 0 ][ 2 ]
            + b[ 1 ][ x - 1 ] * k[ 1 ][ 0 ]
            + b[ 1 ][ x + 1 ] * k[ 1 ][ 2 ]
            + b[ 2 ][ x - 1 ] * k[ 2 ][ 0 ]
            + b[ 2 ][ x     ] * k[ 2 ][ 1 ]
            + b[ 2 ][ x + 1 ] * k[ 2 ][ 2 ];

         /* set the new pixel value */

         fa->setRGB( x, y, out );
      }

      /* shift bottom two scanlines up by one */

      r[ 0 ] = r[ 1 ];  r[ 1 ] = r[ 2 ];
      g[ 0 ] = g[ 1 ];  g[ 1 ] = g[ 2 ];
      b[ 0 ] = b[ 1 ];  b[ 1 ] = b[ 2 ];

      /* once every 8 lines, step the monitor and check for abort */

      if (( y & 7 ) == 7 )
         if ( MON_STEP( fa->monitor )) return;
   }

   /* copy the alpha */

   for ( y = 0; y < fa->height; y++ ) {
      a = fa->getLine( LWBUF_ALPHA, y );
      for ( x = 0; x < fa->width; x++ )
         fa->setAlpha( x, y, a[ x ] );
   }

   MON_DONE( fa->monitor );
}


/*
======================================================================
Handler()

Our activation function.

We need to fill in the fields of the LWImageFilterHandler so that
LightWave can find our callbacks.
====================================================================== */

XCALL_( int )
Handler( long version, GlobalFunc *global, LWImageFilterHandler *local,
   void *serverData )
{
   if ( version != LWIMAGEFILTER_VERSION ) return AFUNC_BADVERSION;

   local->inst->create  = Create;
   local->inst->destroy = Destroy;
   local->inst->copy    = Copy;
   local->inst->load    = NULL;
   local->inst->save    = NULL;
   local->inst->descln  = NULL;

   if ( local->item ) {
      local->item->useItems = NULL;
      local->item->changeID = NULL;
   }

   local->process = Process;
   local->flags   = Flags;

   return AFUNC_OK;
}


/* interface stuff --------------------------------------------------- */

static LWPanelFuncs *panf;                // panel functions
static LWPanelID panel;                   // panel
static LWControl *ctl[ 10 ];              // panel gadgets
static LWPanControlDesc desc;             // required by macros in lwpanel.h
static LWValue
   ival = { LWT_INTEGER },
   fval = { LWT_FLOAT };

static float pkern[][ 3 ][ 3 ] = {
    1.f,  1.f,  1.f,   1.f,  1.f,  1.f,   1.f,  1.f,  1.f,   // blur
    2.f, -1.f, -1.f,  -1.f,  3.f, -1.f,  -1.f, -1.f,  2.f,   // diagonal
   -1.f,  1.f,  1.f,  -1.f, -1.f,  1.f,  -1.f,  1.f,  1.f,   // east
   -1.f, -1.f, -1.f,   2.f,  3.f,  2.f,  -1.f, -1.f, -1.f,   // horizontal
    1.f,  2.f,  1.f,   2.f,  4.f,  2.f,   1.f,  2.f,  1.f,   // gaussian blur
   -1.f, -1.f, -1.f,  -1.f,  8.f, -1.f,  -1.f, -1.f, -1.f,   // laplacian edge
    1.f,  1.f,  1.f,   1.f, -1.f,  1.f,  -1.f, -1.f, -1.f,   // north
    1.f,  1.f,  1.f,  -1.f, -1.f,  1.f,  -1.f, -1.f,  1.f,   // northeast
    1.f,  1.f,  1.f,   1.f, -1.f, -1.f,   1.f, -1.f, -1.f,   // northwest
    0.f, -1.f,  0.f,  -1.f,  5.f, -1.f,   0.f, -1.f,  0.f,   // sharpen1
   -1.f, -1.f, -1.f,  -1.f,  9.f, -1.f,  -1.f, -1.f, -1.f,   // sharpen2
   -1.f, -2.f, -1.f,  -2.f, 16.f, -2.f,  -1.f, -2.f, -1.f,   // sharpen3
   -1.f, -1.f, -1.f,   1.f, -1.f,  1.f,   1.f,  1.f,  1.f,   // south
   -1.f, -1.f,  1.f,  -1.f, -1.f,  1.f,   1.f,  1.f,  1.f,   // southeast
    1.f, -1.f, -1.f,   1.f, -1.f, -1.f,   1.f,  1.f,  1.f,   // southwest
   -1.f,  2.f, -1.f,  -1.f,  3.f, -1.f,  -1.f,  2.f, -1.f,   // vertical
    1.f,  1.f, -1.f,   1.f, -1.f, -1.f,   1.f,  1.f, -1.f,   // west
   -2.f,  0.f,  0.f,   0.f,  5.f,  0.f,   0.f,  0.f, -2.f    // woodcut
};

static char *preset[] = {
   "Blur",
   "Diagonal",
   "East",
   "Horizontal",
   "Gaussian Blur",
   "Laplacian Edge",
   "North",
   "Northeast",
   "Northwest",
   "Sharpen1",
   "Sharpen2",
   "Sharpen3",
   "South",
   "Southeast",
   "Southwest",
   "Vertical",
   "West",
   "Woodcut",
   NULL
};

static int pindex = 0;


/*
======================================================================
set_kernctl()

Set filter kernel controls from values in the instance kernel.  Called
by handle_preset() and create_controls().
====================================================================== */

static void set_kernctl( ConvolveInst *inst )
{
   int i;

   for ( i = 0; i < 9; i++ )
      SET_FLOAT( ctl[ i ], inst->kern[ i / 3 ][ i % 3 ] );
}


/*
======================================================================
handle_preset()

Event callback for the presets popup list.  Called by LWPanels.
====================================================================== */

static void handle_preset( LWControl *ectl, ConvolveInst *inst )
{
   int i;

   GET_INT( ectl, pindex );

   for ( i = 0; i < 9; i++ )
      inst->kern[ i / 3 ][ i % 3 ] = pkern[ pindex ][ i / 3 ][ i % 3 ];

   set_kernctl( inst );
}


/*
======================================================================
create_controls()

Called by Options() to create the panel controls.
====================================================================== */

static void create_controls( ConvolveInst *inst )
{
   const LWDisplayMetrics *dm;
   int x, y, dx, dy, w, i, pw, ph;


   /* create a control */

   ctl[ 0 ] = FLOAT_CTL( panf, panel, "" );

   /* find out how much vertical space the panel wants for drawing its
      own decorations */

   ph = PAN_GETH( panf, panel );
   ph -= CON_H( ctl[ 0 ] );

   /* create the rest of the controls */

   for ( i = 1; i < 9; i++ )
      ctl[ i ] = FLOAT_CTL( panf, panel, "" );

   ctl[ 9 ] = WPOPUP_CTL( panf, panel, "Presets", preset, 150 );

   /* position all of the controls */

   x  = CON_X( ctl[ 0 ] );
   y  = CON_Y( ctl[ 0 ] );
   dx = CON_HOTW( ctl[ 0 ] ) + 8;
   dy = CON_HOTH( ctl[ 0 ] ) + 4;

   for ( i = 1; i < 9; i++ )
      MOVE_CON( ctl[ i ], x + dx * ( i % 3 ), y + dy * ( i / 3 ));

   w = CON_W( ctl[ 9 ] );
   MOVE_CON( ctl[ 9 ], x + 3 * dx - w, y + 10 * dy / 3 );

   /* now that we know how much room the controls will take up, set the
      height of the panel and center it */

   ph += CON_Y( ctl[ 9 ] );
   ph += CON_HOTH( ctl[ 9 ] );
   PAN_SETH( panf, panel, ph - 6 );

   pw = PAN_GETW( panf, panel );
   dm = panf->drawFuncs->dispMetrics();
   MOVE_PAN( panf, panel, ( dm->width - pw ) / 2, ( dm->height - ph ) / 2 );

   /* initialize the controls */

   set_kernctl( inst );
   SET_INT( ctl[ 9 ], pindex );

   /* set the control event callbacks */

   CON_SETEVENT( ctl[ 9 ], handle_preset, inst );
}


/*
======================================================================
Options()

Interface callback.  Called by Layout.
====================================================================== */

XCALL_( static LWError )
Options( ConvolveInst *inst )
{
   panel = PAN_CREATE( panf, "Convolve" );
   if ( !panel )
      return "Convolve couldn't create its panel, not sure why.";

   create_controls( inst );
   panf->open( panel, PANF_BLOCKING );

   PAN_KILL( panf, panel );

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
   if ( version != LWINTERFACE_VERSION )
      return AFUNC_BADVERSION;

   panf = global( LWPANELFUNCS_GLOBAL, GFUSE_TRANSIENT );
   if ( !panf ) return AFUNC_BADGLOBAL;
   panf->globalFun = global;

   local->panel   = NULL;
   local->options = Options;
   local->command = NULL;

   return AFUNC_OK;
}


/*
======================================================================
The server description.
====================================================================== */

ServerRecord ServerDesc[] = {
   { LWIMAGEFILTER_HCLASS, "Demo_Convolve", Handler },
   { LWIMAGEFILTER_ICLASS, "Demo_Convolve", Interface },
   { NULL }
};
