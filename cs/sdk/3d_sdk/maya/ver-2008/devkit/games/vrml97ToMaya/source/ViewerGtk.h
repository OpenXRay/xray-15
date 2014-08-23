// GTK Viewer for Vrml 97 library
//
// Copyright (C) 1998 by Erik Andersen <andersee@debian.org>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

#ifndef _VIEWERGTK_
#define _VIEWERGTK_

#include "config.h"

#if HAVE_GTK

#include "ViewerOpenGL.h"
#include <gtk/gtk.h>
#include <GL/glx.h>		// GLXContext
#include <gtkglarea.h>



class ViewerGtk : public ViewerOpenGL {

public:

  // Create a named OpenGL drawing area widget
  ViewerGtk(VrmlScene *, const char *, GtkWidget* parent);
  virtual ~ViewerGtk();

  // public so the callback functions can access them
  void timerUpdate();
  void handleInput( ViewerOpenGL::EventInfo *e );
  void handleRedraw();
  void SetStop( bool state);

protected:

  // Window system specific methods

  virtual void wsPostRedraw();
  virtual void wsSetCursor( CursorStyle c);
  virtual void wsSwapBuffers();
  virtual void wsSetTimer( double );

private:
  void createWidget( GtkWidget *parent );

  guint d_timer;
  bool d_Stop;
  bool d_redrawNeeded;
  gint d_width, d_height;
  GtkWidget* d_glarea;

};

#endif // HAVE_GTK
#endif // _VIEWERGTK_


