/*
======================================================================
atmosphere.c

Volumetric fog.

Gregory Duquesne
Minor touch-up by Ernie Wright  16 Apr 00

Based on chapter 14 (K. Musgrave, L. Gritz, S. Worley) of TEXTURING
AND MODELING, 2nd ed., ISBN 0-12-228730-4.
====================================================================== */

#include <lwserver.h>
#include <lwhost.h>
#include <lwvolume.h>
#include <lwtxtr.h>
#include <lwtxtred.h>
#include <lwmath.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


/* our instance data */

#define VERSION_NB 3

typedef struct st_AtmosphereData {
   double       lo, hi, fa, lum, opa, den, col[ 3 ];
   int          type, res, useTxtr, bck, march;
   char         desc[ 100 ];
   LWTextureID  txtr;
   LWTECltID    txedID;
   LWTime       time;
} AtmosphereData;


/* some globals */

static LWXPanelFuncs *xpanf;
static LWTextureFuncs *txtrf;
static LWTxtrEdFuncs *txedf;
static LWBackdropInfo *backdropinfo;
static LWItemInfo *iteminfo;


#define length(v) (sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]))
#define upcomp(v) (v[1])


/*
======================================================================
GADD()

Calculate density and illumination at a point.  Called by the ray
marcher.
====================================================================== */

double GADD( const LWVolumeAccess *va, double pp[ 3 ], double y, double falloff,
   int use_lighting, int atmos_type, double li[ 3 ] )
{
   LWItemID light;
   double ilu[ 3 ], ldir[ 3 ];

   /* accumulate contributions from each light */

   if ( use_lighting && ( va->flags & LWVEF_COLOR )) {
      VSET( li, 0.0 );
      light = iteminfo->first( LWI_LIGHT, NULL );
      while ( light ) {
         va->illuminate( light, pp, ldir, ilu );
         VADD( li, ilu );
         light = iteminfo->next( light );
      }
   }
   else
      VSET( li, 1.0 );

   /* return GADD (geometric atmospheric density distribution) */

   if ( atmos_type == 1 )
      return exp( -falloff * y );
   else
      return 1.0;
}


/*
======================================================================
evalTexture()

Evaluate the texture, which the ray marcher uses to modulate the
transparency.
====================================================================== */

double evalTexture( LWMicropol *mp, double pos[ 3 ], double stride,
   double col[ 4 ], LWTextureID txtr )
{
   VCPY( mp->oPos, pos );
   VCPY( mp->wPos, pos );
   mp->spotSize = 0.3333 * stride;
   return txtrf->evaluate( txtr, mp, col );
}


/*
======================================================================
raymarchEval()

Find the ray's intersection with the fog layer and accumulate samples
along the ray.  Each sample has a color, an opacity, a size (called a
stride) and is identified by its position (dist) along the ray.  The
accumulation stops when the end of the intersection, or a max opacity,
or a max number of samples has been reached.
====================================================================== */

void raymarchEval( const LWVolumeAccess *va, AtmosphereData *dat )
{
   double density = 100.0, falloff = 10.0;
   double integstart = 0.0, integend = 100.0;
   double minstepsize = 0.01, maxstepsize = 100.0;
   double k = 200.0;
   double fogclr[ 3 ];
   int use_lighting = 1, atmos_type = dat->type;

   double dtau, last_dtau, total_tau, rstep, tau, x, y, h, col[ 4 ], trans;
   int nsteps = 0;                  /* record number of integration steps */
   double li[ 3 ], last_li[ 3 ];
   LWVolumeSample sample;           /* differential color & opacity */
   LWDVector PP;
   LWMicropol mp;


   k *= dat->res;
   falloff *= dat->fa;
   density /= dat->den;

   h = dat->hi - dat->lo;
   if ( h == 0 ) return;

   /* if using a texture, initialize the micropolygon */

   if ( dat->useTxtr ) {
      memset( &mp, 0, sizeof( LWMicropol ));
      mp.gNorm[ 1 ] = mp.wNorm[ 1 ] = 1.0;
      mp.oAxis = mp.wAxis = 1;
      mp.oScl[ 0 ] = mp.oScl[ 1 ] = mp.oScl[ 2 ] = 1.0;
   }

   /* get the fog color */

   if ( dat->bck )
      backdropinfo->backdrop( dat->time, va->dir, fogclr );
   else
      VCPY( fogclr, dat->col );

   /* assume initially that the ray origin is in the fog */

   sample.dist = va->nearClip;

   /* below the fog bottom, looking up */

   if ( va->dir[ 1 ] > 0 && va->o[ 1 ] < dat->lo ) {
      y = ( dat->lo - va->o[ 1 ] );
      x = y / va->dir[ 1 ];
      sample.dist = sqrt( x * x + y * y );
   }

   /* above the fog top, looking down */

   if ( va->dir[ 1 ] < 0 && va->o[ 1 ] > dat->hi ) {
      y = ( dat->hi - va->o[ 1 ] );
      x = y / va->dir[ 1 ];
      sample.dist = sqrt( x * x + y * y );
   }

   /* test if intersection within clipping range */

   if ( sample.dist > va->farClip ) return;

   VADDS3( PP, va->o, va->dir, sample.dist );
   y = ( upcomp( PP ) - dat->lo ) / h;

   /* test if outside fog */

   if (( y > 1 && va->dir[ 1 ] > 0 ) || ( y < 0 && va->dir[ 1 ] < 0 ))
      return;

   dtau = density * GADD( va, PP, y, falloff, use_lighting, atmos_type, li );
   rstep = 1.0 / ( k * dtau + 0.001 );
   sample.stride = sample.dist * MAX( MIN( CLAMP( rstep, minstepsize, maxstepsize ),
      va->farClip - sample.dist ), 0.0005 );

   ++nsteps;
   total_tau = 0;

   while (( sample.dist <= va->farClip )
      && ( total_tau * dat->opa < 300 ) && ( nsteps < 200 )) {

      last_dtau = dtau;
      VCPY( last_li, li );
      VADDS3( PP, va->o, va->dir, sample.dist );

      y = ( upcomp( PP ) - dat->lo ) / h;

      /* test if outside fog */

      if (( y > 1 && va->dir[ 1 ] > 0 ) || ( y < 0 && va->dir[ 1 ] < 0 ))
         return;

      dtau = density * GADD( va, PP, y, falloff, use_lighting, atmos_type, li );
      if ( !dat->useTxtr ) {
         trans = 0;
         VSCL3( col, fogclr, dat->lum );
      }
      else {
         trans = evalTexture( &mp, PP, sample.stride, col, dat->txtr );
         VSCL( col, dat->lum );
      }

      if ( trans != 1.0 ) {
         tau = 0.5 * sample.stride * ( dtau * ( 1.0 - trans ) + last_dtau );
         total_tau += tau;

         /* color */

         sample.color[ 0 ] = 0.5 * col[ 0 ] * sample.stride
            * ( li[ 0 ] * dtau + last_li[ 0 ] * last_dtau );
         sample.color[ 1 ] = 0.5 * col[ 1 ] * sample.stride
            * ( li[ 1 ] * dtau + last_li[ 1 ] * last_dtau );
         sample.color[ 2 ] = 0.5 * col[ 2 ] * sample.stride
            * ( li[ 2 ] * dtau + last_li[ 2 ] * last_dtau );

         /* opacity */

         VSET( sample.opacity, tau * dat->opa );

         /* add volumetric sample to the ray */

         va->addSample( va->ray, &sample );
      }

      /* new stride = f(dist) */

      rstep = 1.0 / ( k * dtau + 0.001 );
      sample.stride = sample.dist * MAX( MIN( CLAMP( rstep, minstepsize, maxstepsize ),
         va->farClip - sample.dist ), 0.0005 );
      sample.dist += sample.stride;
      nsteps += 1;
   }
}


/*
======================================================================
analyticEval()

The analytic evaluation is different from the ray marching.  What it
does is just evaluate an overall fog effect for the ray based on the
analytical solution of the integration of an exponential density
distribution (d = exp(-y)).  The result is saved as a single sample of
size 0 and opacity 1.0. The same technique can be used to represent
surface samples with the volumetric engine.  By combining the overall
fog effect (=fog amount) and the background color, we can evaluate
this fog in a way similar to LW's standard fog.
====================================================================== */

void analyticEval( const LWVolumeAccess *va, AtmosphereData *dat )
{
   LWVolumeSample sample;                   /* differential color & opacity */
   double density = 100.f, falloff = 200.f;
   double tau, fog, x, y, yo, ye, lo, le, h , nearClip, fogclr[ 3 ], dir[ 3 ];


   /* skip evaluation if this is a shadow ray */

   if ( !( va->flags & LWVEF_COLOR )) return;

   h = dat->hi - dat->lo;
   if ( h == 0 ) return;

   falloff = 200 * dat->fa;
   density /= dat->den;
   nearClip = 0.0001;

   /* find ray/fog intersection */

   y = va->o[ 1 ];
   if ( va->dir[ 1 ] > 0 ) {  /* looking up */
      if ( y > dat->hi )
         return;

      else if ( y > dat->lo && y < dat->hi ) {
         y = ( dat->hi - va->o[ 1 ] );
         x = y / va->dir[ 1 ];

         ye = dat->hi;
         le = sqrt( x * x + y * y );

         lo = nearClip;
         yo = va->o[ 1 ] + lo * va->dir[ 1 ];
         sample.dist = lo;
      }

      else if ( y < dat->lo ) {
         y = ( dat->lo - va->o[ 1 ] );
         x = y / va->dir[ 1 ];

         lo = sqrt( x * x + y * y );
         yo = va->o[ 1 ] + lo * va->dir[ 1 ];

         y = ( dat->hi - va->o[ 1 ] );
         x = y / va->dir[ 1 ];

         ye = dat->hi;
         le = sqrt( x * x + y * y );

         sample.dist = lo;
      }
   }

   if ( va->dir[ 1 ] < 0 ) {  /* looking down */
      if ( y < dat->lo )
         return;

      else if ( y > dat->lo && y < dat->hi ) {
         y = ( dat->lo - va->o[ 1 ] );
         x = y / va->dir[ 1 ];

         ye = dat->lo;
         le = sqrt( x * x + y * y );

         lo = nearClip;
         yo = va->o[ 1 ] + lo * va->dir[ 1 ];
      }

      else if ( y > dat->hi ) {
         y = ( dat->hi - va->o[ 1 ] );
         x = y / va->dir[ 1 ];

         lo = sqrt( x * x + y * y );
         yo = va->o[ 1 ] + lo * va->dir[ 1 ];

         y = ( dat->lo - va->o[ 1 ] );
         x = y / va->dir[ 1 ];

         ye = dat->lo;
         le = sqrt( x * x + y * y );
      }
   }

   if ( lo > va->farClip )
      return;

   /* evaluate tau = optical distance */

   sample.dist = 0.9999 * va->farClip;
   if ( le > va->farClip ) {
      le = va->farClip;
      ye = va->o[ 1 ] + le * va->dir[ 1 ];
   }

   yo = ( yo - dat->lo ) / h;
   ye = ( ye - dat->lo ) / h;
   if ( fabs( va->dir[ 1 ] ) > 0.001 ) {
      tau = exp( -( yo + va->dir[ 1 ] * ( le - lo ))) - exp( -yo );
      tau *= -( density * ( le - lo )) / ( falloff * va->dir[ 1 ] );
   }
   else {
      VCPY( dir, va->dir );
      if ( dir[ 1 ] > 0 )
         dir[ 1 ] = 0.001;
      else
         dir[ 1 ] = -0.001;

      tau = exp( -( yo + dir[ 1 ] * ( le - lo ))) - exp( -yo );
      tau *= -( density * ( le - lo )) / ( falloff * dir[ 1 ] );
   }

   /* convert tau to the overall normalised fog amount along that ray */

   tau -= density * ( le - lo ) * exp( -falloff );
   if ( tau > 0 )
      fog = 1 - exp( -tau );  /* R -> [0,1].... 0 = no fog , 1 = max fog */
   else
      fog = 1;

   fog *= dat->opa;
   CLAMP( fog, 0, 1 );

   /* Now we compute the final color value in a way similar to LW's fog:
        final_color = (1 - fog) * backColor + fog * fogColor * luminosity
      backColor = va->rayColor  is the color viewed from the ray (ie LW's
      render).  The result is set a sample of size 0, and opacity = 1.0
      which means that this sample is opaque (null sized samples are
      treated as surface samples). */

   sample.stride = 0;
   VSET( sample.opacity, 1.0 );
   if ( dat->bck )
      backdropinfo->backdrop( dat->time, va->dir, fogclr );
   else
      VCPY( fogclr, dat->col );

   sample.color[ 0 ] = ( 1 - fog ) * va->rayColor[ 0 ] + fog * fogclr[ 0 ] * dat->lum;
   sample.color[ 1 ] = ( 1 - fog ) * va->rayColor[ 1 ] + fog * fogclr[ 1 ] * dat->lum;
   sample.color[ 2 ] = ( 1 - fog ) * va->rayColor[ 2 ] + fog * fogclr[ 2 ] * dat->lum;

   va->addSample( va->ray, &sample );
}


/*
======================================================================
Create()

Create and initialize the instance data.
====================================================================== */

XCALL_( static LWInstance )
Create( void *priv, void *context, LWError *err )
{
   AtmosphereData *dat;

   dat = calloc( 1, sizeof( AtmosphereData ));
   if ( !dat ) {
      *err = "Couldn't allocate instance data.";
      return NULL;
   }

   dat->lo      = -4.0;
   dat->hi      = 0.0;
   dat->fa      = 0.5;
   dat->lum     = 0.5;
   dat->opa     = 1.0;
   dat->den     = 1.0;
   dat->res     = 2;
   dat->type    = 1;
   dat->bck     = 1;
   dat->useTxtr = 0;
   dat->march   = 0;
   VSET( dat->col, 1.0 );

   return dat;
}


/*
======================================================================
Destroy()

Free instance data.
====================================================================== */

XCALL_( static void )
Destroy( AtmosphereData *dat )
{
   if ( dat ) {
      if ( dat->txtr )
         txtrf->destroy( dat->txtr );
      free( dat );
   }
}


/*
======================================================================
Copy()

Initialize a duplicate instance.  We can't just copy the LWTextureID.
We need to let the texture functions copy() function do that for us.
====================================================================== */

XCALL_( static LWError )
Copy( AtmosphereData *to, AtmosphereData *from)
{
   LWTextureID txtr;

   if ( from->txtr )
      txtrf->copy( to->txtr, from->txtr );

   txtr = to->txtr;
   *to = *from;
   to->txtr = txtr;

   return NULL;
}


/*
======================================================================
Load()

Read instance data from a scene file.
====================================================================== */

static LWBlockIdent headerBlks[] = {
   { 0x53, "VFogTexture" },
   { 0 }
};

XCALL_( static LWError )
Load( AtmosphereData *dat, const LWLoadState *lState )
{
   int version, txtr = 0;
   float fp[ 3 ];

   LWLOAD_I4( lState, &version, 1 );

   LWLOAD_FP( lState, fp, 1);   dat->hi  = fp[ 0 ];
   LWLOAD_FP( lState, fp, 1);   dat->lo  = fp[ 0 ];
   LWLOAD_FP( lState, fp, 1);   dat->fa  = fp[ 0 ];
   LWLOAD_FP( lState, fp, 1);   dat->lum = fp[ 0 ];
   LWLOAD_FP( lState, fp, 1);   dat->opa = fp[ 0 ];
   LWLOAD_FP( lState, fp, 1);   dat->den = fp[ 0 ];
   LWLOAD_FP( lState, fp, 3);   VCPY( dat->col, fp );

   LWLOAD_I4( lState, &dat->res, 1 );
   LWLOAD_I4( lState, &dat->useTxtr, 1 );
   LWLOAD_I4( lState, &dat->march, 1 );
   if ( version > 1 )
      LWLOAD_I4( lState, &dat->bck, 1 );

   if ( version > 2 )
      LWLOAD_I4( lState, &txtr, 1 );
   else
      return NULL;

   if ( !txtr ) return NULL;

   if ( LWLOAD_FIND( lState, headerBlks )) {
      if ( !dat->txtr )
         dat->txtr = txtrf->create( TRT_COLOR, "FogTexture", NULL, NULL );
      txtrf->load( dat->txtr, lState );

      LWLOAD_END( lState );
   }

   return NULL;
}


/*
======================================================================
Save()

Store instance data in a scene file.
====================================================================== */

XCALL_( LWError )
Save( AtmosphereData *dat, const LWSaveState *sState )
{
   int version = VERSION_NB, txtr;
   float fp[ 3 ];

   LWSAVE_I4( sState, &version, 1 );

   fp[ 0 ] = ( float ) dat->hi;        LWSAVE_FP( sState, fp, 1 );
   fp[ 0 ] = ( float ) dat->lo;        LWSAVE_FP( sState, fp, 1 );
   fp[ 0 ] = ( float ) dat->fa;        LWSAVE_FP( sState, fp, 1 );
   fp[ 0 ] = ( float ) dat->lum;       LWSAVE_FP( sState, fp, 1 );
   fp[ 0 ] = ( float ) dat->opa;       LWSAVE_FP( sState, fp, 1 );
   fp[ 0 ] = ( float ) dat->den;       LWSAVE_FP( sState, fp, 1 );
   fp[ 0 ] = ( float ) dat->col[ 0 ];
   fp[ 1 ] = ( float ) dat->col[ 1 ];
   fp[ 2 ] = ( float ) dat->col[ 2 ];  LWSAVE_FP( sState, fp, 3 );

   LWSAVE_I4( sState, &dat->res, 1 );
   LWSAVE_I4( sState, &dat->useTxtr, 1 );
   LWSAVE_I4( sState, &dat->march, 1 );
   LWSAVE_I4( sState, &dat->bck, 1 );

   if ( dat->txtr ) {
      txtr = 1;
      LWSAVE_I4( sState, &txtr, 1 );
      LWSAVE_BEGIN( sState, &headerBlks[ 0 ], 0 );
      txtrf->save( dat->txtr, sState );
      LWSAVE_END( sState );
   }
   else {
      txtr = 0;
      LWSAVE_I4( sState, &txtr, 1 );
   }

   return NULL;
}


/*
======================================================================
Describe()

Return a one-line text description of the instance data.
====================================================================== */

XCALL_( static const char * )
Describe( AtmosphereData *dat )
{
   sprintf( dat->desc, "Bottom: %2f Top: %2f", dat->lo, dat->hi );
   return dat->desc;
}


/*
======================================================================
NewTime()

Called at the start of each frame.
====================================================================== */

XCALL_( static LWError )
NewTime( AtmosphereData *dat, LWFrame f, LWTime t )
{
   dat->time = t;
   return NULL;
}


/*
======================================================================
Evaluate()
====================================================================== */

XCALL_( static double )
Evaluate( AtmosphereData *dat, const LWVolumeAccess *access )
{
   if ( dat->march )
      raymarchEval( access, dat );
   else
      analyticEval( access, dat );

   return 1.0;
}


/*
======================================================================
Flags()
====================================================================== */

XCALL_( static unsigned int )
Flags( AtmosphereData *dat )
{
   return 0;
}


/*
======================================================================
Atmosphere()

The activation function.  Check the version, get some globals, fill in
the LWVolumetricHandler structure.
====================================================================== */

XCALL_( static int )
Atmosphere( long version, GlobalFunc *global, LWVolumetricHandler *local,
   void *serverData )
{
   if ( version != LWVOLUMETRIC_VERSION ) return AFUNC_BADVERSION;

   iteminfo     = global( LWITEMINFO_GLOBAL, GFUSE_TRANSIENT );
   txtrf        = global( LWTEXTUREFUNCS_GLOBAL, GFUSE_TRANSIENT );
   backdropinfo = global( LWBACKDROPINFO_GLOBAL, GFUSE_TRANSIENT );
   if ( !iteminfo || !txtrf || !backdropinfo ) return AFUNC_BADGLOBAL;

   local->inst->create  = Create;
   local->inst->destroy = Destroy;
   local->inst->load    = Load;
   local->inst->save    = Save;
   local->inst->copy    = Copy;
   local->inst->descln  = Describe;
   local->rend->newTime = NewTime;
   local->evaluate      = Evaluate;
   local->flags         = Flags;

   return AFUNC_OK;
}


/* user interface stuff ----- */

enum { ID_MARCH = 0x8001, ID_HI, ID_LO, ID_FA, ID_LUM, ID_OPA, ID_DEN, ID_BCK,
   ID_COL, ID_RES, ID_TXTR, ID_TXBT, ID_RGRP, ID_GGRP, ID_SGRP };


/*
======================================================================
ui_get()

XPanels callback.  Called when the panel needs to read a value from
our data (e.g. when initializing a control).
====================================================================== */

static void *ui_get( AtmosphereData *dat, unsigned long vid )
{
   if ( !dat ) return NULL;

   switch ( vid ) {
      case ID_MARCH:  return &dat->march;
      case ID_HI:     return &dat->hi;
      case ID_LO:     return &dat->lo;
      case ID_FA:     return &dat->fa;
      case ID_DEN:    return &dat->den;
      case ID_LUM:    return &dat->lum;
      case ID_OPA:    return &dat->opa;
      case ID_BCK:    return &dat->bck;
      case ID_COL:    return dat->col;
      case ID_RES:    return &dat->res;
      case ID_TXTR:   return &dat->useTxtr;
      default:
         return NULL;
   }
}


/*
======================================================================
ui_set()

XPanels callback.  Called when a value has changed.
====================================================================== */

static int ui_set( AtmosphereData *dat, unsigned long vid, void *value )
{
   double *d = ( double * ) value;
   int *i = ( int * ) value;

   if ( !dat ) return 0;

   switch ( vid ) {
      case ID_MARCH:  dat->march   = *i;    return 1;
      case ID_HI:     dat->hi      = *d;    return 1;
      case ID_LO:     dat->lo      = *d;    return 1;
      case ID_FA:     dat->fa      = *d;    return 1;
      case ID_LUM:    dat->lum     = *d;    return 1;
      case ID_OPA:    dat->opa     = *d;    return 1;
      case ID_DEN:    dat->den     = *d;    return 1;
      case ID_BCK:    dat->bck     = *i;    return 1;
      case ID_RES:    dat->res     = *i;    return 1;
      case ID_TXTR:   dat->useTxtr = *i;    return 1;
      case ID_COL:    VCPY( dat->col, d );  return 1;
      default:
         return 0;
   }
}


/*
======================================================================
destroy_notify()

XPanels callback.  Called after the panel has been destroyed.
====================================================================== */

static void destroy_notify( AtmosphereData *dat )
{
   if ( dat )
      if ( dat->txedID ) {
         txedf->unsubscribe( dat->txedID );
         dat->txedID = NULL;
      }
}


/*
======================================================================
button_event()

XPanels callback.  Called when the user presses the "Edit Texture"
button.
====================================================================== */

static void button_event( LWXPanelID panel, unsigned long cid )
{
   AtmosphereData *dat;

   dat = xpanf->getData( panel, ID_TXBT );
   if ( !dat ) return;

   if ( !dat->txtr )
      dat->txtr = txtrf->create( TRT_COLOR, "FogTexture", NULL, NULL );
   txedf->open( dat->txedID, dat->txtr, NULL );
}


/*
======================================================================
get_panel()

Create and initialize an xpanel.
====================================================================== */

static LWXPanelID get_panel( AtmosphereData *dat )
{
   static LWXPanelControl ctrl_list[] = {
      { ID_MARCH, "Render Type",        "iPopChoice" },
      { ID_HI,    "Top",                "distance"   },
      { ID_LO,    "Bottom",             "distance"   },
      { ID_FA,    "Falloff",            "percent"    },
      { ID_DEN,   "Nominal Distance",   "distance"   },
      { ID_LUM,   "Luminosity",         "percent"    },
      { ID_OPA,   "Opacity",            "percent"    },
      { ID_BCK,   "Use Backdrop Color", "iBoolean"   },
      { ID_COL,   "Color",              "color"      },
      { ID_RES,   "Quality",            "iPopChoice" },
      { ID_TXTR,  "Use Texture",        "iBoolean"   },
      { ID_TXBT,  "Edit Texture",       "vButton"    },
      { 0 }
   };

   static LWXPanelDataDesc data_descrip[] = {
      { ID_MARCH, "Render Type",        "integer"    },
      { ID_HI,    "Top",                "distance"   },
      { ID_LO,    "Bottom",             "distance"   },
      { ID_FA,    "Falloff",            "percent"    },
      { ID_DEN,   "Nominal Distance",   "distance"   },
      { ID_LUM,   "Luminosity",         "percent"    },
      { ID_OPA,   "Opacity",            "percent"    },
      { ID_BCK,   "Use Backdrop Color", "integer"    },
      { ID_COL,   "Color",              "color"      },
      { ID_RES,   "Quality",            "integer"    },
      { ID_TXTR,  "Use Texture",        "integer"    },
      { ID_TXBT,  "Edit Texture",       NULL         },
      { 0 },
   };

   static char *quality[] = { "Very Low", "Low", "Medium", "Good", "Very Good", NULL };
   static char *render[] = { "Fast Fog", "Ray Marcher", NULL };
   static int bckMap[] = { 1, 0 };

   static LWXPanelHint hint[] = {
      XpLABEL( 0, "VFog Settings" ),
      XpDESTROYNOTIFY( destroy_notify ),
      XpBUTNOTIFY( ID_TXBT, button_event ),
      XpSTRLIST( ID_RES, quality ),
      XpSTRLIST( ID_MARCH, render ),

      XpGROUP_( ID_GGRP ),
         XpH( ID_HI ), XpH( ID_LO ), XpH( ID_FA ), XpH( ID_DEN ), XpEND,
      XpGROUP_( ID_SGRP ),
         XpH( ID_LUM ), XpH( ID_OPA ), XpH( ID_BCK ), XpH( ID_COL ), XpEND,
      XpGROUP_( ID_RGRP ),
         XpH( ID_RES ), XpH( ID_TXTR ), XpH( ID_TXBT ), XpEND,

      XpENABLE_( ID_MARCH ),
         XpH( ID_RGRP ), XpEND,
      XpENABLE_MAP_( ID_BCK, bckMap ),
         XpH( ID_COL ), XpEND,

      XpMIN( ID_LUM, 0 ), XpMAX( ID_LUM, 1 ),
      XpMIN( ID_OPA, 0 ), XpMAX( ID_OPA, 1 ),
      XpMIN( ID_FA, 0 ),  XpMAX( ID_FA, 1 ),

      XpEND
   };

   LWXPanelID panel;


   panel = xpanf->create( LWXP_VIEW, ctrl_list );
   if ( panel ) {
      xpanf->describe( panel, data_descrip, ui_get, ui_set );
      xpanf->hint( panel, 0, hint );
      xpanf->viewInst( panel, dat );
      xpanf->setData( panel, ID_TXBT, dat );
      xpanf->setData( panel, 0, dat );

      if ( !dat->txedID )
         dat->txedID = txedf->subscribe( "FogTexture",
            TEF_ALL - TEF_USEBTN, NULL, NULL, NULL, NULL );
   }

   return panel;
}


/*
======================================================================
Atmosphere_UI()

Interface activation function.  Create an xpanel and fill in the
fields of the LWInterface structure.
====================================================================== */

XCALL_( static int )
Atmosphere_UI( long version, GlobalFunc *global, LWInterface *local,
   void *serverData )
{
   if ( version != LWINTERFACE_VERSION ) return AFUNC_BADVERSION;

   txedf = global( LWTXTREDFUNCS_GLOBAL, GFUSE_TRANSIENT );
   xpanf = global( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT );
   if ( !txedf || !xpanf ) return AFUNC_BADGLOBAL;

   local->panel = get_panel( local->inst );
   local->options = NULL;
   local->command = NULL;

   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { LWVOLUMETRIC_HCLASS, "Demo_GroundFog", Atmosphere },
   { LWVOLUMETRIC_ICLASS, "Demo_GroundFog", Atmosphere_UI },
   { NULL }
};
