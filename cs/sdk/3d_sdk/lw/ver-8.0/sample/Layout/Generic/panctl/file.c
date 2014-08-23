/*
======================================================================
file.c

Test the LWPanels file selection controls.

Ernie Wright  28 Jun 00
====================================================================== */

#include <lwserver.h>
#include <lwpanel.h>
#include <stdio.h>


static char name[ 8 ][ 128 ] = {
   "file",
   "load",
   "save",
   "directory",
   "file",
   "load",
   "save",
   "directory"
};


int open_filepan( LWPanelFuncs *panf )
{
   LWPanControlDesc desc;
   LWValue
      ival = { LWT_INTEGER },
      sval = { LWT_STRING };
   LWPanelID panel;
   LWControl *ctl[ 8 ];
   int n, w, ok;

   if( !( panel = PAN_CREATE( panf, "File" )))
      return 0;

   ctl[ 0 ] = FILE_CTL( panf, panel, "File", 40 );
   ctl[ 1 ] = LOAD_CTL( panf, panel, "Load", 40 );
   ctl[ 2 ] = SAVE_CTL( panf, panel, "Save", 40 );
   ctl[ 3 ] = DIR_CTL( panf, panel, "Directory", 40 );
   ctl[ 4 ] = FILEBUTTON_CTL( panf, panel, "File Button", 40 );
   ctl[ 5 ] = LOADBUTTON_CTL( panf, panel, "Load Button", 40 );
   ctl[ 6 ] = SAVEBUTTON_CTL( panf, panel, "Save Button", 40 );
   ctl[ 7 ] = DIRBUTTON_CTL( panf, panel, "Directory Button", 40 );

   /* align */

   for ( n = 0; n < 8; n++ ) {
      w = CON_LW( ctl[ n ] );
      ival.intv.value = 100 - w;
      ctl[ n ]->set( ctl[ n ], CTL_X, &ival );
   }

   for ( n = 0; n < 8; n++ )
      SET_STR( ctl[ n ], name[ n ], sizeof( name[ n ] ));

   ok = panf->open( panel, PANF_BLOCKING | PANF_CANCEL );

   if ( ok ) {
      for ( n = 0; n < 8; n++ )
         GET_STR( ctl[ n ], name[ n ], sizeof( name[ n ] ));
   }

   PAN_KILL( panf, panel );

   return 1;
}
