/*
======================================================================
specular.c

Reproduce LW's specular highlight.

Ernie Wright  28 Mar 01

   L  vector that points from the surface spot to the light
   N  surface normal
   V  vector pointing from the spot to the ray source
   H  halfway vector between L and V
   x  Phong illumination exponent

   L = direction returned by LWShaderAccess->illuminate()
   N = sa->wNorm (this includes the effect of bump mapping)
   V = sa->wPos - sa->raySource
   H = (L + V) / |L + V|
   x = 1.0 / LWShaderAccess->roughness

   spec = LWShaderAccess->specular * pow(-dot(H, N), x)
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

typedef struct st_Light {
   LWItemID    id;
   LWFVector   wpos;
   LWFVector   L;
   LWFVector   color;
   int         visible;
} Light;

typedef struct st_HotSpot {
   GlobalFunc *global;
   int         nlights;
   Light *     light;
   LWFVector   ambient;
} HotSpot;


/*
======================================================================
Some vector math.
====================================================================== */

float dot( LWFVector a, LWFVector b )
{
   return a[ 0 ] * b[ 0 ] + a[ 1 ] * b[ 1 ] + a[ 2 ] * b[ 2 ];
}


void normalize( LWFVector v )
{
   float r;

   r = ( float ) sqrt( dot( v, v ));
   if ( r > 0 ) {
      v[ 0 ] /= r;
      v[ 1 ] /= r;
      v[ 2 ] /= r;
   }
}


void halfway( LWFVector a, LWFVector b, LWFVector c )
{
   c[ 0 ] = a[ 0 ] + b[ 0 ];
   c[ 1 ] = a[ 1 ] + b[ 1 ];
   c[ 2 ] = a[ 2 ] + b[ 2 ];
   normalize( c );
}


/*
======================================================================
Create()
====================================================================== */

XCALL_( static LWInstance )
Create( void *priv, LWSurfaceID surf, LWError *err )
{
   HotSpot *inst;

   inst = calloc( 1, sizeof( HotSpot ));
   if ( !inst ) {
      *err = "Couldn't allocate memory for instance.";
      return NULL;
   }

   inst->global = ( GlobalFunc * ) priv;
   return inst;
}


/*
======================================================================
Destroy()
====================================================================== */

XCALL_( static void )
Destroy( HotSpot *inst )
{
   free( inst );
}


/*
======================================================================
Don't need these to do anything much.
====================================================================== */

XCALL_( static LWError )
Copy( HotSpot *to, HotSpot *from ) { return NULL; }

XCALL_( static LWError )
Load( HotSpot *inst, const LWLoadState *ls ) { return NULL; }

XCALL_( static LWError )
Save( HotSpot *inst, const LWSaveState *ss ) { return NULL; }

XCALL_( static const char * )
DescLn( HotSpot *inst ) { return "Specular demo shader"; }


/*
======================================================================
Init()

Handler callback, called at the start of rendering.  Count the lights
in the scene and get their IDs.
====================================================================== */

XCALL_( static LWError )
Init( HotSpot *inst, int mode )
{
   LWItemInfo *iteminfo;
   LWItemID id;
   int i;

   inst->nlights = 0;

   iteminfo = inst->global( LWITEMINFO_GLOBAL, GFUSE_TRANSIENT );
   if ( !iteminfo ) return NULL;
   id = iteminfo->first( LWI_LIGHT, LWITEM_NULL );

   while ( id ) {
      ++inst->nlights;
      id = iteminfo->next( id );
   }

   if ( !inst->nlights )
      return NULL;

   inst->light = calloc( inst->nlights, sizeof( Light ));
   if ( !inst->light ) {
      inst->nlights = 0;
      return NULL;
   }

   id = iteminfo->first( LWI_LIGHT, LWITEM_NULL );
   for ( i = 0; i < inst->nlights; i++ ) {
      inst->light[ i ].id = id;
      id = iteminfo->next( id );
   }

   return NULL;
}


/*
======================================================================
Cleanup()

Handler callback, called at the end of rendering.  Free the instance
light array.
====================================================================== */

XCALL_( static void )
Cleanup( HotSpot *inst )
{
   if ( inst->light ) {
      free( inst->light );
      inst->light = NULL;
      inst->nlights = 0;
   }
}


/*
======================================================================
NewTime()

Handler callback, called at the start of each sampling pass.  Get each
light's position, and the ambient light color.
====================================================================== */

XCALL_( static LWError )
NewTime( HotSpot *inst, LWFrame f, LWTime t )
{
   LWItemInfo *iteminfo;
   LWLightInfo *lightinfo;
   LWDVector pos, color;
   int i;

   iteminfo = inst->global( LWITEMINFO_GLOBAL, GFUSE_TRANSIENT );
   if ( !iteminfo ) return NULL;

   for ( i = 0; i < inst->nlights; i++ ) {
      iteminfo->param( inst->light[ i ].id, LWIP_W_POSITION, t, pos );
      inst->light[ i ].wpos[ 0 ] = ( float ) pos[ 0 ];
      inst->light[ i ].wpos[ 1 ] = ( float ) pos[ 1 ];
      inst->light[ i ].wpos[ 2 ] = ( float ) pos[ 2 ];
   }

   lightinfo = inst->global( LWLIGHTINFO_GLOBAL, GFUSE_TRANSIENT );
   if ( lightinfo ) {
      lightinfo->ambient( t, color );
      inst->ambient[ 0 ] = ( float ) color[ 0 ];
      inst->ambient[ 1 ] = ( float ) color[ 1 ];
      inst->ambient[ 2 ] = ( float ) color[ 2 ];
   }

   return NULL;
}


/*
======================================================================
Flags()

Handler callback.  Return flags for the channels to be altered.
====================================================================== */

XCALL_( static unsigned int )
Flags( HotSpot *inst )
{
   return LWSHF_COLOR | LWSHF_RAYTRACE;
}


/*
======================================================================
Evaluate()

Handler callback.  Called for each sample on our surface.  We allocate
and work with a copy of the light array so that we're thread safe.  We
also make float-precision copies of some shader access data, since the
values are floats internally and the low bits of the double mantissas
are spurious.

Derived from a 29 Mar 01 post to the LW plug-in mailing list by Prem
Subrahmanyam.
====================================================================== */

XCALL_( static void )
Evaluate( HotSpot *inst, LWShaderAccess *sa )
{
   Light *light;
   LWDVector dir, lcolor;
   LWFVector N, V, H, color = { 0.0f };
   float lam, spec, x;
   int i;

   if ( !inst->nlights ) return;

   light = calloc( inst->nlights, sizeof( Light ));
   if ( !light ) return;
   memcpy( light, inst->light, inst->nlights * sizeof( Light ));

   N[ 0 ] = ( float ) sa->wNorm[ 0 ];
   N[ 1 ] = ( float ) sa->wNorm[ 1 ];
   N[ 2 ] = ( float ) sa->wNorm[ 2 ];

   x = ( float )( 1.0 / sa->roughness );

   V[ 0 ] = ( float )( sa->wPos[ 0 ] - sa->raySource[ 0 ] );
   V[ 1 ] = ( float )( sa->wPos[ 1 ] - sa->raySource[ 1 ] );
   V[ 2 ] = ( float )( sa->wPos[ 2 ] - sa->raySource[ 2 ] );
   normalize( V );

   /* ambient */

   color[ 0 ] = ( float )( inst->ambient[ 0 ] * sa->color[ 0 ] );
   color[ 1 ] = ( float )( inst->ambient[ 1 ] * sa->color[ 1 ] );
   color[ 2 ] = ( float )( inst->ambient[ 2 ] * sa->color[ 2 ] );

   /* diffuse */

   for ( i = 0; i < inst->nlights; i++ ) {
      light[ i ].visible = sa->illuminate( light[ i ].id, sa->wPos, dir, lcolor );
      if ( !light[ i ].visible ) continue;

      light[ i ].L[ 0 ] = ( float ) dir[ 0 ];
      light[ i ].L[ 1 ] = ( float ) dir[ 1 ];
      light[ i ].L[ 2 ] = ( float ) dir[ 2 ];
      normalize( light[ i ].L );

      light[ i ].color[ 0 ] = ( float ) lcolor[ 0 ];
      light[ i ].color[ 1 ] = ( float ) lcolor[ 1 ];
      light[ i ].color[ 2 ] = ( float ) lcolor[ 2 ];

      lam = -dot( light[ i ].L, N );
      if ( lam < 0.0 ) continue;

      color[ 0 ] += ( float )( sa->diffuse * sa->color[ 0 ] * lcolor[ 0 ] * lam );
      color[ 1 ] += ( float )( sa->diffuse * sa->color[ 1 ] * lcolor[ 1 ] * lam );
      color[ 2 ] += ( float )( sa->diffuse * sa->color[ 2 ] * lcolor[ 2 ] * lam );
   }

   /* specular */

   for ( i = 0; i < inst->nlights; i++ ) {
      if ( !light[ i ].visible ) continue;

      halfway( light[ i ].L, V, H );
      spec = -dot( H, N );
      if ( spec < 0.0f ) continue;
      spec = ( float )( sa->specular * pow( spec, x ));

      color[ 0 ] += light[ i ].color[ 0 ] * spec;
      color[ 1 ] += light[ i ].color[ 1 ] * spec;
      color[ 2 ] += light[ i ].color[ 2 ] * spec;
   }

   free( light );

   /* replace LW's diffuse and specular */

   sa->color[ 0 ] = color[ 0 ];
   sa->color[ 1 ] = color[ 1 ];
   sa->color[ 2 ] = color[ 2 ];
   sa->luminous = 1.0;
   sa->diffuse  = 0.0;
   sa->specular = 0.0;
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

   local->inst->priv     = global;
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


ServerRecord ServerDesc[] = {
   { LWSHADER_HCLASS, "Demo_Specular", Handler },
   { NULL }
};
