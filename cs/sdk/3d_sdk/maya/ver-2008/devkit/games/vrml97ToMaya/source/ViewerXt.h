//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  ViewerXt.h
//  XToolkit version of OpenGL class for display of VRML models.
//

#include "config.h"
#ifndef _VIEWERXT_
#define _VIEWERXT_

#if HAVE_XT

#include "ViewerOpenGL.h"
#include <X11/Intrinsic.h>	// Widget
#include <GL/glx.h>		// GLXContext



class ViewerXt : public ViewerOpenGL {

public:

  // Create a named OpenGL drawing area widget
  ViewerXt(VrmlScene *, const char *, Widget parent, WidgetClass c = 0 );
  virtual ~ViewerXt();

  Widget widget() { return d_widget; }


  // Public so Xt callbacks can access
  void timerUpdate();
  void setContext(GLXContext glxcontext = 0);

  void flushEvents();

protected:

  // Window system specific methods

  virtual void wsPostRedraw();
  virtual void wsSetCursor( CursorStyle c);
  virtual void wsSwapBuffers();
  virtual void wsSetTimer( double );

private:

  // Actually create the colormap, widget, context, and manage the widget
  void createWidget( const char *name, Widget parent, WidgetClass c );

  GLXContext d_context;
  Widget d_widget;
  Colormap d_cmap;

  unsigned long d_timer;
};

#endif // HAVE_XT
#endif // _VIEWERXT_


