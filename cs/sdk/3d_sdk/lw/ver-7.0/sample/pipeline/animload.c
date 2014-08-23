/*
======================================================================
animload.c

Animation loader handler for the pipeline tracer.  See pipeline.c.

Ernie Wright  16 Jul 01
====================================================================== */

#include <lwanimlod.h>
#include <string.h>
#include "pipeline.h"


typedef struct {
   int           w, h, frames;
   double        rate;
   unsigned char scanline[ 32 ];
} AnimLoadInst;


XCALL_( static LWInstance )
Create( void *priv, char *filename, LWError *err )
{
   AnimLoadInst *dat;
   char buf[ 80 ];
   FILE *fp;

   trace( "Create", ANLD_PNAME, "%s", filename );

   fp = fopen( filename, "r" );
   if ( !fp ) {
      *err = "Couldn't open anim file.";
      return NULL;
   }

   fgets( buf, sizeof( buf ), fp );
   if ( strncmp( buf, "Pipeline AnimLoader File", 24 )) {
      fclose( fp );
      return NULL;
   }

   dat = calloc( 1, sizeof( AnimLoadInst ));
   if ( dat ) {
      dat->w = sizeof( dat->scanline );
      dat->h = dat->w / 2;
      dat->frames = 60;
      dat->rate = 30.0;
   }

   return ( LWInstance ) dat;
}


XCALL_( static void )
Destroy( AnimLoadInst *dat )
{
   if ( dat ) {
      trace( "Destroy", ANLD_PNAME, NULL );
      free( dat );
   }
}


XCALL_( static LWError )
Copy( AnimLoadInst *to, AnimLoadInst *from )
{
   trace( "Copy", ANLD_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Load( AnimLoadInst *dat, const LWLoadState *ls )
{
   trace( "Load", ANLD_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Save( AnimLoadInst *dat, const LWSaveState *ls )
{
   trace( "Save", ANLD_PNAME, NULL );
   return NULL;
}


XCALL_( static const char * )
DescLn( AnimLoadInst *dat )
{
   trace( "DescLn", ANLD_PNAME, NULL );
   return ANLD_PNAME;
}


XCALL_( static int )
FrameCount( AnimLoadInst *dat )
{
   trace( "FrameCount", ANLD_PNAME, NULL );
   return dat->frames;
}


XCALL_( static double )
FrameRate( AnimLoadInst *dat )
{
   trace( "FrameRate", ANLD_PNAME, NULL );
   return dat->rate;
}


XCALL_( static double )
Aspect( AnimLoadInst *dat, int *w, int *h, double *paspect )
{
   trace( "Aspect", ANLD_PNAME, NULL );
   *w = dat->w;
   *h = dat->h;
   *paspect = 1.0;
   return ( double ) dat->w / dat->h;
}


XCALL_( static void )
Evaluate( AnimLoadInst *dat, double t, LWAnimFrameAccess *access )
{
   LWImageProtocolID ip;
   int i, result;

   trace( "Evaluate", ANLD_PNAME, NULL );
   ip = access->begin( access->priv_data, LWIMTYP_GREY8 );
   if ( !ip ) return;

   LWIP_SETSIZE( ip, dat->w, dat->h );
   LWIP_SETPARAM( ip, LWIMPAR_ASPECT, 0, 1.0f );

   for ( i = 0; i < dat->h; i++ ) {
      result = LWIP_SENDLINE( ip, i, dat->scanline );
      if ( result != IPSTAT_OK ) break;
   }

   LWIP_DONE( ip, result );
   access->done( access->priv_data, ip );
}


XCALL_( int )
AnimLoad( long version, GlobalFunc *global, LWAnimLoaderHandler *local,
   void *serverData )
{
   if ( version != LWANIMLOADER_VERSION ) return AFUNC_BADVERSION;

   local->inst->create   = Create;
   local->inst->destroy  = Destroy;
   local->inst->copy     = Copy;
   local->inst->load     = Load;
   local->inst->save     = Save;
   local->inst->descln   = DescLn;
   local->frameCount     = FrameCount;
   local->frameRate      = FrameRate;
   local->aspect         = Aspect;
   local->evaluate       = Evaluate;

   trace( "Activate", ANLD_PNAME, NULL );

   return AFUNC_OK;
}
