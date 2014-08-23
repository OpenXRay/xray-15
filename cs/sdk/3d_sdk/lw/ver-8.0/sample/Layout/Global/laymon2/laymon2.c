/*
======================================================================
laymon2.c

A Global class plug-in for Layout similar to the monitor global in
Modeler.

Ernie Wright  1 May 00
====================================================================== */

#include <lwserver.h>
#include <lwglobsrv.h>
#include <lwmonitor.h>
#include <lwpanel.h>
#include <stdlib.h>
#include <string.h>
#include "laymon2.h"

#define BARWIDTH  200
#define BARHEIGHT 12


typedef struct {
   unsigned int   current, total;
   int            closed;
   LWPanelID      panel;
   LWControl *    ctl;
   char           title[ 80 ];
   char           caption[ 80 ];
} LMData;


static LWPanelFuncs *panf = NULL;
static LWValue ival = { LWT_INTEGER };


/*
======================================================================
ctl_draw()

Callback that draws the progress bar.
====================================================================== */

static void ctl_draw( LWControl *ctl, LMData *data, DrMode mode )
{
   int x, y, w, h;

   x = CON_HOTX( ctl );
   y = CON_HOTY( ctl );
   w = CON_HOTW( ctl ) * data->current / data->total;
   h = CON_HOTH( ctl );

   panf->drawFuncs->drawBox( data->panel, LWP_INFO_DIS, x, y, w, h );
}


/*
======================================================================
pan_close()

Called when the panel is closed.
====================================================================== */

static void pan_close( LWPanelID panel, LMData *data )
{
   data->closed = 1;
}


/*
======================================================================
get_panel()

Create the monitor panel.
====================================================================== */

static int get_panel( LMData *data )
{
   LWPanelID panel;
   LWControl *ctl;
   LWPanControlDesc desc;
   int x, y;

   panel = PAN_CREATE( panf, data->title );
   if ( !panel ) return 0;

   ctl = CANVAS_CTL( panf, panel, data->caption, BARWIDTH, BARHEIGHT );
   if ( !ctl ) {
      PAN_KILL( panf, panel );
      return 0;
   }

   data->panel = panel;
   data->ctl = ctl;

   x = CON_X( ctl );
   y = CON_Y( ctl );
   MOVE_CON( ctl, x, y + 8 );

   panf->set( panel, PAN_USERDATA, data );
   panf->set( panel, PAN_USERCLOSE, pan_close );

   ival.ptr.ptr = ctl_draw;
   ctl->set( ctl, CTL_USERDRAW, &ival );
   ival.ptr.ptr = data;
   ctl->set( ctl, CTL_USERDATA, &ival );

   return 1;
}


/*
======================================================================
init()

Monitor init function.
====================================================================== */

static void init( LMData *data, unsigned int total )
{
   data->total   = total;
   data->current = 0;
   data->closed  = 0;

   panf->open( data->panel, PANF_CANCEL | PANF_ABORT );
}


/*
======================================================================
step()

Monitor step function.
====================================================================== */

static int step( LMData *data, unsigned int step )
{
   panf->handle( data->panel, 0 );
   if ( data->closed ) return 1;

   data->current += step;
   if ( data->current > data->total )
      data->current = data->total;

   RENDER_CON( data->ctl );

   return 0;
}


/*
======================================================================
done()

Monitor done function.
====================================================================== */

static void done( LMData *data )
{
   if ( !data->closed )
      panf->close( data->panel );
}


/*
======================================================================
lmcreate()

Create a monitor.
====================================================================== */

static LWMonitor *lmcreate( const char *title, const char *caption )
{
   LWMonitor *mon;
   LMData *data;

   mon = calloc( 1, sizeof( LWMonitor ));
   if ( !mon ) return NULL;

   data = calloc( 1, sizeof( LMData ));
   if ( !data ) {
      free( mon );
      return NULL;
   }

   if ( title )   strcpy( data->title, title );
   if ( caption ) strcpy( data->caption, caption );

   if ( !get_panel( data )) {
      free( data );
      free( mon );
      return NULL;
   }

   mon->data = data;
   mon->init = init;
   mon->step = step;
   mon->done = done;

   return mon;
}


/*
======================================================================
lmdestroy()

Free resources allocated by lmcreate().
====================================================================== */

static void lmdestroy( LWMonitor *mon )
{
   if ( mon ) {
      if ( mon->data ) {
         LMData *data = ( LMData * ) mon->data;
         if ( data->panel )
            PAN_KILL( panf, data->panel );
         free( data );
      }
      free( mon );
   }
}


/* this is what the global returns */

static LayoutMonitorFuncs lmfuncs = { lmcreate, lmdestroy };


/*
======================================================================
LayoutMon()

The activation function for the LayMon global.
====================================================================== */

XCALL_( int )
LayoutMon( long version, GlobalFunc *global, LWGlobalService *local,
   void *serverData)
{
   if ( version != LWGLOBALSERVICE_VERSION ) return AFUNC_BADVERSION;

   if ( !panf )
      panf = global( LWPANELFUNCS_GLOBAL, GFUSE_TRANSIENT );
   if ( !panf ) return AFUNC_BADGLOBAL;

   if ( !strcmp( local->id, LAYOUTMONITORFUNCS_GLOBAL ))
      local->data = &lmfuncs;
   else
      local->data = NULL;

   return AFUNC_OK;
}


/*
======================================================================
LayMonTest()

This is a generic that tests the LayMon global.  It's only compiled
into the .p file if _DEBUG is defined.
====================================================================== */

#ifdef _DEBUG

#include <lwgeneric.h>
#include <time.h>

XCALL_( int )
LayMonTest( long version, GlobalFunc *global, LWLayoutGeneric *local,
   void *serverData )
{
   LayoutMonitorFuncs *monf;
   LWMonitor *mon;
   clock_t t;
   int i;

   monf = global( LAYOUTMONITORFUNCS_GLOBAL, GFUSE_TRANSIENT );
   if ( !monf ) return AFUNC_BADGLOBAL;

   mon = monf->create( "Just Testing", "Progress Bar" );
   MON_INIT( mon, 10 );
   for ( i = 0; i < 10; i++ ) {
      t = clock();
      while (( t + CLOCKS_PER_SEC ) > clock() ) ;
      if ( MON_STEP( mon )) break;
   }
   MON_DONE( mon );
   monf->destroy( mon );

   return AFUNC_OK;
}

#endif


ServerRecord ServerDesc[] = {
   { LWGLOBALSERVICE_CLASS, LAYOUTMONITORFUNCS_GLOBAL, LayoutMon },
#ifdef _DEBUG
   { LWLAYOUTGENERIC_CLASS, "LayMonTest", LayMonTest },
#endif
   { NULL }
};
