/*
======================================================================
hotvideo.c

Sample image filter plug-in that marks or repairs hot pixels.

Ernie Wright  24 Jun 00

A pixel is hot when its RGB levels fall outside the legal range of
analog video encoding standards.  See David Martindale and Alan W.
Paeth, "Television Color Encoding and 'Hot' Broadcast Colors", in
GRAPHICS GEMS II, James Arvo ed., Academic Press, 1991, pp. 147-158,
which this code is based on.

We're able to use floating-point RGB levels directly, since LW is
happy to supply them.
====================================================================== */

#include <lwserver.h>
#include <lwfilter.h>
#include <lwmonitor.h>
#include <lwxpanel.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define ENC_NTSC 0
#define GAM_NTSC 2.2f
#define PED_NTSC 7.5f

#define ENC_PAL  1
#define GAM_PAL  2.8f
#define PED_PAL  0.0f

#define OPT_BLACK 0
#define OPT_LUM   1
#define OPT_SAT   2

#define DEF_BLACK 0.0f
#define DEF_WHITE 1.0f
#define DEF_COMP  110.0f
#define DEF_CHR   58.0f


typedef struct {
   float white;
   float black;
   float pedestal;
   float composite;
   float chroma;
   float gamma;
   int   encoding;
   int   opt;
   char  desc[ 80 ];
} HotData;


static float mat_ntsc[ 9 ] = {    // RGB to YIQ
    0.2989f,  0.5866f,  0.1144f,
    0.5959f, -0.2741f, -0.3218f,
    0.2113f, -0.5227f,  0.3113f,
};

static float mat_pal[ 9 ] = {     //  RGB to YUV
    0.2989f,  0.5866f,  0.1144f,
   -0.1473f, -0.2891f,  0.4364f,
    0.6149f, -0.5145f, -0.1004f,
};


/*
======================================================================
Create()
====================================================================== */

XCALL_( HotData * )
Create( void *priv, void *context, LWError *err )
{
   HotData *inst;

   inst = calloc( 1, sizeof( HotData ));
   if ( !inst ) {
      *err = "Couldn't allocate instance data.";
      return NULL;
   }

   inst->white     = DEF_WHITE;
   inst->black     = DEF_BLACK;
   inst->pedestal  = PED_NTSC;
   inst->composite = DEF_COMP;
   inst->chroma    = DEF_CHR;
   inst->gamma     = GAM_NTSC;
   inst->encoding  = ENC_NTSC;
   inst->opt       = OPT_SAT;

   return inst;
}


/*
======================================================================
Destroy()
====================================================================== */

XCALL_( void )
Destroy( HotData *inst )
{
   if ( inst ) free( inst );
}


/*
======================================================================
Copy()
====================================================================== */

XCALL_( LWError )
Copy( HotData *to, HotData *from )
{
   *to = *from;
   return NULL;
}


/*
======================================================================
Load()
====================================================================== */

XCALL_( static LWError )
Load( HotData *inst, const LWLoadState *ls )
{
   LWLOAD_FP( ls, &inst->white, 6 );
   LWLOAD_I4( ls, &inst->encoding, 2 );
   return NULL;
}


/*
======================================================================
Save()
====================================================================== */

XCALL_( static LWError )
Save( HotData *inst, const LWSaveState *ss )
{
   LWSAVE_FP( ss, &inst->white, 6 );
   LWSAVE_I4( ss, &inst->encoding, 2 );
   return NULL;
}


/*
======================================================================
DescLn()
====================================================================== */

XCALL_( static const char * )
DescLn( HotData *inst )
{
   char *enc;

   switch ( inst->encoding ) {
      case ENC_PAL:   enc = "PAL";   break;
      case ENC_NTSC:
      default:        enc = "NTSC";  break;
   }

   sprintf( inst->desc, "HotVideo %s %g %g",
      enc, inst->composite, inst->chroma );

   return inst->desc;
}


/*
======================================================================
Flags()
====================================================================== */

XCALL_( static unsigned int )
Flags( HotData *inst )
{
   return 0;
}


/* these are constant for all pixels */

static float gamma, igamma, black, scale, ylimit, climit, c2limit, *m;
static int opt;


/*
======================================================================
gc()

Gamma correct the value.  Martindale and Paeth point out that while
this is currently just a call to pow(), future standards may require
more complicated response curves.
====================================================================== */

static float gc( float v )
{
   return ( float ) pow(( v - black ) / scale, gamma );
}


/*
======================================================================
ungc()

Undo gamma correction (the inverse of gc()).
====================================================================== */

static float ungc( float v )
{
   return scale * ( float ) pow( v, igamma ) + black;
}


/*
======================================================================
hot()

If the color is hot, it's adjusted by the method specified in opt and
the function returns 1.  Otherwise the color is unaltered and the
function returns 0.

i and q are the two chrominance components.  For NTSC, they are I and
Q.  For PAL, i is U (scaled B-Y) and q is V (scaled R-Y).  We only
care about the length of the chroma vector, not its angle, so we don't
need to distinguish between these cases.

Chrominance is too large if

   sqrt( i^2 + q^2 ) > climit.

The composite signal amplitude is too large if

   y + sqrt( i^2 + q^2 ) > ylimit.

We avoid doing the square roots by checking

   i^2 + q^2 > climit^2
     and
   i^2 + q^2 > ( ylimit - y )^2.

Note that the y test fails when y > ylimit, a possibility I don't
think Martindale and Paeth contemplated.  It implies a luminance limit
of less than 100 IRE.  But we can test for that case directly.

OPT_BLACK sets hot pixels to black.  OPT_LUM scales the RGB levels.
OPT_SAT scales just the chroma vector without changing luminance.
====================================================================== */

static int hot( LWFVector out )
{
   float y, i, q, c, c2, y2, r, g, b, s, t;
   int h;

   r = gc( out[ 0 ] );
   g = gc( out[ 1 ] );
   b = gc( out[ 2 ] );

   y = m[ 0 ] * r + m[ 1 ] * g + m[ 2 ] * b;
   i = m[ 3 ] * r + m[ 4 ] * g + m[ 5 ] * b;
   q = m[ 6 ] * r + m[ 7 ] * g + m[ 8 ] * b;

   h = 1;
   if ( y <= ylimit ) {
      c2 = i * i + q * q;
      if ( c2 <= c2limit ) {
         y2 = ylimit - y;
         y2 *= y2;
         if ( c2 <= y2 )
            h = 0;
      }
   }

   if ( h ) {
      switch ( opt )
      {
         case OPT_BLACK:
            out[ 0 ] = out[ 1 ] = out[ 2 ] = black;
            break;

         case OPT_LUM:
            c = ( float ) sqrt( i * i + q * q );
            s = climit / c;
            t = ylimit / ( y + c );
            if ( t < s ) s = t;
            s = ( float ) pow( s, igamma );
            out[ 0 ] = ( out[ 0 ] - black ) * s + black;
            out[ 1 ] = ( out[ 1 ] - black ) * s + black;
            out[ 2 ] = ( out[ 2 ] - black ) * s + black;
            break;

         case OPT_SAT:
            c = ( float ) sqrt( i * i + q * q );
            s = climit / c;
            t = ( ylimit - y ) / c;
            if ( t < s ) s = t;
            out[ 0 ] = ungc( y + s * ( r - y ));
            out[ 1 ] = ungc( y + s * ( g - y ));
            out[ 2 ] = ungc( y + s * ( b - y ));
            break;
      }
   }

   return h;
}


/*
======================================================================
Process()
====================================================================== */

XCALL_( static void )
Process( HotData *inst, const LWFilterAccess *fa )
{
   LWFVector out;
   float *r, *g, *b, *a, d;
   int x, y;


   /* initialize the constants */

   d = 100.0f - inst->pedestal;
   ylimit  = ( inst->composite - inst->pedestal ) / d;
   climit  = inst->chroma / d;
   c2limit = climit * climit;
   black   = inst->black;
   scale   = inst->white - inst->black;
   gamma   = 1.0f / inst->gamma;
   igamma  = inst->gamma;
   opt     = inst->opt;

   switch ( inst->encoding ) {
      case ENC_PAL:
         m = mat_pal;
         break;
      case ENC_NTSC:
      default:
         m = mat_ntsc;
         break;
   }

   /* and away we go */

   MON_INIT( fa->monitor, fa->height / 16 );

   for ( y = 0; y < fa->height; y++ ) {
      r = fa->getLine( LWBUF_RED, y );
      g = fa->getLine( LWBUF_GREEN, y );
      b = fa->getLine( LWBUF_BLUE, y );
      a = fa->getLine( LWBUF_ALPHA, y );

      for ( x = 0; x < fa->width; x++ ) {
         out[ 0 ] = r[ x ];
         out[ 1 ] = g[ x ];
         out[ 2 ] = b[ x ];
         hot( out );
         fa->setRGB( x, y, out );
         fa->setAlpha( x, y, a[ x ] );
      }

      if (( y & 15 ) == 15 )
         if ( MON_STEP( fa->monitor )) return;
   }

   MON_DONE( fa->monitor );
}


/*
======================================================================
Handler()

Handler activation function.
====================================================================== */

XCALL_( int )
Handler( long version, GlobalFunc *global, LWImageFilterHandler *local,
   void *serverData )
{
   if ( version != LWIMAGEFILTER_VERSION ) return AFUNC_BADVERSION;

   local->inst->create  = Create;
   local->inst->destroy = Destroy;
   local->inst->copy    = Copy;
   local->inst->load    = Load;
   local->inst->save    = Save;
   local->inst->descln  = DescLn;

   if ( local->item ) {
      local->item->useItems = NULL;
      local->item->changeID = NULL;
   }

   local->process = Process;
   local->flags   = Flags;

   return AFUNC_OK;
}


/* ----- interface stuff ----- */

static LWXPanelFuncs   *xpanf;

/* panel control IDs */

enum { IDBLK = 0x8001, IDWTE, IDCOM, IDCHR, IDGAM, IDPED, IDENC, IDOPT, IDDEF };

/* simplify get and set */

#define XSETFLT(id, v)  FSETFLT(xpanf, panel, id, v)
#define XSETINT(id, v)  FSETINT(xpanf, panel, id, v)

#define XGETFLT(id)  ((float)(FGETFLT(xpanf, panel, id)))
#define XGETINT(id)  FGETINT(xpanf, panel, id)


/*
======================================================================
setvals()

Set control values.
====================================================================== */

static void setvals( LWXPanelID panel, HotData *inst )
{
   XSETFLT( IDBLK, inst->black );
   XSETFLT( IDWTE, inst->white );
   XSETFLT( IDPED, inst->pedestal );
   XSETFLT( IDCOM, inst->composite );
   XSETFLT( IDCHR, inst->chroma );
   XSETFLT( IDGAM, inst->gamma );
   XSETINT( IDENC, inst->encoding );
   XSETINT( IDOPT, inst->opt );
}


/*
======================================================================
btn_default()

Load default values when the user presses the Defaults button.
====================================================================== */

static void btn_default( LWXPanelID panel, int cid )
{
   HotData *inst = xpanf->getData( panel, 0 );

   inst->white     = DEF_WHITE;
   inst->black     = DEF_BLACK;
   inst->composite = DEF_COMP;
   inst->chroma    = DEF_CHR;

   switch ( inst->encoding ) {
      case ENC_PAL:
         inst->pedestal = PED_PAL;
         inst->gamma    = GAM_PAL;
         break;
      case ENC_NTSC:
      default:
         inst->pedestal = PED_NTSC;
         inst->gamma    = GAM_NTSC;
         break;
   }

   setvals( panel, inst );
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
   HotData *inst = xpanf->getData( panel, 0 );

   if ( event == LWXPEVENT_TRACK || event == LWXPEVENT_VALUE ) {
      switch ( vid ) {
         case IDBLK:  inst->black     = XGETFLT( IDBLK );  break;
         case IDWTE:  inst->white     = XGETFLT( IDWTE );  break;
         case IDCOM:  inst->composite = XGETFLT( IDCOM );  break;
         case IDCHR:  inst->chroma    = XGETFLT( IDCHR );  break;
         case IDGAM:  inst->gamma     = XGETFLT( IDGAM );  break;
         case IDPED:  inst->pedestal  = XGETFLT( IDPED );  break;
         case IDENC:  inst->encoding  = XGETINT( IDENC );  break;
         case IDOPT:  inst->opt       = XGETINT( IDOPT );  break;
      }
   }

   return LWXPRC_NONE;
}


/*
======================================================================
get_xpanel()

Create and initialize an xpanel.
====================================================================== */

static LWXPanelID get_xpanel( HotData *inst )
{
   static char *enc[] = {
      "NTSC",
      "PAL",
      NULL
   };

   static char *opt[] = {
      "Set to Black",
      "Reduce Luminance",
      "Reduce Saturation",
      NULL
   };

   static LWXPanelControl ctrl_list[] = {
      { IDBLK, "Black Point",           "float"      },
      { IDWTE, "White Point",           "float"      },
      { IDCOM, "Composite Limit (IRE)", "float"      },
      { IDCHR, "Chroma Limit (IRE)",    "float"      },
      { IDGAM, "Gamma",                 "float"      },
      { IDPED, "Pedestal",              "float"      },
      { IDENC, "Encoding",              "iPopChoice" },
      { IDOPT, "Correction",            "iPopChoice" },
      { IDDEF, "Defaults",              "vButton"    },
      { 0 }
   };

   static LWXPanelDataDesc data_descrip[] = {
      { IDBLK, "Black Point",           "float"   },
      { IDWTE, "White Point",           "float"   },
      { IDCOM, "Composite Limit (IRE)", "float"   },
      { IDCHR, "Chroma Limit (IRE)",    "float"   },
      { IDGAM, "Gamma",                 "float"   },
      { IDPED, "Pedestal",              "float"   },
      { IDENC, "Encoding",              "integer" },
      { IDOPT, "Correction",            "integer" },
      { 0 }
   };

   static LWXPanelHint hint[] = {
      XpLABEL( 0, "Hot Video Filter" ),
      XpCHGNOTIFY( ui_chgnotify ),
      XpBUTNOTIFY( IDDEF, btn_default ),

      XpSTRLIST( IDENC, enc ),
      XpSTRLIST( IDOPT, opt ),

      XpMIN( IDBLK, -10 ), XpMAX( IDBLK,  10 ), XpSTEP( IDBLK,  1 ),
      XpMIN( IDWTE,   0 ), XpMAX( IDWTE,  20 ), XpSTEP( IDWTE,  1 ),
      XpMIN( IDCOM,  80 ), XpMAX( IDCOM, 135 ), XpSTEP( IDCOM, 10 ),
      XpMIN( IDCHR,  30 ), XpMAX( IDCHR,  80 ), XpSTEP( IDCHR, 10 ),
      XpMIN( IDPED,   0 ), XpMAX( IDPED,  15 ), XpSTEP( IDPED,  5 ),

      XpEND
   };

   LWXPanelID panel;

   panel = xpanf->create( LWXP_FORM, ctrl_list );
   if ( !panel ) return NULL;

   xpanf->hint( panel, 0, hint );
   xpanf->describe( panel, data_descrip, NULL, NULL );
   xpanf->setData( panel, 0, inst );
   setvals( panel, inst );

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
   if ( version != LWINTERFACE_VERSION ) return AFUNC_BADVERSION;

   xpanf = global( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT );
   if ( !xpanf ) return AFUNC_BADGLOBAL;

   local->panel = get_xpanel( local->inst );
   if ( !local->panel ) return AFUNC_BADGLOBAL;

   local->options = NULL;
   local->command = NULL;

   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { LWIMAGEFILTER_HCLASS, "Demo_HotVideo", Handler },
   { LWIMAGEFILTER_ICLASS, "Demo_HotVideo", Interface },
   { NULL }
};
