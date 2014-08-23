//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  ViewerGlut.cpp
//  Display of VRML models using OpenGL/GLUT.
//

#include "config.h"
#include "ViewerGlut.h"

#include "System.h"

#if HAVE_GLUT
#include <GL/glut.h>

// Last mouse button pressed.
static int lastButton = 0;

// Map from GLUT window number to Viewer*
#define MAX_WINDOWS 10

// This could be a list<> if anyone cared...
static ViewerGlut* viewers[MAX_WINDOWS] = { 0 };

static ViewerGlut* getViewer(int w)
{
  return (w <= MAX_WINDOWS) ? viewers[w-1] : 0;
}


//  These callbacks locate the viewer object then call the 
//  viewer-specific versions.

//  Do redraw callback.

static void display() 
{
  ViewerGlut *viewer = getViewer( glutGetWindow() );
  if (viewer) viewer->redraw();
}

//  Keyboard button callback

static void keyboard(unsigned char key, int x, int y)
{
  ViewerGlut *viewer = getViewer( glutGetWindow() );

  ViewerOpenGL::EventInfo e = { ViewerOpenGL::EVENT_KEY_DOWN, key, x, y };
  if (viewer) viewer->input( &e );
}

//  More keyboard buttons callback

static void specialKey(int key, int x, int y)
{
  ViewerOpenGL::EventInfo e = { ViewerOpenGL::EVENT_KEY_DOWN, 0, x, y };

  switch (key)
    {
    case GLUT_KEY_HOME:  e.what = ViewerOpenGL::KEY_HOME; break;
    case GLUT_KEY_LEFT:  e.what = ViewerOpenGL::KEY_LEFT; break;
    case GLUT_KEY_UP:    e.what = ViewerOpenGL::KEY_UP; break;
    case GLUT_KEY_RIGHT: e.what = ViewerOpenGL::KEY_RIGHT; break;
    case GLUT_KEY_DOWN:  e.what = ViewerOpenGL::KEY_DOWN; break;
    case GLUT_KEY_PAGE_DOWN: e.what = ViewerOpenGL::KEY_PAGE_DOWN; break;
    case GLUT_KEY_PAGE_UP: e.what = ViewerOpenGL::KEY_PAGE_UP; break;
    default: return;
    }

  ViewerGlut *viewer = getViewer( glutGetWindow() );

  if (viewer) viewer->input( &e );
}

//  Mouse button callback

static void mouse(int button, int state, int x, int y)
{
  ViewerGlut *viewer = getViewer( glutGetWindow() );

  ViewerOpenGL::EventInfo e = { ViewerOpenGL::EVENT_MOUSE_CLICK, button, x, y };
  if (state == GLUT_UP) e.event = ViewerOpenGL::EVENT_MOUSE_RELEASE;
  lastButton = button;
  if (viewer) viewer->input( &e );
}

//  Mouse drag motion callback (button down)

static void motion(int x, int y)
{
  ViewerGlut *viewer = getViewer( glutGetWindow() );
  ViewerOpenGL::EventInfo e = { ViewerOpenGL::EVENT_MOUSE_DRAG,
				lastButton, x, y };
  if (viewer) viewer->input( &e );
}

//  Mouse motion callback

static void passiveMotion(int x, int y)
{
  ViewerGlut *viewer = getViewer( glutGetWindow() );
  ViewerOpenGL::EventInfo e = { ViewerOpenGL::EVENT_MOUSE_MOVE,
				lastButton, x, y };
  if (viewer) viewer->input( &e );
}

#if USE_STENCIL_SHAPE

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
#include "glutint.h"

static GC gc = 0;
static Pixmap shape_mask = 0;
static unsigned int shapew = 0, shapeh = 0, bwidth = 0;
static XImage *shape_image = 0;
static char *stencil_data = 0;

static void resizeshapemask (void)
{
  Display *display = __glutDisplay;
  Window window = __glutCurrentWindow->win;

  Window root;
  int x, y;
  unsigned int h, w, depth;
  XGCValues gcvalues;   

  /* Create a pixmap the same size as the window */
  XGetGeometry( display, window, &root, &x, &y,
		&w, &h, &bwidth, &depth );  

  if (! shape_mask || w != shapew || h != shapeh)
    {
      if (gc) XFreeGC(display, gc);
      if (shape_mask) XFreePixmap(display, shape_mask);
      if (stencil_data) delete [] stencil_data;

      shapew = w; shapeh = h;
      gcvalues.foreground = 1;
      gcvalues.background = 0;
      shape_mask = XCreatePixmap( display, root, shapew, shapeh, 1 );
      gc = XCreateGC(display, shape_mask, GCForeground | GCBackground, &gcvalues);
      stencil_data = new char[shapew*shapeh];
    }
}


static void doshape (void)
{
  Display *display = __glutDisplay;
  Window window = __glutCurrentWindow->win;

  if (! shape_mask) resizeshapemask();

  if (! shape_image)
    {
      shape_image = XGetImage(display, shape_mask, 0, 0, shapew, shapeh,
			      1, XYPixmap);
      if (! shape_image->data) {
	XDestroyImage(shape_image);
	shape_image = 0;
	return;
      }
    }

  glReadPixels(0, 0, shapew, shapeh, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE,
	       stencil_data);
  
  // yuck
  int i,j,k = 0;
  for (j=0; j<shapeh; ++j)
    {
      for (i=0; i<shapew; ++i)
	{
	  XPutPixel(shape_image, i, shapeh-j-1,
		    (unsigned long) stencil_data[k++]);
	}
    }

  XPutImage(display, shape_mask, gc, shape_image, 0, 0, 0, 0, shapew, shapeh);

  XShapeCombineMask(display,window,ShapeBounding, -bwidth, -bwidth, 
		    shape_mask, 
		    ShapeSet );
}

#endif

//  Window resize callback

static void reshape(int width, int height)
{
  ViewerGlut *viewer = getViewer( glutGetWindow() );
  if (viewer) viewer->resize(width, height);
#if USE_STENCIL_SHAPE
  resizeshapemask();
#endif
}

ViewerGlut::ViewerGlut(VrmlScene *scene) : ViewerOpenGL(scene)
{
#if USE_STENCIL_SHAPE
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STENCIL);
#else
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE );
#endif

#if 0
  if (!  glutGet((GLenum) GLUT_DISPLAY_MODE_POSSIBLE))
    exit(1);
#endif

  glutCreateWindow("VRML97 OpenGL Viewer");
  d_window = glutGetWindow();
  d_timerPending = false;

  // If d_window is not "small", convert viewers to a map...
  if (d_window > MAX_WINDOWS)
    {
      theSystem->error("ViewerGlut: too many windows.\n");
      return;
    }
  
  viewers[d_window-1] = this;

  // Register callbacks
  glutDisplayFunc( display);
  glutKeyboardFunc( keyboard);
  glutSpecialFunc( specialKey);
  glutMouseFunc( mouse);
  glutMotionFunc( motion);
  glutPassiveMotionFunc( passiveMotion);
  glutReshapeFunc( reshape);

}


ViewerGlut::~ViewerGlut()
{
  viewers[d_window-1] = 0;
  glutDestroyWindow(d_window);
}


void ViewerGlut::wsPostRedraw() { 
  glutPostRedisplay(); 
}

void ViewerGlut::wsSetCursor( CursorStyle c)
{
  static int cursorMap[] = { GLUT_CURSOR_INHERIT,
			     GLUT_CURSOR_INFO,
			     GLUT_CURSOR_CYCLE,
			     GLUT_CURSOR_UP_DOWN,
			     GLUT_CURSOR_CROSSHAIR
  };
			     
  glutSetCursor( cursorMap[c] );
}

void ViewerGlut::wsSwapBuffers() { 
#if USE_STENCIL_SHAPE
  doshape();
#endif
  glutSwapBuffers(); 
}


// Timer callback calls the viewer update() method.
static void timer(int)
{
  ViewerGlut *viewer = getViewer( glutGetWindow() );
  if (viewer) viewer->timerUpdate();
}

void ViewerGlut::timerUpdate()
{
  d_timerPending = false;
  update( 0.0 );
}

// I think glut timers are one-shot deals.
void ViewerGlut::wsSetTimer( double t ) 
{
  if (! d_timerPending)
    {
      unsigned int millis = (unsigned int) (1000.0 * t);
      glutTimerFunc( millis, timer, 1);
      d_timerPending = true;
    }
}


#endif // HAVE_GLUT
