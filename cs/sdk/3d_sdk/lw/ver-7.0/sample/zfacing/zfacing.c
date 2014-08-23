/*
======================================================================
zfacing.c()

Demonstrate the mesh edit OPSEL_MODIFY mode.

Ernie Wright  28 Mar 00

This is both a mesh edit and a command sequence plug-in.  CS plug-ins
can use OPSEL_MODIFY mesh edits to directly select points and polygons
by ID.
====================================================================== */

#include <lwserver.h>
#include <lwmeshedt.h>
#include <lwcmdseq.h>
#include <stdlib.h>
#include <math.h>


/*
======================================================================
polscan()

The callback passed to the MeshEditOp polyScan() function.  If it's
not a face with a valid normal, we ignore it.  We select it if its
normal is dominated by a +z component, otherwise we deselect it.
====================================================================== */

XCALL_( static EDError )
polscan( MeshEditOp *edit, const EDPolygonInfo *pi )
{
   double n[ 3 ];

   if ( pi->type != LWPOLTYPE_FACE )
      return EDERR_NONE;

   if ( !edit->polyNormal( edit->state, pi->pol, n ))
      return EDERR_NONE;

   return edit->polSelect( edit->state, pi->pol,
      n[ 2 ] > fabs( n[ 0 ] ) && n[ 2 ] > fabs( n[ 1 ] ));
}


/*
======================================================================
meZFacing()

The mesh edit activation function.
====================================================================== */

XCALL_( static int )
meZFacing( long version, GlobalFunc *global, MeshEditBegin *local,
   void *serverData )
{
   MeshEditOp *edit;
   EDError err = EDERR_NONE;

   edit = local( 0, 0, OPSEL_DIRECT | OPSEL_MODIFY );
   if ( !edit ) return AFUNC_BADLOCAL;

   err = edit->polyScan( edit->state, polscan, edit, OPLYR_PRIMARY );
   edit->done( edit->state, err, 0 );

   return AFUNC_OK;
}


/*
======================================================================
csZFacing()

The command sequence activation function.
====================================================================== */

XCALL_( static int )
csZFacing( long version, GlobalFunc *global, LWModCommand *local,
   void *serverData )
{
   MeshEditOp *edit;
   EDError err = EDERR_NONE;

   /* ... some commands ... */

   edit = local->editBegin( 0, 0, OPSEL_DIRECT | OPSEL_MODIFY );
   if ( !edit ) return AFUNC_BADLOCAL;

   err = edit->polyScan( edit->state, polscan, edit, OPLYR_PRIMARY );
   edit->done( edit->state, err, 0 );

   /* ... some more commands ... */

   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { LWMESHEDIT_CLASS, "MeshEdit_ZFacing", meZFacing },
   { LWMODCOMMAND_CLASS, "Command_ZFacing", csZFacing },
   { NULL }
};
