//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  ViewerXt.cpp
//  Display of VRML models using OpenGL/XToolkit.
//

#include "config.h"
#if HAVE_XT

#include "ViewerXt.h"

#include "GLwDrawA.h"

#include <X11/StringDefs.h>	// XtNcolormap
#include <X11/Xatom.h>		// XA_RGB_DEFAULT
#include <X11/keysym.h>		// XK_*
#include <X11/cursorfont.h>	// XC_*

#include <GL/glu.h>		// gluOrtho2D

#include "System.h"



ViewerXt::ViewerXt(VrmlScene *scene,
		   const char *name,
		   Widget parent,
		   WidgetClass c ) :
  ViewerOpenGL(scene),
  d_context(0),
  d_widget(0),
  d_cmap(0),
  d_timer(0)
{
  if (c == 0) c = glwDrawingAreaWidgetClass;
  createWidget( name, parent, c );
}


ViewerXt::~ViewerXt()
{
  if (d_widget)
    {
      if (d_context) glXDestroyContext(XtDisplay(d_widget), d_context);
      //if (d_cmap) XFreeColormap(XtDisplay(d_widget), d_cmap);
      //XtDestroyWidget(d_widget);
      d_widget = 0;
    }
}


// Graphics initialization callback.

static void ginit(Widget w, XtPointer clientData, XtPointer)
{
  XVisualInfo *visinfo;
  Dimension width, height;

  // Create rendering context (no dlist sharing, direct render true).
  XtVaGetValues( w,
		 GLwNvisualInfo, &visinfo,
		 XtNwidth, &width,
		 XtNheight, &height,
		 0);
  ViewerXt *v = (ViewerXt*)clientData;
  if (v)
    {
      GLXContext c = glXCreateContext(XtDisplay(w), visinfo, 0, True);
      if (! c)
	theSystem->error("ViewerXt: couldn't create GLXContext.");
      else
	{
	  v->setContext( c );
	  v->resize( width, height );
	}
    }
}


//  Do redraw callback.

static void expose(Widget, XtPointer clientData, XtPointer)
{
  ViewerXt *viewer = (ViewerXt*)clientData;
  if (viewer)
    {
      viewer->flushEvents();
      viewer->redraw();
    }
}

//  Input callback

static void xinput(Widget, XtPointer clientData, XtPointer callData)
{
  ViewerXt *viewer = (ViewerXt*)clientData;

  if (viewer)
    {
      GLwDrawingAreaCallbackStruct *x;
      x = (GLwDrawingAreaCallbackStruct *)callData;
      ViewerOpenGL::EventInfo e;
      char cbuf[32];
      KeySym keysym;

      switch (x->event->type)
	{
	case KeyPress:
	  {
	    int nc = XLookupString((XKeyEvent*)(x->event),
				   cbuf, sizeof(cbuf)-1,
				   &keysym, 0);
	    e.event = ViewerOpenGL::EVENT_KEY_DOWN;
	    switch (keysym)
	      {
	      case XK_Home:	 e.what = ViewerOpenGL::KEY_HOME; break;
	      case XK_Left:	 e.what = ViewerOpenGL::KEY_LEFT; break;
	      case XK_Up:	 e.what = ViewerOpenGL::KEY_UP; break;
	      case XK_Right:	 e.what = ViewerOpenGL::KEY_RIGHT; break;
	      case XK_Down:	 e.what = ViewerOpenGL::KEY_DOWN; break;
	      case XK_Page_Up:	 e.what = ViewerOpenGL::KEY_PAGE_UP; break;
	      case XK_Page_Down: e.what = ViewerOpenGL::KEY_PAGE_DOWN; break;

	      default:
		if (nc <= 0) return; // Unhandled non-printable key
		e.what = cbuf[0];
		break;
	      }
	    e.x = ((XKeyEvent*)(x->event))->x;
	    e.y = ((XKeyEvent*)(x->event))->y;
	    break;
	  }

	case ButtonPress:
	  e.event = ViewerOpenGL::EVENT_MOUSE_CLICK;
	  switch (((XButtonEvent*)(x->event))->button)
	    {
	    default:
	    case Button1: e.what = 0; break;
	    case Button2: e.what = 1; break;
	    case Button3: e.what = 2; break;
	    }
	  e.x = ((XButtonEvent*)(x->event))->x;
	  e.y = ((XButtonEvent*)(x->event))->y;
	  break;

	case ButtonRelease:
	  e.event = ViewerOpenGL::EVENT_MOUSE_RELEASE;
	  switch (((XButtonEvent*)(x->event))->button)
	    {
	    default:
	    case Button1: e.what = 0; break;
	    case Button2: e.what = 1; break;
	    case Button3: e.what = 2; break;
	    }
	  e.x = ((XButtonEvent*)(x->event))->x;
	  e.y = ((XButtonEvent*)(x->event))->y;
	  break;

	case MotionNotify:
	  e.event = ViewerOpenGL::EVENT_MOUSE_DRAG;
	  e.what = 0;
	  if (((XMotionEvent*)(x->event))->state & Button1Mask)
	    e.what = 0;
	  else if (((XMotionEvent*)(x->event))->state & Button2Mask)
	    e.what = 1;
	  else if (((XMotionEvent*)(x->event))->state & Button3Mask)
	    e.what = 2;
	  else
	    e.event = ViewerOpenGL::EVENT_MOUSE_MOVE;
	  e.x = ((XMotionEvent*)(x->event))->x;
	  e.y = ((XMotionEvent*)(x->event))->y;
	  break;

	default:
	  return;
	}

      viewer->input( &e );
    }
}

//  Input event handler

static void xinputEvent(Widget w,
			XtPointer clientData,
			XEvent *event,
			Boolean *propagate)
{
  GLwDrawingAreaCallbackStruct x;
  x.event = event;
  xinput(w, clientData, (XtPointer) &x);
  *propagate = False;
}


//  Window resize callback

static void xresize(Widget, XtPointer clientData, XtPointer callData)
{
  ViewerXt *viewer = (ViewerXt*)clientData;
  if (viewer)
    {
      GLwDrawingAreaCallbackStruct *x;
      x = (GLwDrawingAreaCallbackStruct *)callData;
      viewer->setContext();
      glXWaitX();		// Wait for the resize to happen
      viewer->resize(x->width, x->height);
    }
}

// Not declared on Solaris5.5.1
extern "C"
int XmuLookupStandardColormap( Display*, int, VisualID, int, int, int, int);

// From MJK, OpenGL Programming for X, p50
static Colormap getcmap(XVisualInfo *vi, Display *dpy) 
{
  Status status;
  XStandardColormap *standardCmaps;
  Colormap cmap;
  int i, numCmaps;
  Atom *rgbmap, atoms[3] = { XA_RGB_DEFAULT_MAP, XA_RGB_COLOR_MAP, 0 };

  //if (vi->c_class != TrueColor)
  //  return 0;

  for (rgbmap=&atoms[0]; *rgbmap; ++rgbmap)
    {
      status = XmuLookupStandardColormap(dpy, vi->screen, vi->visualid,
					 vi->depth, *rgbmap,
					 False, True);
      if (status == 1)
	{
	  status = XGetRGBColormaps(dpy, RootWindow(dpy, vi->screen),
				    &standardCmaps, &numCmaps, *rgbmap);
	  if (status == 1)
	    {
	      for (i=0; i<numCmaps; ++i)
		if (standardCmaps[i].visualid == vi->visualid)
		  {
		    cmap = standardCmaps[i].colormap;
		    XFree(standardCmaps);
		    return cmap;
		  }

	      XFree(standardCmaps);
	    }
	}
    }

  theSystem->inform("Creating colormap");
  cmap = XCreateColormap(dpy, RootWindow(dpy, vi->screen),
			 vi->visual, AllocNone);
  return cmap;
}


void ViewerXt::createWidget( const char *name,
			     Widget parent,
			     WidgetClass c )
{
  Display *dpy = XtDisplay(parent);
  int screen = XScreenNumberOfScreen(XtScreen(parent));

  // Request a double-buffered RGBA buffer with Z-buffer:
  int vType[] = { GLX_ALPHA_SIZE, 1,
		  GLX_DOUBLEBUFFER,
		  GLX_DEPTH_SIZE, 1,
		  GLX_RGBA,
		  GLX_RED_SIZE, 1,
		  //GLX_X_VISUAL_TYPE_EXT, GLX_TRUE_COLOR_EXT,
		  (int)None };

  XVisualInfo *visinfo = 0;
  for (int v=0; v<3; ++v)
    if ((visinfo = glXChooseVisual(dpy, screen, &vType[v])) != 0)
      break;

  // What now? 
  if (! visinfo)
    {
      theSystem->error("ViewerXt: glXChooseVisual failed.\n");
      return;
    }

  d_cmap = getcmap(visinfo, dpy);
  theSystem->debug("ViewerXt: visual ID 0x%x, %d bits, class %d, cmap 0x%x.\n",
		   visinfo->visualid,
		   visinfo->depth,
		   visinfo->c_class,
		   d_cmap);

  // If you are having trouble with colormap flashing on an 8 bit
  // display, try restarting your X server (eg, log out and back in
  // in an xdm environment) and running "xstdcmap -default" BEFORE
  // running anything else. In fact, you might put that command in
  // your X startup script.
  if (! d_cmap)
    {
      theSystem->warn("ViewerXt: couldn't get a TrueColor colormap (class %d)\n",
		      visinfo->c_class);
      d_widget = XtVaCreateWidget(name, c, parent,
				  GLwNvisualInfo, visinfo,
				  GLwNallocateBackground, False,
				  GLwNallocateOtherColors, False,
				  GLwNinstallBackground, False,
				  0);
    }
  else
    d_widget = XtVaCreateWidget(name, c, parent,
				GLwNvisualInfo, visinfo,
				XtNcolormap, d_cmap,
				GLwNallocateBackground, False,
				GLwNallocateOtherColors, False,
				GLwNinstallBackground, False,
				0);

  if (! d_widget)
    {
      theSystem->error("ViewerXt: couldn't create GL widget.\n");
      return;
    }

  // Register callbacks
  XtAddCallback(d_widget, GLwNginitCallback, ginit, this);
  XtAddCallback(d_widget, GLwNexposeCallback, expose, this);
  XtAddCallback(d_widget, GLwNresizeCallback, xresize, this);
  XtAddCallback(d_widget, GLwNinputCallback, xinput, this);

  // GLwNinputCallback doesn't seem to get pointer motion events
  XtAddEventHandler(d_widget, PointerMotionMask, True, xinputEvent, this);
  // If you are not getting keyboard events, check focus policy (see xmlookat)

  XtManageChild( d_widget );
}



void ViewerXt::setContext(GLXContext glxcontext)
{
  if (glxcontext)
    d_context = glxcontext;
  if (d_context)
    glXMakeCurrent(XtDisplay(d_widget), XtWindow(d_widget), d_context);
}


// 
void ViewerXt::wsPostRedraw()
{
  //if (d_context) redraw();
  if (d_widget)
    {
      XEvent e;
      e.xexpose.type = Expose;
      e.xexpose.serial = 0;
      e.xexpose.send_event = True;
      e.xexpose.display = XtDisplay(d_widget);
      e.xexpose.window = XtWindow(d_widget);
      e.xexpose.x = e.xexpose.y = 0;
      e.xexpose.width = e.xexpose.height = 1;
      e.xexpose.count = 0;
      XSendEvent(XtDisplay(d_widget), XtWindow(d_widget),
		 False, 0L, &e);
    }
}

void ViewerXt::flushEvents()
{
  if (d_widget)
    {
      XEvent e;
      while (XCheckTypedWindowEvent(XtDisplay(d_widget),
				    XtWindow(d_widget),
				    Expose, &e))
	;
    }
}

typedef struct _CursorTable {
  int glyph;
  Cursor cursor;
} CursorTable;

void ViewerXt::wsSetCursor( CursorStyle c )
{
  static CursorTable cursorMap[] = {
    { (int)None, None },
    { XC_hand1, None },
    { XC_exchange, None },
    { XC_sb_v_double_arrow, None },
    { XC_crosshair, None }
  };

  if (c == CURSOR_INHERIT)
    XDefineCursor( XtDisplay(d_widget), XtWindow(d_widget), None );
  else
    {
      if (cursorMap[c].cursor == None)
	cursorMap[c].cursor = XCreateFontCursor(XtDisplay(d_widget),
						cursorMap[c].glyph);
      XDefineCursor( XtDisplay(d_widget), XtWindow(d_widget),
		     cursorMap[c].cursor );
    }

  XFlush( XtDisplay(d_widget) );
}

void ViewerXt::wsSwapBuffers()
{
  glXSwapBuffers(XtDisplay(d_widget),XtWindow(d_widget));
}

// Timer callback calls the viewer update() method.
static void timer(XtPointer clientData, XtIntervalId *)
{
  ViewerXt *viewer = (ViewerXt*)clientData;
  if (viewer) viewer->timerUpdate();
}

void ViewerXt::timerUpdate()
{
  d_timer = 0;
  update( 0.0 );
}

// 
void ViewerXt::wsSetTimer( double t ) 
{
  if (! d_timer)
    {
      unsigned int millis = (unsigned int) (1000.0 * t);
      d_timer = (unsigned long)
	XtAppAddTimeOut( XtWidgetToApplicationContext(d_widget),
			 millis, timer, this );
    }
}


#endif // HAVE_XT
