// GTK Viewer for Vrml 97 library
//
// Copyright (C) 1998 by Erik Andersen <andersee@debian.org>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA  02111-1307, USA.


#include "config.h"

#if HAVE_GTK

#include "ViewerGtk.h"
#include "gdk/gdkx.h"

#include <X11/keysym.h>		// XK_*

#define VIEW_ASPECT 1.3


ViewerGtk::ViewerGtk(VrmlScene *scene,
		     const char *,
		     GtkWidget *parent) :
  ViewerOpenGL(scene),
  d_timer(0),
  d_Stop(false),
  d_redrawNeeded(false),
  d_width(0),
  d_height(0),
  d_glarea(NULL)
{
  createWidget( parent );
}


ViewerGtk::~ViewerGtk()
{
}


void ViewerGtk::handleInput( ViewerOpenGL::EventInfo *e )
{
  gtk_gl_area_begingl( GTK_GL_AREA(d_glarea));
  input( e );
  gtk_gl_area_endgl( GTK_GL_AREA(d_glarea));
}


void ViewerGtk::handleRedraw()
{
  if ( d_glarea )
    {
      gint width, height;
      gdk_window_get_size( d_glarea->window, &width, &height);

      gtk_gl_area_begingl( GTK_GL_AREA(d_glarea));
      if (width != d_width || height != d_height)
	{
	  resize( width, height );
	  d_height = height;
	  d_width = width;
	}
      redraw();
      gtk_gl_area_endgl( GTK_GL_AREA(d_glarea));
    }

  d_redrawNeeded = false;
}


//  Do expose/redraw callback.

gint gtk_expose( GtkWidget *,
		 GdkEventExpose *event,
		 ViewerGtk* viewer )
{
  /* draw only on the last expose event */
  if ( event->count == 0 && viewer )
    viewer->handleRedraw();

  return TRUE;
}

//  Input callback

gint gtk_button_press(GtkWidget *,
		      GdkEventButton *event, 
		      ViewerGtk* viewer)
{
  ViewerOpenGL::EventInfo e;
  e.event = ViewerOpenGL::EVENT_MOUSE_CLICK;
  switch (event->button)
    {
    case Button1: e.what = 0; break;
    case Button2: e.what = 1; break;
    case Button3: e.what = 2; break;
    }
  e.x = (int)event->x;
  e.y = (int)event->y;
  
  viewer->handleInput( &e );

  return TRUE;
}


gint gtk_button_release(GtkWidget *,
			GdkEventButton *event, 
			ViewerGtk* viewer)
{
  ViewerOpenGL::EventInfo e;
  e.event = ViewerOpenGL::EVENT_MOUSE_RELEASE;
  switch (event->button)
    {
    case Button1: e.what = 0; break;
    case Button2: e.what = 1; break;
    case Button3: e.what = 2; break;
    }
  e.x = (int)event->x;
  e.y = (int)event->y;
  viewer->handleInput( &e );

  return TRUE;
}


gint gtk_motion_notify(GtkWidget *,
		       GdkEventMotion *event, 
		       ViewerGtk* viewer)
{
  ViewerOpenGL::EventInfo e;
  e.event = ViewerOpenGL::EVENT_MOUSE_DRAG;
  e.what = 0;
  if (event->state & Button1Mask)
    e.what = 0;
  else if (event->state & Button2Mask)
    e.what = 1;
  else if (event->state & Button3Mask)
    e.what = 2;
  else
    e.event = ViewerOpenGL::EVENT_MOUSE_MOVE;
  e.x = (int)event->x;
  e.y = (int)event->y;

  viewer->handleInput( &e );
  return TRUE;
}


gint gtk_key_press(GtkWidget *, GdkEventKey *event, ViewerGtk* viewer)
{
  ViewerOpenGL::EventInfo e;
  e.event = ViewerOpenGL::EVENT_KEY_DOWN;
  switch (event->keyval) {
      case XK_Home:	 e.what = ViewerOpenGL::KEY_HOME; break;
      case XK_Left:	 e.what = ViewerOpenGL::KEY_LEFT; break;
      case XK_Up:	 e.what = ViewerOpenGL::KEY_UP; break;
      case XK_Right:	 e.what = ViewerOpenGL::KEY_RIGHT; break;
      case XK_Down:	 e.what = ViewerOpenGL::KEY_DOWN; break;
      case XK_Page_Up:	 e.what = ViewerOpenGL::KEY_PAGE_UP; break;
      case XK_Page_Down: e.what = ViewerOpenGL::KEY_PAGE_DOWN; break;

      default:
	if (event->length <= 0) return TRUE; // Unhandled non-printable key
	e.what = event->string[0];
	break;
  }

  viewer->handleInput( &e );
  return TRUE;
}

gint gtk_destroy(GtkWidget *widget, ViewerGtk* viewer)
{
  ViewerOpenGL::EventInfo e;
  e.event = ViewerOpenGL::EVENT_KEY_DOWN;
  e.what = 'q';
  viewer->handleInput( &e );

  return TRUE;
}

void ViewerGtk::wsPostRedraw()
{
  if ( d_glarea && ! d_redrawNeeded )
    {
      d_redrawNeeded = true;
#if 0 
      // Apparently the gdk_event_put bypasses the X event queue,
      // so user input gets shut out during animations...
      GdkEvent event;
      event.expose.type = GDK_EXPOSE;
      event.expose.window = d_glarea->window;
      event.expose.send_event = FALSE;
      event.expose.area.x = 0;
      event.expose.area.y = 0;
      event.expose.area.width = d_width;
      event.expose.area.height = d_height;
      event.expose.count = 0;

      gdk_event_put( &event );
#else
      // Use XSendEvent instead...
      XEvent e;
      e.xexpose.type = Expose;
      e.xexpose.serial = 0;
      e.xexpose.send_event = False;
      e.xexpose.display = GDK_WINDOW_XDISPLAY( d_glarea->window );
      e.xexpose.window = GDK_WINDOW_XWINDOW( d_glarea->window );
      e.xexpose.x = e.xexpose.y = 0;
      e.xexpose.width = 0;
      e.xexpose.height = 0;
      e.xexpose.count = 0;
      XSendEvent( e.xexpose.display, e.xexpose.window,
		  False, 0L, &e );
#endif
    }
}



void ViewerGtk::wsSetCursor( CursorStyle c )
{
  if (d_glarea)
    {
      GdkCursor* cursor;
      switch(c) {
      case CURSOR_INHERIT:
	XDefineCursor( GDK_WINDOW_XDISPLAY( d_glarea->window ),
		       GDK_WINDOW_XWINDOW( d_glarea->window ), None );
	return;
      case CURSOR_INFO:
	cursor = gdk_cursor_new ( GDK_HAND1 );
	break;
      case CURSOR_CYCLE:
	cursor = gdk_cursor_new ( GDK_EXCHANGE );
	break;
      case CURSOR_UP_DOWN:
	cursor = gdk_cursor_new ( GDK_SB_V_DOUBLE_ARROW );
	break;
      case CURSOR_CROSSHAIR:
	cursor = gdk_cursor_new ( GDK_CROSSHAIR );
	break;
      default:
	cursor = gdk_cursor_new ( GDK_ARROW);
	break;
      }
      gdk_window_set_cursor (d_glarea->window, cursor );
      gdk_cursor_destroy( cursor);
    }
}

void ViewerGtk::wsSwapBuffers()
{
  GtkGLArea *area = GTK_GL_AREA(d_glarea);
  gtk_gl_area_swapbuffers(area);
}

// Timer callback calls the base class viewer update() method via
// the public timerUpdate method.

static gint timeout_callback(gpointer data)
{
  ViewerGtk *viewer = (ViewerGtk*)data;
  if (viewer) viewer->timerUpdate();
  return FALSE;
}

void ViewerGtk::timerUpdate()
{
  d_timer = 0;
  update( 0.0 );	// No gl calls should be made from update()
}

void ViewerGtk::SetStop( bool state)
{
  d_Stop=state;
  if (state==true)
    timerUpdate();
}

// 
void ViewerGtk::wsSetTimer( double t ) 
{
  if (! d_timer && ! d_Stop )
    {
      unsigned int millis = (unsigned int) (1000.0 * t);
      d_timer = gtk_timeout_add(millis, GtkFunction(timeout_callback), this);
    }
}


void ViewerGtk::createWidget( GtkWidget *window )
{
  /* create new OpenGL widget */
  d_glarea = gtk_gl_area_new_vargs(NULL, /* no sharing */
	    GDK_GL_ALPHA_SIZE,1,
	    GDK_GL_DOUBLEBUFFER,
	    GDK_GL_DEPTH_SIZE,1,
	    GDK_GL_RGBA,
	    GDK_GL_RED_SIZE,1,
	    //GDK_GL_X_VISUAL_TYPE_EXT, 
	    GDK_GL_NONE);  /* last argument must be GDK_GL_NONE */


  if (d_glarea == NULL || !GTK_IS_GL_AREA(d_glarea) ) {
    g_error("Can't create GtkGLArea widget\n");
    return;
  }

  /* set up events and signals for GTK OpenGL widget */
  gtk_widget_set_events(d_glarea,
                        GDK_EXPOSURE_MASK|
                        GDK_BUTTON_PRESS_MASK|
                        GDK_BUTTON_RELEASE_MASK|
                        GDK_KEY_PRESS_MASK|
                        GDK_POINTER_MOTION_MASK);

  GTK_WIDGET_SET_FLAGS(d_glarea, GTK_CAN_FOCUS);
  gtk_signal_connect (GTK_OBJECT(d_glarea), "expose_event",
                      GTK_SIGNAL_FUNC(gtk_expose), this);
  gtk_signal_connect (GTK_OBJECT(d_glarea), "key_press_event",
                      GTK_SIGNAL_FUNC(gtk_key_press), this);
  gtk_signal_connect (GTK_OBJECT(d_glarea), "motion_notify_event",
                      GTK_SIGNAL_FUNC(gtk_motion_notify), this);
  gtk_signal_connect (GTK_OBJECT(d_glarea), "button_press_event",
                      GTK_SIGNAL_FUNC(gtk_button_press), this);
  gtk_signal_connect (GTK_OBJECT(d_glarea), "button_release_event",
                      GTK_SIGNAL_FUNC(gtk_button_release), this);
  gtk_signal_connect (GTK_OBJECT(d_glarea), "destroy",
                      GTK_SIGNAL_FUNC (gtk_destroy), this);
  /* minimum size */
  gtk_widget_set_usize(d_glarea, 400, (gint)(400/VIEW_ASPECT) ); 


  /* destroy this window when exiting from gtk_main() */
  gtk_quit_add_destroy(1, GTK_OBJECT(d_glarea));

  /* put glarea into window and show it */
  gtk_container_add(GTK_CONTAINER(window), d_glarea);
  gtk_widget_show(d_glarea);
}


#endif // HAVE_GTK

