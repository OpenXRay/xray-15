/*
======================================================================
sqtool.c

Superquadric creation as a MeshEditTool.

Arnie Cachelin  21 Sep 00
Ernie Wright  24 Sep 00
====================================================================== */

#include <lwserver.h>
#include <lwmodtool.h>
#include "superq.h"

static int
   zax[] = { 1, 2, 0 },
   xax[] = { 2, 0, 1 },
   yax[] = { 0, 1, 2 };

static char *shapeNames[] = {
   "Ellipsoid",
   "Toroid",
   NULL
};


/*
======================================================================
setBoxCorners()

Figure out where the handles should be based on the tool settings.
====================================================================== */

void setBoxCorners( sqData *tool )
{
   int x, y, z, i;

   y = tool->axis;
   x = xax[ tool->axis ];
   z = zax[ tool->axis ];

   VCPY( tool->top[ 0 ], tool->center );
   VCPY( tool->bot[ 0 ], tool->center );

   tool->top[ 0 ][ y ] += ( float )( tool->rad[ y ] );
   tool->bot[ 0 ][ y ] -= ( float )( tool->rad[ y ] );

   for ( i = 1; i < 4; i++ ) {
      VCPY( tool->top[ i ], tool->top[ 0 ] );
      VCPY( tool->bot[ i ], tool->bot[ 0 ] );
   }

   tool->top[ 0 ][ x ] += ( float ) tool->rad[ x ];
   tool->bot[ 0 ][ x ] += ( float ) tool->rad[ x ];
   tool->top[ 0 ][ z ] += ( float ) tool->rad[ z ];
   tool->bot[ 0 ][ z ] += ( float ) tool->rad[ z ];

   tool->top[ 1 ][ x ] += ( float ) tool->rad[ x ];
   tool->bot[ 1 ][ x ] += ( float ) tool->rad[ x ];
   tool->top[ 1 ][ z ] -= ( float ) tool->rad[ z ];
   tool->bot[ 1 ][ z ] -= ( float ) tool->rad[ z ];

   tool->top[ 2 ][ x ] -= ( float ) tool->rad[ x ];
   tool->bot[ 2 ][ x ] -= ( float ) tool->rad[ x ];
   tool->top[ 2 ][ z ] -= ( float ) tool->rad[ z ];
   tool->bot[ 2 ][ z ] -= ( float ) tool->rad[ z ];

   tool->top[ 3 ][ x ] -= ( float ) tool->rad[ x ];
   tool->bot[ 3 ][ x ] -= ( float ) tool->rad[ x ];
   tool->top[ 3 ][ z ] += ( float ) tool->rad[ z ];
   tool->bot[ 3 ][ z ] += ( float ) tool->rad[ z ];

   VCPY( tool->holex, tool->center );
   VCPY( tool->holez, tool->center );
   if ( tool->shape ) {
      tool->holex[ x ] += ( float )( tool->diam * tool->rad[ x ] );
      tool->holez[ z ] += ( float )( tool->diam * tool->rad[ z ] );
   }
}


/*
======================================================================
SuperQ_Build()

Generate geometry based on the tool settings.
====================================================================== */

static LWError SuperQ_Build( sqData *tool, MeshEditOp *edit )
{
   int ok;
   switch ( tool->shape ) {
      case 0:  ok = ellipsoid( edit, tool );  break;
      case 1:  ok = toroid( edit, tool );     break;
      default: break;
   }
   tool->update = LWT_TEST_NOTHING;
   return ok ? NULL : "Failed";
}


/*
======================================================================
SuperQ_Test()
====================================================================== */

static int SuperQ_Test( sqData *tool )
{
   return tool->update;
}


/*
======================================================================
SuperQ_End()
====================================================================== */

static void SuperQ_End( sqData *tool, int keep )
{
   tool->update = LWT_TEST_NOTHING;
   tool->active = 0;
}


/*
======================================================================
SuperQ_Help()

Set the hint text for the tool.
====================================================================== */

static const char *SuperQ_Help( sqData *tool, LWToolEvent *event )
{
   static char buf[ 64 ];

   sprintf( buf, "SuperQuadric: %s", shapeNames[ tool->shape ] );
   return buf;
}


/*
======================================================================
SuperQ_Draw()

Draw the tool.
====================================================================== */

static void SuperQ_Draw( sqData *tool, LWWireDrawAccess *draw )
{
   LWFVector top[ 4 ], bot[ 4 ], del;
   int x, y, z, i;

   if ( !tool->active )
      return;

   y = tool->axis;
   x = xax[ tool->axis ];
   z = zax[ tool->axis ];

   /* bounding box */

   draw->moveTo( draw->data, tool->top[ 0 ], LWWIRE_DASH );
   draw->lineTo( draw->data, tool->top[ 1 ], LWWIRE_ABSOLUTE );
   draw->lineTo( draw->data, tool->top[ 2 ], LWWIRE_ABSOLUTE );
   draw->lineTo( draw->data, tool->top[ 3 ], LWWIRE_ABSOLUTE );
   draw->lineTo( draw->data, tool->top[ 0 ], LWWIRE_ABSOLUTE );

   draw->moveTo( draw->data, tool->bot[ 0 ], LWWIRE_DASH );
   draw->lineTo( draw->data, tool->bot[ 1 ], LWWIRE_ABSOLUTE );
   draw->lineTo( draw->data, tool->bot[ 2 ], LWWIRE_ABSOLUTE );
   draw->lineTo( draw->data, tool->bot[ 3 ], LWWIRE_ABSOLUTE );
   draw->lineTo( draw->data, tool->bot[ 0 ], LWWIRE_ABSOLUTE );

   draw->lineTo( draw->data, tool->top[ 0 ], LWWIRE_ABSOLUTE );
   draw->moveTo( draw->data, tool->top[ 3 ], LWWIRE_DASH );
   draw->lineTo( draw->data, tool->bot[ 3 ], LWWIRE_ABSOLUTE );
   draw->moveTo( draw->data, tool->bot[ 2 ], LWWIRE_DASH );
   draw->lineTo( draw->data, tool->top[ 2 ], LWWIRE_ABSOLUTE );
   draw->moveTo( draw->data, tool->top[ 1 ], LWWIRE_DASH );
   draw->lineTo( draw->data, tool->bot[ 1 ], LWWIRE_ABSOLUTE );

   /* center crosshair */

   for ( i = 0; i < 3; i++ ) {
      VCPY( top[ i ], tool->center );
      VCPY( bot[ i ], tool->center);
   }

   VSUB3( del, tool->top[ 0 ], tool->bot[ 2 ] );
   VSCL( del, 0.25f );

   top[ 0 ][ z ] += del[ z ];
   bot[ 0 ][ z ] -= del[ z ];
   top[ 1 ][ x ] += del[ x ];
   bot[ 1 ][ x ] -= del[ x ];
   top[ 2 ][ y ] += del[ y ];
   bot[ 2 ][ y ] -= del[ y ];

   draw->moveTo( draw->data, top[ 0 ], LWWIRE_SOLID );
   draw->lineTo( draw->data, bot[ 0 ], LWWIRE_ABSOLUTE );
   draw->moveTo( draw->data, top[ 1 ], LWWIRE_SOLID );
   draw->lineTo( draw->data, bot[ 1 ], LWWIRE_ABSOLUTE );
   draw->moveTo( draw->data, top[ 2 ], LWWIRE_SOLID );
   draw->lineTo( draw->data, bot[ 2 ], LWWIRE_ABSOLUTE );

   /* handles for toroid hole */

   if ( tool->shape ) {
      VCPY( del, tool->center );
      draw->moveTo( draw->data, del, LWWIRE_DASH );
      draw->lineTo( draw->data, tool->holex, LWWIRE_ABSOLUTE );
      draw->circle( draw->data, 0.025, LWWIRE_SCREEN );
      draw->moveTo( draw->data, del, LWWIRE_DASH );
      draw->lineTo( draw->data, tool->holez, LWWIRE_ABSOLUTE );
      draw->circle( draw->data, 0.025, LWWIRE_SCREEN );
   }

   tool->dirty = 0;
}


/*
======================================================================
SuperQ_Dirty()

Tell Modeler whether the tool or the hint text needs to be redrawn.
====================================================================== */

static int SuperQ_Dirty( sqData *tool )
{
   return tool->dirty ? LWT_DIRTY_WIREFRAME | LWT_DIRTY_HELPTEXT : 0;
}


/*
======================================================================
SuperQ_Count()

Returns the number of handles.
====================================================================== */

static int SuperQ_Count( sqData *tool, LWToolEvent *event )
{
   return tool->active ? 9 + 2 * tool->shape : 0;
}


/*
======================================================================
SuperQ_Start()

Called for the first mouse-down event.  Stores the start point and
returns the index of the handle that should be considered active.
====================================================================== */

static int SuperQ_Start( sqData *tool, LWToolEvent *event )
{
   if ( !tool->active )
      tool->active = 1;

   VCPY( tool->org, event->posSnap );
   VCPY_F( tool->center, event->posSnap );

   if ( event->portAxis >= 0 )
      tool->axis = event->portAxis;

   setBoxCorners( tool );

   return 0;
}


/*
======================================================================
SuperQ_Handle()

Tell Modeler where our handle points are.
====================================================================== */

static int SuperQ_Handle( sqData *tool, LWToolEvent *event, int handle,
   LWDVector pos )
{
   if ( handle <= 3 )
      VCPY( pos, tool->top[ handle ] );
   else if ( handle <= 7 )
      VCPY( pos, tool->bot[ handle - 4 ] );
   else if ( handle == 8 )
      VCPY( pos, tool->org );
   else if ( handle == 9 )
      VCPY( pos, tool->holex );
   else if ( handle == 10 )
      VCPY( pos, tool->holez );

   /* The center crosshair and the hole size handle will overlap in some
      views.  Give the center handle a higher priority. */

   return handle == 8 ? 2 : 1;
}


/*
======================================================================
SuperQ_Adjust()

Modeler tells us where the user has moved our handle points.
====================================================================== */

static int SuperQ_Adjust( sqData *tool, LWToolEvent *event, int handle )
{
   int x, y, z, i, j;

   y = tool->axis;
   x = xax[ tool->axis ];
   z = zax[ tool->axis ];

   if ( event->portAxis >= 0 ) {
      if ( event->flags & LWTOOLF_CONSTRAIN ) {
         int tx, ty, stxax[] = { 1, 2, 0 }, styax[] = { 2, 0, 1 };

         tx = stxax[ event->portAxis ];
         ty = styax[ event->portAxis ];
         if ( event->flags & LWTOOLF_CONS_X )
            event->posSnap[ tx ] -= event->deltaSnap[ tx ];
         else if ( event->flags & LWTOOLF_CONS_Y )
            event->posSnap[ ty ] -= event->deltaSnap[ ty ];
      }
   }

   /* dragging a corner, update the radius */

   if ( handle <= 7 ) {
      VSUB3( tool->rad, event->posSnap, tool->org );
      tool->rad[ 0 ] = ABS( tool->rad[ 0 ] );
      tool->rad[ 1 ] = ABS( tool->rad[ 1 ] );
      tool->rad[ 2 ] = ABS( tool->rad[ 2 ] );
      setBoxCorners( tool );
   }

   /* dragging the center, update the position */

   else if ( handle == 8 ) {
      LWDVector dif;

      VSUB3( dif, event->posSnap, tool->org );
      for ( j = 0; j < 4; j++ ) {
         for ( i = 0; i < 3; i++ ) {
            tool->top[ j ][ i ] += ( float ) dif[ i ];
            tool->bot[ j ][ i ] += ( float ) dif[ i ];
         }
      }
      for ( i = 0; i < 3; i++ ) {
         tool->holex[ i ] += ( float ) dif[ i ];
         tool->holez[ i ] += ( float ) dif[ i ];
      }
      VCPY( tool->org, event->posSnap );
      VCPY_F( tool->center, tool->org );
   }

   /* dragging the X hole size handle */

   else if ( handle == 9 ) {
      event->posSnap[ y ] = tool->org[ y ];
      event->posSnap[ z ] = tool->org[ z ];
      if ( tool->rad[ x ] > 1e-7f )
         tool->diam = ( event->posSnap[ x ] - tool->org[ x ] ) / tool->rad[ x ];
      if ( tool->diam < 1.0f )
         tool->diam = 1.0f;

      VCPY_F( tool->holex, event->posSnap );
      VCPY( tool->holez, tool->center );
      tool->holez[ z ] += ( float )( tool->diam * tool->rad[ z ] );
   }

   /* dragging the Z hole size handle */

   else if ( handle == 10 ) {
      event->posSnap[ y ] = tool->org[ y ];
      event->posSnap[ x ] = tool->org[ x ];
      if ( tool->rad[ z ] > 1e-7f )
         tool->diam = ( event->posSnap[ z ] - tool->org[ z ] ) / tool->rad[ z ];
      if ( tool->diam < 1.0f )
         tool->diam = 1.0f;

      VCPY_F( tool->holez, event->posSnap );
      VCPY(tool->holex, tool->center);
      tool->holex[ x ] += ( float )( tool->diam * tool->rad[ x ] );
   }

   tool->dirty = 1;
   tool->update = LWT_TEST_UPDATE;
   return handle;
}


/*
======================================================================
SuperQ_Event()

Called when a user action changes the status of the tool.
====================================================================== */

static void SuperQ_Event( sqData *tool, int code )
{
   switch ( code )
   {
      case LWT_EVENT_DROP:

      /* The drop action is caused when the user clicks in the blank
         area of the display or uses the keyboard equivalent.  If the
         tool is active, we force a rejection of any interactive
         action partly complete.  For inactive tools we drop through
         to the reset action. */

      if ( tool->active ) {
         tool->update = LWT_TEST_REJECT;
         break;
      }

      case LWT_EVENT_RESET:

      /* The reset action corresponds to the reset command on the
         numeric panel, and causes us to snap the SuperQ factor back
         to zero.  Resets are also implicit when the user drops an
         inactive tool, thus the passthru above. */

         tool->nsides = 24;
         tool->nsegments = 12;
         VSET( tool->org, 0.0 );
         VSET( tool->center, 0.0f );
         VSET( tool->holex, 0.0f );
         VSET( tool->holez, 0.0f );
         VSET( tool->rad, 0.5 );
         tool->shape = 0;
         tool->bf1 = 2.0;
         tool->bf2 = 2.0;
         tool->axis = 1;
         tool->diam = 0.50;
         tool->holex[ 0 ] = ( float )( tool->diam * 0.5 );
         tool->holez[ 2 ] = tool->holex[ 0 ];
         tool->update = LWT_TEST_UPDATE;
         tool->dirty = 1;
         setBoxCorners( tool );
         break;

      case LWT_EVENT_ACTIVATE:

      /* Activation can be triggered from the numeric window or with a
         keystroke, and it should restart the edit operation with its
         current settings. */

         tool->update = LWT_TEST_UPDATE;
         tool->active = 1;
         tool->dirty = 1;
         break;
   }
}


/*
======================================================================
SuperQ_Done()

Called when the user drops the tool.
====================================================================== */

static void SuperQ_Done( sqData *tool )
{
   free( tool );
}


/* ----- interface stuff ----- */

static LWXPanelFuncs *xpanf;

static double bf1 = 1.66, bf2 = 1.66;
static double org[ 3 ] = { 0.0, 0.0, 0.0 };
static double rad[ 4 ] = { 0.5, 0.5, 0.5, 2.0 };
static int nsides = 24, nsegments = 12, shape = 1, axis = 1;

enum {
   XID_ACTI = 0x8F00,
   XID_SID,
   XID_SEG,
   XID_TYPE,
   XID_BF1,
   XID_BF2,
   XID_CEN,
   XID_RAD,
   XID_DIAM,
   XID_UVS,
   XID_AXIS,
   XID_GRP1,
   XID_GRP2,
   XID_TAB
};


/*
======================================================================
SuperQ_Get()

Send XPanels the value of a control.
====================================================================== */

static void *SuperQ_Get( sqData *tool, unsigned long vid )
{
   switch ( vid ) {
      case XID_AXIS:  return &tool->axis;
      case XID_UVS:   return &tool->uvs;
      case XID_SID:   return &tool->nsides;
      case XID_SEG:   return &tool->nsegments;
      case XID_TYPE:  return &tool->shape;
      case XID_BF1:   return &tool->bf1;
      case XID_BF2:   return &tool->bf2;
      case XID_CEN:   return &tool->org;
      case XID_RAD:   return &tool->rad;
      case XID_DIAM:  return &tool->diam;
      case XID_ACTI:  return &tool->active;
   }
   return NULL;
}


/*
======================================================================
SuperQ_Set()

Store the value of an xpanel control.
====================================================================== */

static int SuperQ_Set( sqData *tool, unsigned long vid, void *value )
{
   switch ( vid )
   {
      case XID_AXIS:
         axis = tool->axis = *(( int * ) value );
         break;

      case XID_UVS:
         tool->uvs = *(( int * ) value );
         break;

      case XID_SID:
         nsides = tool->nsides = *(( int * ) value );
         if ( nsides < 2 ) nsides = tool->nsides = 2;
         break;

      case XID_SEG:
         nsegments = tool->nsegments = *(( int * ) value );
         if ( nsegments < 2 ) nsegments = tool->nsegments = 2;
         break;

      case XID_TYPE:
         shape = tool->shape = *(( int * ) value );
         break;

      case XID_BF1:
         bf1 = *(( double * ) value );
         tool->bf1 = bf1 < 0.01 ? 0.01 : bf1;
         bf1 = tool->bf1;
         break;

      case XID_BF2:
         bf2 = *(( double * ) value );
         tool->bf2 = bf2 < 0.01 ? 0.01 : bf2;
         bf2 = tool->bf2;
         break;

      case XID_CEN:
         VCPY( tool->org, ( double * ) value );
         VCPY( org, ( double * ) value );
         VCPY_F( tool->center, ( double * ) value );
         break;

      case XID_RAD:
         VCPY( tool->rad, ( double * ) value );
         VCPY( rad, ( double * ) value );
         break;

      case XID_DIAM:
         tool->diam = *(( double * ) value );
         rad[ 3 ] = tool->diam;
         break;

      case XID_ACTI:
         break;

      default:
         return 0;
   }

   tool->update = LWT_TEST_UPDATE;
   tool->dirty = 1;
   setBoxCorners( tool );

   return 1;
}


/*
======================================================================
SuperQ_Panel()

Create the numeric panel for the tool.
====================================================================== */

static LWXPanelID SuperQ_Panel( sqData *tool )
{
   static LWXPanelDataDesc def[] = {
      { XID_SID,  "Sides",      "integer"   },
      { XID_SEG,  "Segments",   "integer"   },
      { XID_TYPE, "Shape",      "integer"   },
      { XID_AXIS, "Axis",       "integer"   },
      { XID_CEN,  "Center",     "distance3" },
      { XID_RAD,  "Size",       "distance3" },
      { XID_DIAM, "Hole Size",  "distance"  },
      { XID_BF1,  "Top Bulge",  "float"     },
      { XID_BF2,  "Side Bulge", "float"     },
      { XID_ACTI, "--hidden--", "integer"   },
      { XID_UVS,  "Make UVs",   "integer"   },
      { 0 }
   };

   static LWXPanelControl con[] = {
      { XID_SID,  "Sides",      "integer"    },
      { XID_SEG,  "Segments",   "integer"    },
      { XID_TYPE, "Shape",      "iPopChoice" },
      { XID_AXIS, "Axis",       "axis"       },
      { XID_CEN,  "Center",     "distance3"  },
      { XID_RAD,  "Size",       "distance3"  },
      { XID_DIAM, "Hole Size",  "distance"   },
      { XID_BF1,  "Top Bulge",  "float"      },
      { XID_BF2,  "Side Bulge", "float"      },
      { XID_ACTI, "--hidden--", "integer"    },
      { XID_UVS,  "Make UVs",   "iBoolean"   },
      { 0 }
   };

   static LWXPanelHint hint[] = {
      XpDELETE( XID_ACTI ),
      XpSTRLIST( XID_TYPE, shapeNames ),
      XpENABLEMSG_( XID_ACTI, "Tool is currently inactive." ),
         XpH( XID_SID ),
         XpH( XID_SEG ),
         XpH( XID_AXIS ),
         XpH( XID_CEN ),
         XpH( XID_RAD ),
         XpH( XID_BF1 ),
         XpH( XID_BF2 ),
         XpH( XID_DIAM ),
         XpH( XID_TYPE ),
         XpH( XID_UVS ),
         XpEND,
      XpENABLEMSG_( XID_TYPE, "Ellipsoids don't have a hole!" ),
         XpH( XID_DIAM ),
         XpEND,
      XpMIN( XID_SID, 2 ),
      XpMAX( XID_SID, 256 ),
      XpMIN( XID_SEG, 2 ),
      XpMAX( XID_SEG, 256 ),
      XpMIN( XID_BF1, 0 ),
      XpMIN( XID_BF2, 0 ),
      XpEND
   };

   LWXPanelID pan;

   pan = xpanf->create( LWXP_VIEW, con );
   if ( !pan )
      return NULL;

   xpanf->describe( pan, def, SuperQ_Get, SuperQ_Set );
   xpanf->hint( pan, 0, hint );

   return pan;
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
   sqData *tool;

   if ( version != LWMESHEDITTOOL_VERSION )
      return AFUNC_BADVERSION;

   xpanf = global( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT );
   if ( !xpanf )
      return AFUNC_BADGLOBAL;

   tool = calloc( 1, sizeof( sqData ));
   if ( !tool )
      return AFUNC_OK;

   tool->nsides    = nsides;
   tool->nsegments = nsegments;
   tool->shape     = shape;
   tool->axis      = axis;
   tool->bf1       = bf1;
   tool->bf2       = bf2;
   tool->diam      = rad[ 3 ];
   tool->update    = LWT_TEST_NOTHING;
   tool->active    = 0;

   VCPY( tool->org, org );
   VCPY_F( tool->center, org );
   VCPY( tool->rad, rad );

   local->instance     = tool;
   local->tool->done   = SuperQ_Done;
   local->tool->help   = SuperQ_Help;
   local->tool->count  = SuperQ_Count;
   local->tool->handle = SuperQ_Handle;
   local->tool->adjust = SuperQ_Adjust;
   local->tool->start  = SuperQ_Start;
   local->tool->draw   = SuperQ_Draw;
   local->tool->dirty  = SuperQ_Dirty;
   local->tool->event  = SuperQ_Event;
   local->tool->panel  = SuperQ_Panel;
   local->build        = SuperQ_Build;
   local->test         = SuperQ_Test;
   local->end          = SuperQ_End;

   return AFUNC_OK;
}


/*
======================================================================
mk_tags[]

The ServerTagInfo array is new for LW 6.1 and replaces the ServerName
array.  In addition to an internationalized list of plug-in names, you
can also specify the command group, the menu in which the plug-in
will appear, and the text of the button that will be created in that
menu.
====================================================================== */

static ServerTagInfo mk_tags[] = {
   { "SuperQuadric Tool", SRVTAG_USERNAME | LANGID_USENGLISH },
   { "create", SRVTAG_CMDGROUP },
   { "objects/primitives", SRVTAG_MENU },
   { "", 0 }
};

ServerRecord ServerDesc[] = {
   { LWMESHEDITTOOL_CLASS, "SuperQuadric", Activate, mk_tags },
   { NULL }
};
