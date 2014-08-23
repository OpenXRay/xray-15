/*
======================================================================
choice.c

Test the LWPanels choice controls.

Ernie Wright  28 Jun 00
====================================================================== */

#include <lwserver.h>
#include <lwrender.h>
#include <lwenvel.h>
#include <lwpanel.h>
#include <stdio.h>


/* the choices for most of the controls */

static char *choices[] = {
   "First",
   "Second",
   "Third",
   NULL
};


/*
======================================================================
count()

Callback used by CUSTPOPUP, LISTBOX, MULTILISTBOX.
====================================================================== */

static int count( void *data )
{
   return 3;
}


/*
======================================================================
name()

Callback used by CUSTPOPUP and LISTBOX.  Returns the list item, given
a 0-based index.
====================================================================== */

static char *name( void *data, int index )
{
   if ( index >= 0 && index < 3 )
      return choices[ index ];
   else
      return NULL;
}


/*
======================================================================
mname()

Callback used by MULTILISTBOX.  Returns the list item, given a 0-based
index and column.
====================================================================== */

static char *mname( void *data, int index, int column )
{
   static char buf[ 32 ];
   static char *col2[] = {
      "the first choice",
      "the second one",
      "the last one",
      NULL
   };

   if ( index == -1 ) {
      sprintf( buf, "Column %d", column );
      return buf;
   }

   if ( index >= 0 && index < 3 )
      if ( column == 0 )
         return choices[ index ];
      else
         return col2[ index ];
}


/*
======================================================================
colwidth()

Callback used by MULTILISTBOX.  Returns the width of each column, in
pixels.  The number of columns is set to the first index we return 0
for.
====================================================================== */

static int colwidth( void *data, int index )
{
   return index < 2 ? 100 : 0;
}


static int tab = 0, hc = 0, vc = 0, pop = 0, cpop = 0, dpop = 0, list = 0, mlist = 0;
static LWItemID id = 0, pid = 0;
static LWChannelID chan = NULL;


/*
======================================================================
open_choicepan()

Open the panel.
====================================================================== */

int open_choicepan( LWPanelFuncs *panf )
{
   LWPanControlDesc desc;
   LWValue
      ival = { LWT_INTEGER },
      sval = { LWT_STRING };
   LWPanelID panel;
   LWControl *ctl[ 11 ] = { NULL };
   LWChannelInfo *chinfo;
   LWItemInfo *iteminfo;
   int n, w, ok;

   chinfo = panf->globalFun( LWCHANNELINFO_GLOBAL, GFUSE_TRANSIENT );
   iteminfo = panf->globalFun( LWITEMINFO_GLOBAL, GFUSE_TRANSIENT );
   if ( !chinfo || !iteminfo ) return 0;

   if ( !id )
      id = pid = iteminfo->first( LWI_CAMERA, LWITEM_NULL );

   if ( !chan )
      chan = chinfo->nextChannel( iteminfo->chanGroup( id ), NULL );

   if( !( panel = PAN_CREATE( panf, "Choice" )))
      return 0;

   ctl[ 0 ] = TABCHOICE_CTL( panf, panel, "Tab Choice", choices );
   ctl[ 1 ] = HCHOICE_CTL( panf, panel, "Horizontal Choice", choices );
   ctl[ 2 ] = VCHOICE_CTL( panf, panel, "Vertical Choice", choices );
   ctl[ 3 ] = POPUP_CTL( panf, panel, "Popup", choices );
   ctl[ 4 ] = CUSTPOPUP_CTL( panf, panel, "Custom Popup", 100, name, count );
   ctl[ 5 ] = POPDOWN_CTL( panf, panel, "Popdown", choices );
   ctl[ 6 ] = LISTBOX_CTL( panf, panel, "List Box", 100, 5, name, count );
   ctl[ 7 ] = MULTILIST_CTL( panf, panel, "Multi List Box", 200, 3, mname, count, colwidth );
   ctl[ 8 ] = ITEM_CTL( panf, panel, "Item", panf->globalFun, LWI_CAMERA );
   ctl[ 9 ] = PIKITEM_CTL( panf, panel, "Pikitem", panf->globalFun, LWI_CAMERA, 100 );
   ctl[ 10 ] = CHANNEL_CTL( panf, panel, "Channel", 200, 140 );

   /* align */

   for ( n = 0; n < 11; n++ ) {
      if ( ctl[ n ] ) {
         w = CON_LW( ctl[ n ] );
         ival.intv.value = 100 - w;
         ctl[ n ]->set( ctl[ n ], CTL_X, &ival );
      }
   }

   SET_INT( ctl[ 0 ], tab );
   SET_INT( ctl[ 1 ], hc );
   SET_INT( ctl[ 2 ], vc );
   SET_INT( ctl[ 3 ], pop );
   SET_INT( ctl[ 4 ], cpop );
   SET_INT( ctl[ 5 ], dpop );
   SET_INT( ctl[ 6 ], list );
   SET_INT( ctl[ 7 ], mlist );
   SET_INT( ctl[ 8 ], ( int ) id );
   SET_INT( ctl[ 9 ], ( int ) pid );
   SET_INT( ctl[ 10 ], ( int ) chan );

   /* need these so that Panels can do some formatting */

   CON_SETEVENT( ctl[ 6 ], NULL, NULL );
   CON_SETEVENT( ctl[ 7 ], NULL, NULL );

   ok = panf->open( panel, PANF_BLOCKING | PANF_CANCEL );

   if ( ok ) {
      GET_INT( ctl[ 1 ], hc );
      GET_INT( ctl[ 2 ], vc );
      GET_INT( ctl[ 3 ], pop );
      GET_INT( ctl[ 4 ], cpop );
      GET_INT( ctl[ 5 ], dpop );
      GET_INT( ctl[ 6 ], list );
      GET_INT( ctl[ 7 ], mlist );

      ctl[ 8 ]->get( ctl[ 8 ], CTL_VALUE, &ival );
      id = ( LWItemID ) ival.intv.value;

      ctl[ 9 ]->get( ctl[ 9 ], CTL_VALUE, &ival );
      pid = ( LWItemID ) ival.intv.value;

      ctl[ 10 ]->get( ctl[ 10 ], CTL_VALUE, &ival );
      chan = ( LWChannelID ) ival.ptr.ptr;
   }

   PAN_KILL( panf, panel );

   return 1;
}
