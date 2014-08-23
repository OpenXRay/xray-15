/*
======================================================================
ui.c

The numeric panel for the box tool.

Ernie Wright  30 Jun 01
====================================================================== */

#include "box.h"
#include <lwsurf.h>
#include <stdlib.h>
#include <string.h>


static LWXPanelFuncs *xpanf;
static LWSurfaceFuncs *surff;

static BoxData sbox = {
   1.0, 1.0, 1.0, 0.0, 0.0, 0.0, "Default", "MyUVs"
};


enum { ID_SIZE = 0x8001, ID_CENTER, ID_SURFLIST, ID_VMAPNAME };


/*
======================================================================
new_box()

Allocate and initialize an instance of the tool's data.
====================================================================== */

BoxData *new_box( void )
{
   BoxData *box;

   box = calloc( 1, sizeof( BoxData ));
   if ( box ) {
      memcpy( box, &sbox, sizeof( BoxData ));
      calc_handles( box );
   }
   return box;
}


/*
======================================================================
get_xpanf()

Get the globals needed for the interface.
====================================================================== */

int get_xpanf( GlobalFunc *global )
{
   xpanf = global( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT );
   surff = global( LWSURFACEFUNCS_GLOBAL, GFUSE_TRANSIENT );
   return (( xpanf != NULL ) && ( surff != NULL ));
}


/*
======================================================================
get_surfcount()
get_surfname()
get_surfindex()

Functions supporting the surface name list.
====================================================================== */

static int get_surfcount( void *userdata )
{
   LWSurfaceID surfid;
   int count = 0;

   surfid = surff->first();
   while ( surfid ) {
      ++count;
      surfid = surff->next( surfid );
   }

   if ( count == 0 ) count = 1;
   return count;
}


static const char *get_surfname( void *userdata, int index )
{
   LWSurfaceID surfid;
   int i = 0;

   surfid = surff->first();
   while ( surfid && i < index ) {
      ++i;
      surfid = surff->next( surfid );
   }

   if ( !surfid )
      return "Default";
   else
      return surff->name( surfid );
}


static int get_surfindex( const char *name )
{
   LWSurfaceID surfid;
   int i = 0;

   surfid = surff->first();
   while ( surfid ) {
      if ( !strcmp( name, surff->name( surfid )))
         return i;
      ++i;
      surfid = surff->next( surfid );
   }
   return 0;
}


/*
======================================================================
Get()

Send XPanels the value of a control.
====================================================================== */

static void *Get( BoxData *box, unsigned long vid )
{
   static int i;

   switch ( vid ) {
      case ID_SIZE:      return &box->size;
      case ID_CENTER:    return &box->center;
      case ID_SURFLIST:
         i = get_surfindex( box->surfname );
         return &i;
      case ID_VMAPNAME:  return &box->vmapname;
      default:           return NULL;
   }
}


/*
======================================================================
Set()

Store the value of a control.  We make a local copy of the data so
that the next time the tool is activated, we start where we left off.
====================================================================== */

static int Set( BoxData *box, unsigned long vid, void *value )
{
   const char *a;
   double *d;
   int i;

   switch ( vid )
   {
      case ID_SIZE:
         d = ( double * ) value;
         sbox.size[ 0 ] = box->size[ 0 ] = d[ 0 ];
         sbox.size[ 1 ] = box->size[ 1 ] = d[ 1 ];
         sbox.size[ 2 ] = box->size[ 2 ] = d[ 2 ];
         break;

      case ID_CENTER:
         d = ( double * ) value;
         sbox.center[ 0 ] = box->center[ 0 ] = d[ 0 ];
         sbox.center[ 1 ] = box->center[ 1 ] = d[ 1 ];
         sbox.center[ 2 ] = box->center[ 2 ] = d[ 2 ];
         break;

      case ID_SURFLIST:
         i = *(( int * ) value );
         a = get_surfname( NULL, i );
         strcpy( box->surfname, a );
         strcpy( sbox.surfname, a );
         break;

      case ID_VMAPNAME:
         a = ( const char * ) value;
         strcpy( box->vmapname, a );
         strcpy( sbox.vmapname, a );
         break;

      default:
         return LWXPRC_NONE;
   }

   box->update = LWT_TEST_UPDATE;
   box->dirty = 1;
   calc_handles( box );

   return LWXPRC_DRAW;
}


/*
======================================================================
Panel()

Create the numeric panel for the tool.
====================================================================== */

LWXPanelID Panel( BoxData *box )
{
   LWXPanelID panel;

   static LWXPanelControl ctl[] = {
      { ID_SIZE,     "Size",      "distance3"  },
      { ID_CENTER,   "Center",    "distance3"  },
      { ID_SURFLIST, "Surface",   "iPopChoice" },
      { ID_VMAPNAME, "VMap Name", "string"     },
      { 0 }
   };
   static LWXPanelDataDesc cdata[] = {
      { ID_SIZE,     "Size",      "distance3" },
      { ID_CENTER,   "Center",    "distance3" },
      { ID_SURFLIST, "Surface",   "integer"   },
      { ID_VMAPNAME, "VMap Name", "string"    },
      { 0 }
   };
   LWXPanelHint hint[] = {
      XpLABEL( 0, "Box Tutorial Part 4" ),
      XpPOPFUNCS( ID_SURFLIST, get_surfcount, get_surfname ),
      XpDIVADD( ID_SIZE ),
      XpDIVADD( ID_CENTER ),
      XpEND
   };

   panel = xpanf->create( LWXP_VIEW, ctl );
   if ( !panel ) return NULL;

   xpanf->describe( panel, cdata, Get, Set );
   xpanf->hint( panel, 0, hint );

   return panel;
}
