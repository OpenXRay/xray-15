/* Flare2Alpha.c - An ImageFilter which copies missing RGB elements into the alpha channel 
 * by Arnie Cachelin, 2002
 */

#include <lwserver.h>
#include <lwfilter.h>
#include <lwmonitor.h>
#include <lwmath.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

XCALL_( LWInstance )
F2ACreate( void *priv, void *context, LWError *err )
{
   return ( LWInstance ) "NIL";
}


XCALL_( void ) F2ADestroy ( LWInstance inst ) {}

XCALL_( LWError ) F2ACopy ( LWInstance to, LWInstance from ) { return NULL; }


XCALL_( static unsigned int )
F2AFlags( LWInstance inst )
{
   return 0;
}


XCALL_( static void )
F2AProcess( LWInstance inst, const LWFilterAccess *fa )
{
	LWFVector out;
	float *r, *g, *b, *a, da;
	int x, y;

	MON_INIT( fa->monitor, fa->height / 8 );

	for ( y = 0; y < fa->height; y++ ) 
	{

		r = fa->getLine( LWBUF_RED, y );
		g = fa->getLine( LWBUF_GREEN, y );
		b = fa->getLine( LWBUF_BLUE, y );
		a = fa->getLine( LWBUF_ALPHA, y );

		for ( x = 0; x < fa->width; x++ ) 
		{
			da = MAX(r[x],g[x]);
			da = MAX(da, b[x]);
			a[x] = MAX(da, a[x]);

			out[ 0 ] = r[ x ];
			out[ 1 ] = g[ x ];
			out[ 2 ] = b[ x ];

			fa->setRGB( x, y, out );
			fa->setAlpha( x, y, a[ x ] );
		}
	if (( y & 7 ) == 7 )
		if ( MON_STEP( fa->monitor )) return;
	}

	MON_DONE( fa->monitor );
}


XCALL_( int )
Activate( long version, GlobalFunc *global, LWImageFilterHandler *local,
   void *serverData )
{
   if ( version != LWIMAGEFILTER_VERSION ) return AFUNC_BADVERSION;

   local->inst->create  = F2ACreate;
   local->inst->destroy = F2ADestroy;
   local->inst->copy    = F2ACopy;
   local->inst->load    = NULL;
   local->inst->save    = NULL;
   local->inst->descln  = NULL;

   if ( local->item ) {
      local->item->useItems = NULL;
      local->item->changeID = NULL;
   }

   local->process = F2AProcess;
   local->flags   = F2AFlags;

   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { LWIMAGEFILTER_HCLASS, "Flare2Alpha", Activate },
   { NULL }
};
