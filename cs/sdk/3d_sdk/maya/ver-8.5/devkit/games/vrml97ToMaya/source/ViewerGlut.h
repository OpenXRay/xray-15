//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  ViewerGlut.h
//  GLUT version of OpenGL class for display of VRML models.
//

#ifndef _VIEWERGLUT_
#define _VIEWERGLUT_

#include "config.h"
#include "ViewerOpenGL.h"


class ViewerGlut : public ViewerOpenGL {

public:

  ViewerGlut(VrmlScene *);
  virtual ~ViewerGlut();

  // Public so glut callbacks can access
  void timerUpdate();

protected:

  // Window system specific methods

  virtual void wsPostRedraw();
  virtual void wsSetCursor( CursorStyle c);
  virtual void wsSwapBuffers();
  virtual void wsSetTimer( double );

private:

  int d_window;
  bool d_timerPending;

};

#endif // _VIEWERGLUT_


