/*
======================================================================
wdbtool.c

The Windows debug version of tool.c.  Writes a trace of each callback
call to a text file.  Only one of these (wdbtool.c, tool.c) should be
included in your project, not both.

Ernie Wright  5 Jul 01
====================================================================== */

#include <windows.h>
#include "box.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* this is where the trace is written */

#define TR_FILENAME  "trace.txt"

static FILE *fp;
static HANDLE hinst;
static HHOOK hhook;

#define TR_DONE      0
#define TR_HELP      1
#define TR_COUNT     2
#define TR_HANDLE    3
#define TR_ADJUST    4
#define TR_START     5
#define TR_DRAW      6
#define TR_DIRTY     7
#define TR_EVENT     8
#define TR_PANEL     9
#define TR_BUILD     10
#define TR_TEST      11
#define TR_END       12
#define TR_MDOWN     13
#define TR_MMOVE     14
#define TR_MUP       15
#define TR_PANSET    16
#define TR_PANGET    17


void trace( int cb, BoxData *box, void *data )
{
   static char *line[] = {
      "Done",
      "Help     ",   // LWToolEvent *event
      "Count    ",   // LWToolEvent *event
      "Handle   ",   // int *handle
      "Adjust   ",   // int *handle
      "Start    ",   // LWToolEvent *event
      "Draw     ",   // LWWireDrawAccess *draw
      "Dirty    ",
      "Event    ",   // int *code
      "Panel    ",
      "Build    ",
      "Test     ",
      "End      ",   // int *keep
      "----mouse down----",
      "----mouse move----",
      "----mouse up------",
      "Pan Set  ",  // int *vid
      "Pan Get  "   // int *vid
   };

   SYSTEMTIME st;
   unsigned long vid;

   if ( fp ) {
      GetLocalTime( &st );
      fprintf( fp, "%02.2d:%02.2d:%02.2d.%03.3d  ",
         st.wHour, st.wMinute, st.wSecond, st.wMilliseconds );
      fprintf( fp, "%s", line[ cb ] );
      switch ( cb ) {
         case TR_HANDLE:
         case TR_ADJUST:
            fprintf( fp, "handle = %d\n", *(( int * ) data ));
            break;
         case TR_DRAW:
            fprintf( fp, "axis = %d\n", (( LWWireDrawAccess * ) data )->axis );
            break;
         case TR_HELP:
            fprintf( fp, "portAxis = %d\n", (( LWToolEvent * ) data )->portAxis );
            break;
         case TR_DIRTY:
            fprintf( fp, "box->dirty = %d\n", box->dirty );
            break;
         case TR_COUNT:
            fprintf( fp, "box->active = %d\n", box->active );
            break;
         case TR_TEST:
            fprintf( fp, "box->update = %d\n", box->update );
            break;
         case TR_END:
            fprintf( fp, "keep = %d\n", *(( int * ) data ));
            break;
         case TR_PANSET:
         case TR_PANGET:
            vid = *(( unsigned long * ) data );
            fprintf( fp, "vid = 0x%04.4x", vid );
            switch ( vid ) {
               case 0x8001:
                  fprintf( fp, "  %g %g %g\n",
                     box->size[ 0 ], box->size[ 1 ], box->size[ 2 ] );
                  break;
               case 0x8002:
                  fprintf( fp, "  %g %g %g\n",
                     box->center[ 0 ], box->center[ 1 ], box->center[ 2 ] );
                  break;
               case 0x8003:
                  fprintf( fp, "  %s\n", box->surfname );
                  break;
               case 0x8004:
                  fprintf( fp, "  %s\n", box->vmapname );
                  break;
               default:
                  fprintf( fp, "\n" );
                  break;
            }
            break;
         default:
            fprintf( fp, "\n" );
            break;
      }
   }
}


BOOL WINAPI DllMain( HANDLE hInstance, ULONG reason, LPVOID reserved )
{
   if ( reason == DLL_PROCESS_ATTACH )
      hinst = hInstance;

   return TRUE;
}


LRESULT CALLBACK mouse_hook( int code, WPARAM wp, LPARAM lp )
{
   static int down = 0;
   MOUSEHOOKSTRUCT *mhs = ( MOUSEHOOKSTRUCT * ) lp;

   if ( code >= 0 ) {
      switch ( wp ) {
         case WM_LBUTTONDOWN:
            down = 1;
            trace( TR_MDOWN, NULL, mhs );
            break;
         case WM_LBUTTONUP:
            down = 0;
            trace( TR_MUP, NULL, mhs );
            break;
         case WM_MOUSEMOVE:
            if ( down )
               trace( TR_MMOVE, NULL, mhs );
            break;
      }
   }

   return CallNextHookEx( hhook, code, wp, lp );
}


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
   trace( TR_BUILD, box, NULL );

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
   trace( TR_TEST, box, NULL );

   return box->update;
}


/*
======================================================================
End()
====================================================================== */

static void End( BoxData *box, int keep )
{
   trace( TR_END, box, &keep );

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

   trace( TR_HELP, box, event );
   return buf;
}


/*
======================================================================
Draw()

Draw the tool.
====================================================================== */

static void Draw( BoxData *box, LWWireDrawAccess *draw )
{
   trace( TR_DRAW, box, draw );

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
   trace( TR_DIRTY, box, NULL );

   return box->dirty ? LWT_DIRTY_WIREFRAME : 0;
}


/*
======================================================================
Count()

Returns the number of handles.
====================================================================== */

static int Count( BoxData *box, LWToolEvent *event )
{
   trace( TR_COUNT, box, event );

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

   trace( TR_START, box, event );

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
   trace( TR_HANDLE, box, &handle );

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
   trace( TR_ADJUST, box, &handle );

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
   trace( TR_EVENT, box, &code );

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
   trace( TR_DONE, box, NULL );
   free( box );
   if ( fp ) { fclose( fp ); fp = NULL; }
   if ( hhook ) {
      UnhookWindowsHookEx( hhook );
      hhook = NULL;
   }
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

   /* the debug trace file and a mouse hook */

   fp = fopen( TR_FILENAME, "w" );
   SetWindowsHookEx( WH_MOUSE, mouse_hook, hinst, 0 );

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
