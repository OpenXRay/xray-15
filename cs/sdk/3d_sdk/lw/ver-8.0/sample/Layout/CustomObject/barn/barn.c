/*
======================================================================
barn.c

A custom null in the shape of a barn.

Ernie Wright  16 Mar 00

This isn't entirely useless.  You sometimes want to double-check
programming assumptions about the effects of animation parameters on
the orientation of objects.  Creating this custom null is a convenient
way to get a simple oriented object into the scene.

From F.S. Hill, COMPUTER GRAPHICS, MacMillan, ISBN 0-02-354860-6.
====================================================================== */

#include <lwserver.h>
#include <lwhandler.h>
#include <lwrender.h>
#include <lwcustobj.h>
#include <lwhost.h>
#include <stdlib.h>


typedef struct {
   LWItemID    id;
   GlobalFunc *global;
} MyObject;


static double vert[ 10 ][ 3 ] = {
   0.0,  0.0,  0.0,
   1.0,  0.0,  0.0,
   1.0,  1.0,  0.0,
   0.5,  1.5,  0.0,
   0.0,  1.0,  0.0,
   0.0,  0.0, -1.0,
   1.0,  0.0, -1.0,
   1.0,  1.0, -1.0,
   0.5,  1.5, -1.0,
   0.0,  1.0, -1.0
};

static int edge[ 17 ][ 2 ] = {
   0, 1,  1, 2,  2, 3,  3, 4,  4, 0,  5, 6,  6, 7,  7, 8,
   8, 9,  9, 5,  0, 5,  1, 6,  2, 7,  3, 8,  4, 9,  1, 4,
   0, 2
};


/*
======================================================================
Create()

Handler callback.  Allocate and initialize instance data.  We don't
really have any instance data, but we need to return something, and
this at least shows what create() would normally be doing.
====================================================================== */

XCALL_( static LWInstance )
Create( void *priv, LWItemID item, LWError *err )
{
   MyObject *inst;

   if ( inst = malloc( sizeof( MyObject ))) {
      inst->id = item;
   }
   else
      *err = "Couldn't allocate 4 bytes!";

   return inst;
}


/*
======================================================================
Destroy()

Handler callback.  Free resources allocated by Create().
====================================================================== */

XCALL_( static void )
Destroy( MyObject *inst )
{
   if( inst ) free( inst );
}


/*
======================================================================
Copy()

Handler callback.  Copy instance data.
====================================================================== */

XCALL_( static LWError )
Copy( MyObject *to, MyObject *from )
{
   LWItemID id;

   id = to->id;
   *to = *from;
   to->id = id;

   return NULL;
}


/*
======================================================================
Load(), Save(), Describe(), UseItems(), ChangeID(), Init(), Cleanup(),
NewTime(), Flags()

We're a pretty simple plug-in, so we don't need to do anything in
these callbacks, but they're here if we want to fill them in later.
====================================================================== */

XCALL_( static LWError )
Load( MyObject *inst, const LWLoadState *ls ) { return NULL; }

XCALL_( static LWError )
Save( MyObject *inst, const LWSaveState *ss ) { return NULL; }

XCALL_( static const char * )
Describe( MyObject *inst ) { return "Basic Barn"; }

XCALL_( static const LWItemID * )
UseItems( MyObject *inst ) { return NULL; }

XCALL_( static void )
ChangeID( MyObject *inst, const LWItemID *ids ) { }

XCALL_( static LWError )
Init( MyObject *inst, int mode ) { return NULL; }

XCALL_( static void )
Cleanup( MyObject *inst ) { }

XCALL_( static LWError )
NewTime( MyObject *inst, LWFrame fr, LWTime t ) { return NULL; }

XCALL_( static unsigned int )
Flags( MyObject *inst ) { return 0; }


/*
======================================================================
Evaluate()

Handler callback.  This is called each time the custom object needs to
be redrawn.  We just draw our edges, letting Layout choose the color
based on selection state and user preference.
====================================================================== */

XCALL_( static void )
Evaluate( MyObject *inst, const LWCustomObjAccess *access )
{
   int i;

   for ( i = 0; i < 15; i++ )
      access->line( access->dispData,
         vert[ edge[ i ][ 0 ]], vert[ edge[ i ][ 1 ]], LWCSYS_OBJECT );
   
   access->setPattern( access->dispData, LWLPAT_DOT );
   for ( i = 15; i < 17; i++ )
      access->line( access->dispData,
         vert[ edge[ i ][ 0 ]], vert[ edge[ i ][ 1 ]], LWCSYS_OBJECT );
}


/*
======================================================================
Handler()

Handler activation function.  Check the version, get some globals, and
fill in the callback fields of the handler structure.
====================================================================== */

XCALL_( static int )
Handler( long version, GlobalFunc *global, LWCustomObjHandler *local,
   void *serverData)
{
   if ( version != LWCUSTOMOBJ_VERSION )
      return AFUNC_BADVERSION;

   local->inst->create  = Create;
   local->inst->destroy = Destroy;
   local->inst->load    = Load;
   local->inst->save    = Save;
   local->inst->copy    = Copy;
   local->inst->descln  = Describe;

   if ( local->item ) {
      local->item->useItems = UseItems;
      local->item->changeID = ChangeID;
   }

   if ( local->rend ) {
      local->rend->init    = Init;
      local->rend->cleanup = Cleanup;
      local->rend->newTime = NewTime;
   }

   local->evaluate = Evaluate;
   local->flags    = Flags;

   return AFUNC_OK;
}


/*
======================================================================
Options()

Interface callback.
====================================================================== */

XCALL_( static LWError )
Options( MyObject *inst )
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
   MyObject *inst = ( MyObject * ) local->inst;

   if ( version != LWINTERFACE_VERSION )
      return AFUNC_BADVERSION;

   local->panel   = NULL;
   local->options = Options;
   local->command = NULL;

   inst->global = global;

   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { LWCUSTOMOBJ_HCLASS, "BasicBarn", Handler },
   { LWCUSTOMOBJ_ICLASS, "BasicBarn", Interface },
   { NULL }
};
