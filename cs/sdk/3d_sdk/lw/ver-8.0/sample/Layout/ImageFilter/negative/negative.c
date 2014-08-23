/*
======================================================================
negative.c

Sample image filter plug-in that inverts the color of the image.

Allen Hastings and Arnie Cachelin
revised by Stuart Ferguson  6/9/95
updated for LW 6 by Ernie Wright  20 Mar 00
====================================================================== */

#include <lwserver.h>
#include <lwfilter.h>
#include <lwmonitor.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


/*
======================================================================
NilCreate(), NilDestroy(), NilCopy()

Handler instance callbacks.

We're required to supply the create(), destroy() and copy() handler
callbacks, and the create() function has to return *something*, since
a NULL return value indicates failure.  But we don't have any real
instance data to manage, so we just return a string.
====================================================================== */

XCALL_( LWInstance )
NilCreate( void *priv, void *context, LWError *err )
{
   return ( LWInstance ) "NIL";
}


XCALL_( void ) NilDestroy ( LWInstance inst ) {}

XCALL_( LWError ) NilCopy ( LWInstance to, LWInstance from ) { return NULL; }


/*
======================================================================
NegFlags()

LightWave calls the flags function to determine what options are
needed by this instance of the filter--specifically, which buffers
other than the normal RGBA buffers should be available.

Since the negative operation only needs to know the RGB, the buffers
for which are always available, we don't need anything else and we
return 0.
====================================================================== */

XCALL_( static unsigned int )
NegFlags( LWInstance inst )
{
   return 0;
}


/*
======================================================================
NegProcess()

LightWave calls the process function to process each rendered frame.
The processing function has to call setRGB() and setAlpha() for every
pixel in the image, even if only a few, or none, are changed.

In this case the red, green and blue buffers are read and inverted
before being set, and the alpha buffer is simply copied.

The monitor in the filter access is used to track the progress of the
filter for the user and to check for user abort.
====================================================================== */

XCALL_( static void )
NegProcess( LWInstance inst, const LWFilterAccess *fa )
{
   LWFVector out;
   float *r, *g, *b, *a;
   int x, y;

   /* fire up the monitor */

   MON_INIT( fa->monitor, fa->height / 8 );

   for ( y = 0; y < fa->height; y++ ) {

      /* get each scanline */

      r = fa->getLine( LWBUF_RED, y );
      g = fa->getLine( LWBUF_GREEN, y );
      b = fa->getLine( LWBUF_BLUE, y );
      a = fa->getLine( LWBUF_ALPHA, y );

      for ( x = 0; x < fa->width; x++ ) {

         /* change each pixel value c to 1.0 - c (leave the alpha alone) */

         out[ 0 ] = 1.0f - r[ x ];
         out[ 1 ] = 1.0f - g[ x ];
         out[ 2 ] = 1.0f - b[ x ];

         /* set the new value */

         fa->setRGB( x, y, out );
         fa->setAlpha( x, y, a[ x ] );
      }

      /* once every 8 lines, step the monitor and check for abort */

      if (( y & 7 ) == 7 )
         if ( MON_STEP( fa->monitor )) return;
   }

   MON_DONE( fa->monitor );
}


/*
======================================================================
Activate()

Our activation function.

We need to fill in the fields of the LWImageFilterHandler so that
LightWave can find our callbacks.
====================================================================== */

XCALL_( int )
Activate( long version, GlobalFunc *global, LWImageFilterHandler *local,
   void *serverData )
{
   if ( version != LWIMAGEFILTER_VERSION ) return AFUNC_BADVERSION;

   local->inst->create  = NilCreate;
   local->inst->destroy = NilDestroy;
   local->inst->copy    = NilCopy;
   local->inst->load    = NULL;
   local->inst->save    = NULL;
   local->inst->descln  = NULL;

   if ( local->item ) {
      local->item->useItems = NULL;
      local->item->changeID = NULL;
   }

   local->process = NegProcess;
   local->flags   = NegFlags;

   return AFUNC_OK;
}


/*
======================================================================
The server description.
====================================================================== */

ServerRecord ServerDesc[] = {
   { LWIMAGEFILTER_HCLASS, "Negative", Activate },
   { NULL }
};
