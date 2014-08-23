/*
======================================================================
draw.c

Test the LWPanels draw controls.

Ernie Wright  28 Jun 00
====================================================================== */

#include <lwserver.h>
#include <lwpanel.h>
#include <stdio.h>

/* Windows-specific.  Change these for other platforms. */

#include <windows.h>
#include <gl/gl.h>


/*
======================================================================
draw()

Draw callback.  None of the drawing area controls render their labels,
so we draw the labels here.  The label string was set as the control's
userdata when the control was created.
====================================================================== */

static void draw( LWControl *ctl, char *label, DrMode mode )
{
   LWPanelFuncs *panf;
   LWPanelID panel;
   LWValue ival = { LWT_INTEGER };
   int x, y;

   x = CON_HOTX( ctl );
   y = CON_HOTY( ctl );

   panel = ( LWPanelID ) CON_PAN( ctl );
   panf = ( LWPanelFuncs * ) CON_PANFUN( ctl );

   panf->drawFuncs->drawText( panel, label, COLOR_BLACK, x + 2, y + 2 );
}


/*
======================================================================
gldraw()

OPENGL event/draw callback.  This uses OpenGL functions to draw a
label for the control.
====================================================================== */

static void gldraw( LWControl *ctl, void *data, DrMode mode )
{
   glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
   glClear( GL_COLOR_BUFFER_BIT );
   glColor3f( 1.0f, 1.0f, 1.0f );

   glBegin( GL_LINE_STRIP );
   glVertex2f( 0.42f, 0.75f );
   glVertex2f( 0.12f, 0.75f );
   glVertex2f( 0.12f, 0.25f );
   glVertex2f( 0.42f, 0.25f );
   glVertex2f( 0.42f, 0.50f );
   glVertex2f( 0.25f, 0.50f );
   glEnd();

   glBegin( GL_LINE_STRIP );
   glVertex2f( 0.58f, 0.75f );
   glVertex2f( 0.58f, 0.25f );
   glVertex2f( 0.88f, 0.25f );
   glEnd();

   glFlush();
}


/*
======================================================================
event()

Event callback for the AREA and DRAGAREA controls.  We draw the mouse
coordinates.
====================================================================== */

static void event( LWControl *ctl, char *label )
{
   LWPanelFuncs *panf;
   const LWDisplayMetrics *dmet;
   LWPanelID panel;
   LWValue
      ival = { LWT_INTEGER },
      ivecval = { LWT_VINT };
   int x, y, w, mx, my, md[ 3 ];
   char a[ 32 ];


   /* panel, panf, dmet */

   panel = ( LWPanelID ) CON_PAN( ctl );
   panf = ( LWPanelFuncs * ) CON_PANFUN( ctl );
   dmet = panf->drawFuncs->dispMetrics();

   /* control dimensions */

   x = CON_HOTX( ctl );
   y = CON_HOTY( ctl );
   w = CON_HOTW( ctl );

   /* mouse click coordinates */

   ctl->get( ctl, CTL_MOUSEX, &ival );   mx = ival.intv.value;
   ctl->get( ctl, CTL_MOUSEY, &ival );   my = ival.intv.value;

   sprintf( a, "%d %d", mx, my );
   panf->drawFuncs->drawBox( panel, COLOR_WHITE, x, y + 20, w,
      dmet->textHeight );
   panf->drawFuncs->drawText( panel, a, COLOR_BLACK, x + 2, y + 20 );

   /* mouse drag coordinates (drag area control only) */

   if ( label[ 0 ] == 'D' ) {
      GETV_IVEC( ctl, md );
      sprintf( a, "%d %d %d", md[ 0 ], md[ 1 ], md[ 2 ] );
      panf->drawFuncs->drawBox( panel, COLOR_LT_YELLOW, x, y + 40, w,
         dmet->textHeight );
      panf->drawFuncs->drawText( panel, a, COLOR_BLACK, x + 2, y + 40 );
   }
}


static char *text[] = {
   "...the first line of text",
   "...the second line",
   NULL
};


/*
======================================================================
open_drawpan()

Open the panel.
====================================================================== */

int open_drawpan( LWPanelFuncs *panf )
{
   LWPanControlDesc desc;
   LWValue ival = { LWT_INTEGER };
   LWPanelID panel;
   LWControl *ctl[ 6 ];
   int n, w;

   if( !( panel = PAN_CREATE( panf, "Draw" )))
      return 0;

   ctl[ 0 ] = TEXT_CTL( panf, panel, "Text", text );
   ctl[ 1 ] = AREA_CTL( panf, panel, "", 80, 60 );
   ctl[ 2 ] = DRAGAREA_CTL( panf, panel, "", 80, 60 );
   ctl[ 3 ] = CANVAS_CTL( panf, panel, "", 80, 60 );
   ctl[ 4 ] = BORDER_CTL( panf, panel, "", 80, 60 );
   ctl[ 5 ] = OPENGL_CTL( panf, panel, "", 80, 60 );

   CON_SETEVENT( ctl[ 1 ], event, "Area" );
   CON_SETEVENT( ctl[ 2 ], event, "Drag Area" );
   CON_SETEVENT( ctl[ 3 ], NULL, "Canvas" );
   CON_SETEVENT( ctl[ 4 ], NULL, "Border" );

   CON_SETEVENT( ctl[ 5 ], gldraw, NULL );

   for ( n = 1; n < 5; n++ ) {
      ival.ptr.ptr = draw;
      ctl[ n ]->set( ctl[ n ], CTL_USERDRAW, &ival );
   }

   /* align */

   for ( n = 0; n < 6; n++ ) {
      w = CON_LW( ctl[ n ] );
      ival.intv.value = 100 - w;
      ctl[ n ]->set( ctl[ n ], CTL_X, &ival );
   }

   panf->open( panel, PANF_BLOCKING );

   PAN_KILL( panf, panel );

   return 1;
}
