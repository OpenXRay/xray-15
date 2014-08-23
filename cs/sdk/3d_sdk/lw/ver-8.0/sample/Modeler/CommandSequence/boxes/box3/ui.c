/*
======================================================================
ui.c

The graphical user interface for the box plug-in.

Ernie Wright  10 Jun 01
====================================================================== */

#include <lwxpanel.h>
#include <lwsurf.h>
#include <stdlib.h>
#include <string.h>


static char **surflist;


/*
======================================================================
free_surflist()

Free memory allocated by init_surflist().
====================================================================== */

void free_surflist( void )
{
   int i;

   if ( surflist ) {
      for ( i = 0; surflist[ i ]; i++ )
         free( surflist[ i ] );
      free( surflist );
   }
}


/*
======================================================================
init_surflist()

Allocate and initialize an array of strings containing surface names.
get_user() passes this array to XPanels to initialize the menu of
surface names on our panel.  The array is NULL-terminated, meaning the
last element of the array is NULL.

If no surfaces have been created yet in the current Modeler session,
the array contains the single string "Default".

Returns the number of names in the array, which is 0 if an error
occurred.
====================================================================== */

int init_surflist( LWSurfaceFuncs *surff )
{
   LWSurfaceID surfid;
   const char *name;
   int i, count = 0;

   /* count the surfaces */

   surfid = surff->first();
   while ( surfid ) {
      ++count;
      surfid = surff->next( surfid );
   }

   /* if none, create a default list */

   if ( !count ) {
      surflist = calloc( 2, sizeof( char * ));
      surflist[ 0 ] = malloc( 8 );
      strcpy( surflist[ 0 ], "Default" );
      return 1;
   }

   /* allocate an array of strings */

   surflist = calloc( count + 1, sizeof( char * ));
   if ( !surflist ) return 0;

   /* for each surface, allocate a string and fill in the name */

   surfid = surff->first();
   for ( i = 0; i < count; i++ ) {
      name = surff->name( surfid );
      if ( !name ) {
         free_surflist();
         return 0;
      }
      surflist[ i ] = malloc( strlen( name ) + 1 );
      if ( !surflist[ i ] ) {
         free_surflist();
         return 0;
      }
      strcpy( surflist[ i ], name );
      surfid = surff->next( surfid );
   }

   /* return the surface count */

   return count;
}


/*
======================================================================
get_user()

Display a modal user interface.
====================================================================== */

int get_user( LWXPanelFuncs *xpanf, double *size, double *center,
   char *surfname, char *vmapname )
{
   LWXPanelID panel;
   int ok = 0;

   enum { ID_SIZE = 0x8001, ID_CENTER, ID_SURFLIST, ID_VMAPNAME };
   LWXPanelControl ctl[] = {
      { ID_SIZE,     "Size",      "distance3"  },
      { ID_CENTER,   "Center",    "distance3"  },
      { ID_SURFLIST, "Surface",   "iPopChoice" },
      { ID_VMAPNAME, "VMap Name", "string"     },
      { 0 }
   };
   LWXPanelDataDesc cdata[] = {
      { ID_SIZE,     "Size",      "distance3" },
      { ID_CENTER,   "Center",    "distance3" },
      { ID_SURFLIST, "Surface",   "integer"   },
      { ID_VMAPNAME, "VMap Name", "string"    },
      { 0 }
   };
   LWXPanelHint hint[] = {
      XpLABEL( 0, "Box Tutorial Part 3" ),
      XpDIVADD( ID_SIZE ),
      XpDIVADD( ID_CENTER ),
      XpSTRLIST( ID_SURFLIST, surflist ),
      XpEND
   };

   panel = xpanf->create( LWXP_FORM, ctl );
   if ( !panel ) return 0;

   xpanf->describe( panel, cdata, NULL, NULL );
   xpanf->hint( panel, 0, hint );
   xpanf->formSet( panel, ID_SIZE, size );
   xpanf->formSet( panel, ID_CENTER, center );
   xpanf->formSet( panel, ID_SURFLIST, 0 );
   xpanf->formSet( panel, ID_VMAPNAME, vmapname );

   ok = xpanf->post( panel );

   if ( ok ) {
      double *d;
      int *i;
      char *a;

      d = xpanf->formGet( panel, ID_SIZE );
      size[ 0 ] = d[ 0 ];
      size[ 1 ] = d[ 1 ];
      size[ 2 ] = d[ 2 ];

      d = xpanf->formGet( panel, ID_CENTER );
      center[ 0 ] = d[ 0 ];
      center[ 1 ] = d[ 1 ];
      center[ 2 ] = d[ 2 ];

      i = xpanf->formGet( panel, ID_SURFLIST );
      strcpy( surfname, surflist[ *i ] );

      a = xpanf->formGet( panel, ID_VMAPNAME );
      strcpy( vmapname, a );
   }

   xpanf->destroy( panel );
   return ok;
}
