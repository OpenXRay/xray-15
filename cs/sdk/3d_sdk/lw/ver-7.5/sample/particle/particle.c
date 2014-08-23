/*
======================================================================
particle.c

Demonstrates the particle system global.

Gregory Duquesne, NewTek LP
Ernie Wright  6 May 00

To test this, add it to an object, then add Hypervoxels and activate
the object.
====================================================================== */

#include <stdlib.h>
#include <lwserver.h>
#include <lwhandler.h>
#include <lwdisplce.h>
#include <lwrender.h>
#include <lwprtcl.h>


/* our instance data */

typedef struct st_ptData {
   LWPSysID  psys;
   LWPSBufID pos, col, enb;
   LWItemID  id;
} ptData;


LWPSysFuncs *pf;     /* particle system global */


/*
======================================================================
ptCreate()

Handler callback.  Create a new instance.
====================================================================== */

XCALL_( static LWInstance )
ptCreate( void *priv, LWItemID context, LWError *err )
{
   ptData *inst;
   float pos[ 3 ] = { 0.0f };

   inst = malloc( sizeof( ptData ));
   if ( !inst ) {
      *err = "Couldn't allocate instance data.";
      return NULL;
   }

   /* create particle system with position, state and color buffers */

   inst->psys = pf->create( LWPSB_POS | LWPSB_ENB | LWPSB_RGBA, LWPST_PRTCL );

   /* get the position, state and color buffer IDs */

   inst->pos = pf->getBufID( inst->psys, LWPSB_POS );
   inst->col = pf->getBufID( inst->psys, LWPSB_RGBA );
   inst->enb = pf->getBufID( inst->psys, LWPSB_ENB );

   /* Attach the particle sytem to the object.  This is important for
      clients like HyperVoxels that need to look up particle information
      using the itemID.  We need to remember the item ID so that we can
      call detach() in ptDestroy(). */

   inst->id = context;
   pf->attach( inst->psys, inst->id );
/*
   pf->addParticle( inst->psys );
   pf->setParticle( inst->pos, 0, pos ); */

   return inst;
}


/*
======================================================================
ptDestroy()

Handler callback.  Called when the instance is being destroyed.
====================================================================== */

XCALL_( static void )
ptDestroy( ptData *inst )
{
   pf->detach( inst->psys, inst->id );
   pf->destroy( inst->psys );

   free( inst );
}


/*
======================================================================
ptCopy()

Handler callback.  Called when Layout needs to make a copy of our
instance data.  This is called after ptCreate() makes the new
instance, and there's nothing to do after that, so we just return
NULL.
====================================================================== */

XCALL_( static LWError )
ptCopy( ptData *to, ptData *from )
{
   return NULL;
}


/*
======================================================================
ptDescLn()

Handler callback.  Return a one-line description of the instance data.
====================================================================== */

XCALL_( static const char * )
ptDescLn( ptData *inst )
{
   return "Particle System global demo";
}


/*
======================================================================
ptInit()

Handler callback.  Called when rendering begins.
====================================================================== */

XCALL_( static LWError )
ptInit( ptData *inst, int mode )
{
   return NULL;
}


/*
======================================================================
ptNewtime()

Handler callback.  Called at the start of each render time.  We call
pf->cleanup() to free the buffer memory allocated by addParticle() in
ptEvaluate() for the previous time.
====================================================================== */

XCALL_( static LWError )
ptNewtime( ptData *inst, LWFrame f, LWTime t )
{
   pf->cleanup( inst->psys );

   return NULL;
}


/*
======================================================================
ptCleanup()

Handler callback.  Called when rendering is finished.
====================================================================== */

void ptCleanup( ptData *inst )
{
   return;
}


/*
======================================================================
ptFlags()

Handler callback.  We ask for world coordinate point positions.
====================================================================== */

XCALL_( unsigned int )
ptFlags( ptData *inst )
{
   return LWDMF_WORLD;
}


/*
======================================================================
ptEvaluate()

Handler callback.  This is called for each vertex of the object.  We
add a particle for each vertex and set its position, display color and
enable state.

Note that we don't have to use the object's vertices to define the
particles.  This is just a convenience for demo purposes.
====================================================================== */

XCALL_( static void )
ptEvaluate( ptData *inst, LWDisplacementAccess *da )
{
   unsigned char rgba[ 4 ] = { 100, 150, 200, 255 }, enable = 1;
   float pos[ 3 ];
   int index;

   /* add a new particle */

   index = pf->addParticle( inst->psys );

   /* set particle position */

   pos[ 0 ] = ( float ) da->source[ 0 ];
   pos[ 1 ] = ( float ) da->source[ 1 ];
   pos[ 2 ] = ( float ) da->source[ 2 ];
   pf->setParticle( inst->pos, index, pos );

   /* set particle color */

   pf->setParticle( inst->col, index, rgba );

   /* set particle state */

   pf->setParticle( inst->enb, index, &enable );
}


/*
======================================================================
PTestActivate()

The activation function.
====================================================================== */

XCALL_( static int )
PTestActivate( long version, GlobalFunc *global, LWDisplacementHandler *local,
   void *serverData )
{
   if ( version != LWDISPLACEMENT_VERSION )
      return AFUNC_BADVERSION;

   pf = global( LWPSYSFUNCS_GLOBAL, GFUSE_TRANSIENT );
   if ( !pf ) return AFUNC_BADGLOBAL;

   local->inst->create  = ptCreate;
   local->inst->destroy = ptDestroy;
   local->inst->copy    = ptCopy;
   local->inst->descln  = ptDescLn;

   local->rend->init    = ptInit;
   local->rend->newTime = ptNewtime;
   local->rend->cleanup = ptCleanup;

   local->evaluate      = ptEvaluate;
   local->flags         = ptFlags;

   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { LWDISPLACEMENT_HCLASS, "Demo_Particle", PTestActivate },
   { NULL }
};
