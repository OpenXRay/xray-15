/*
======================================================================
tool.c

The MeshEditTool version of the box tutorial plug-in.

Ernie Wright  30 Jun 01
====================================================================== */

#include "box.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>


/*
======================================================================
calc_handles()

Precalculate the positions of the tool handles based on the size and
center.
====================================================================== */

void calc_handles( BoxData *box )
{
   int i;

   for ( i = 0; i < 3; i++ ) {
      box->hpos[ 0 ][ i ] = ( float ) box->center[ i ];
      box->hpos[ 1 ][ i ] = box->hpos[ 0 ][ i ]
         + ( float )( box->size[ i ] * 0.5 );
   }
}


/*
======================================================================
Build()

Generate geometry based on the tool settings.
====================================================================== */

static LWError Build( BoxData *box, MeshEditOp *edit )
{
   makebox( edit, box );
   box->update = LWT_TEST_NOTHING;
   return NULL;
}


/*
======================================================================
Test()
====================================================================== */

static int Test( BoxData *box )
{
   return box->update;
}


/*
======================================================================
End()
====================================================================== */

static void End( BoxData *box, int keep )
{
   box->update = LWT_TEST_NOTHING;
   box->active = 0;
}


/*
======================================================================
Help()

Set the hint text for the tool.
====================================================================== */

static const char *Help( BoxData *box, LWToolEvent *event )
{
   static char buf[] = "Box Tool Plug-in Tutorial";
   return buf;
}


/*
======================================================================
Draw()

Draw the tool.
====================================================================== */

static void Draw( BoxData *box, LWWireDrawAccess *draw )
{
   if ( !box->active ) return;

   draw->moveTo( draw->data, box->hpos[ 0 ], LWWIRE_SOLID );
   draw->lineTo( draw->data, box->hpos[ 1 ], LWWIRE_ABSOLUTE );

   box->dirty = 0;
}


/*
======================================================================
Dirty()

Tell Modeler whether the tool or the hint text needs to be redrawn.
====================================================================== */

static int Dirty( BoxData *box )
{
   return box->dirty ? LWT_DIRTY_WIREFRAME | LWT_DIRTY_HELPTEXT : 0;
}


/*
======================================================================
Count()

Returns the number of handles.
====================================================================== */

static int Count( BoxData *box, LWToolEvent *event )
{
   return box->active ? 2 : 0;
}


/*
======================================================================
Start()

Called for the first mouse-down event.  Stores the start point and
returns the index of the handle that should be considered active.
====================================================================== */

static int Start( BoxData *box, LWToolEvent *event )
{
   int i;

   if ( !box->active )
      box->active = 1;

   for ( i = 0; i < 3; i++ ) {
      box->center[ i ] = event->posSnap[ i ];
      box->size[ i ] = 0.0;
   }
   calc_handles( box );

   return 1;
}


/*
======================================================================
Handle()

Tell Modeler where our handle points are.  The return value is the
handle priority.
====================================================================== */

static int Handle( BoxData *box, LWToolEvent *event, int handle,
   LWDVector pos )
{
   if ( handle >= 0 && handle < 2 ) {
      pos[ 0 ] = box->hpos[ handle ][ 0 ];
      pos[ 1 ] = box->hpos[ handle ][ 1 ];
      pos[ 2 ] = box->hpos[ handle ][ 2 ];
   }

   return handle + 1;
}


/*
======================================================================
Adjust()

Modeler tells us where the user has moved our handle points.
====================================================================== */

static int Adjust( BoxData *box, LWToolEvent *event, int handle )
{
   /* account for constraint */

   if ( event->portAxis >= 0 ) {
      if ( event->flags & LWTOOLF_CONSTRAIN ) {
         int x, y, xaxis[] = { 1, 2, 0 }, yaxis[] = { 2, 0, 1 };
         x = xaxis[ event->portAxis ];
         y = yaxis[ event->portAxis ];
         if ( event->flags & LWTOOLF_CONS_X )
            event->posSnap[ x ] -= event->deltaSnap[ x ];
         else if ( event->flags & LWTOOLF_CONS_Y )
            event->posSnap[ y ] -= event->deltaSnap[ y ];
      }
   }

   if ( handle == 0 ) {  /* center */
      box->center[ 0 ] = event->posSnap[ 0 ];
      box->center[ 1 ] = event->posSnap[ 1 ];
      box->center[ 2 ] = event->posSnap[ 2 ];
   }
   else if ( handle == 1 ) {  /* corner */
      box->size[ 0 ] = 2.0 * fabs( event->posSnap[ 0 ] - box->center[ 0 ] );
      box->size[ 1 ] = 2.0 * fabs( event->posSnap[ 1 ] - box->center[ 1 ] );
      box->size[ 2 ] = 2.0 * fabs( event->posSnap[ 2 ] - box->center[ 2 ] );
   }

   calc_handles( box );
   box->dirty = 1;
   box->update = LWT_TEST_UPDATE;
   return handle;
}


/*
======================================================================
Event()

Called when a user action changes the status of the tool.
====================================================================== */

static void Event( BoxData *box, int code )
{
   switch ( code )
   {
      case LWT_EVENT_DROP:

      /* The drop action is caused when the user clicks in the blank
         area of the display or uses the keyboard equivalent.  If the
         tool is active, we force a rejection of any interactive
         action partly complete.  For inactive tools we drop through
         to the reset action. */

      if ( box->active ) {
         box->update = LWT_TEST_REJECT;
         break;
      }

      case LWT_EVENT_RESET:

      /* The reset action corresponds to the reset command on the
         numeric panel.  Resets are also implicit when the user drops
         an inactive tool, thus the passthru above. */

         box->size[ 0 ] = box->size[ 1 ] = box->size[ 2 ] = 1.0;
         box->center[ 0 ] = box->center[ 1 ] = box->center[ 2 ] = 0.0;
         strcpy( box->surfname, "Default" );
         strcpy( box->vmapname, "MyUVs" );
         box->update = LWT_TEST_UPDATE;
         box->dirty = 1;
         calc_handles( box );
         break;

      case LWT_EVENT_ACTIVATE:

      /* Activation can be triggered from the numeric window or with a
         keystroke, and it should restart the edit operation with its
         current settings. */

         box->update = LWT_TEST_UPDATE;
         box->active = 1;
         box->dirty = 1;
         break;
   }
}


/*
======================================================================
Done()

Called when the user drops the tool.
====================================================================== */

static void Done( BoxData *box )
{
   free( box );
}


/*
======================================================================
Activate()

Entry point for the plug-in.
====================================================================== */

XCALL_( int )
Activate( long version, GlobalFunc *global, LWMeshEditTool *local,
   void *serverData )
{
   BoxData *box;

   if ( version != LWMESHEDITTOOL_VERSION )
      return AFUNC_BADVERSION;

   if ( !get_xpanf( global )) return AFUNC_BADGLOBAL;
   box = new_box();
   if ( !box ) return AFUNC_OK;

   local->instance     = box;

   local->tool->done   = Done;
   local->tool->help   = Help;
   local->tool->count  = Count;
   local->tool->handle = Handle;
   local->tool->adjust = Adjust;
   local->tool->start  = Start;
   local->tool->draw   = Draw;
   local->tool->dirty  = Dirty;
   local->tool->event  = Event;
   local->tool->panel  = Panel;

   local->build        = Build;
   local->test         = Test;
   local->end          = End;

   return AFUNC_OK;
}


static ServerTagInfo srvtag[] = {
   { "Tutorial: Box 4", SRVTAG_USERNAME | LANGID_USENGLISH },
   { "create", SRVTAG_CMDGROUP },
   { "objects/primitives", SRVTAG_MENU },
   { "Tut Box 4", SRVTAG_BUTTONNAME },
   { "", 0 }
};

ServerRecord ServerDesc[] = {
   { LWMESHEDITTOOL_CLASS, "Tutorial_Box4", Activate, srvtag },
   { NULL }
};
