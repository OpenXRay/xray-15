/* Modified by Chris Morley <cmorley@vermontel.com> to demonstrate
 * LibVRML97 ViewerXt class. 9 Sep 1998
 */
/*

                                BOILERPLATE


    This code is hereby placed under GNU GENERAL PUBLIC LICENSE.
    Copyright (C) 1996 Jeroen van der Zijp <jvz@cyberia.cfdrc.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


/* Include the kitchen sink */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/BulletinB.h>
#include <Xm/DialogS.h>
#include <Xm/FileSB.h>
#include <Xm/Frame.h>
#include <Xm/MessageB.h>
#include <Xm/Form.h>
#include <Xm/Separator.h>
#include <Xm/TextF.h>
#include <Xm/MwmUtil.h>

/* Now some good stuff */
#include "VrmlScene.h"
#include "ViewerXt.h"

VrmlScene *vrmlScene = 0;
ViewerXt *viewer = 0;

/* Stuff */
Display      *display;
XtAppContext  app_context;
Widget        toplevel;
Widget        mainwindow;
Widget        menubar;
Widget        mainform;
Widget        mainframe;
Widget        message;
Widget        viewpane;


#include "Doc.h"

// Subclass from the System class to direct messages to our message widget.
// Should put all output into a console widget...
#include "System.h"
#include <stdarg.h>

class SystemXm : public System {

public:

  virtual void inform(const char *, ...);

};

SystemXm xmSystem;

static void worldChangedCB(int);
static void openCB(Widget w,XtPointer client_data,XtPointer call_data);
static void byeCB(Widget w,XtPointer client_data,XtPointer call_data);
static void aboutCB(Widget w,XtPointer client_data,XtPointer call_data);
static void viewCB(Widget w,XtPointer client_data,XtPointer call_data);



/* Sample application */
int main(int argc, char *argv[]){
  int n;
  Arg args[30];
  Widget pane,cascade,but;

  /*
  ** Initialize toolkit
  ** We do *not* use XtAppInitialize as we want to figure visual and
  ** colormap BEFORE we create the top level shell!!!
  */
  XtToolkitInitialize();

  /* Make application context */
  app_context=XtCreateApplicationContext();

  /* Try open display */
  display=XtOpenDisplay(app_context,NULL,"xmlookat","xmLookat",
			NULL,0,&argc,argv);

  /* Report failure */
  if(!display){
    fprintf(stderr,"Unable to open the specified display.\n");
    fprintf(stderr,"Set your `DISPLAY' environment variable properly or\n");
    fprintf(stderr,"use the `xhost' command to authorize access to the display.\n");
    exit(1);
    }

  /* Check for extension; for Mesa, this is always cool */
  if(!glXQueryExtension(display,NULL,NULL)){
    fprintf(stderr,"The specified display does not support the OpenGL extension\n");
    exit(1);
    }

  /* Create application shell, finally */
  n=0;
  XtSetArg(args[n],XmNinput,True); n++;
  XtSetArg(args[n],XmNkeyboardFocusPolicy,XmPOINTER); n++;
  toplevel=XtAppCreateShell("xmlookat","xmLookat",
                            applicationShellWidgetClass,display,args,n);


  /* Main window */
  n=0;
  XtSetArg(args[n],XmNtraversalOn,True); n++;
  mainwindow=XmCreateMainWindow(toplevel,"window",args,n);
  XtManageChild(mainwindow);

  /* Make a menu */
  n=0;
  XtSetArg(args[n],XmNmarginWidth,0); n++;
  XtSetArg(args[n],XmNmarginHeight,0); n++;
  menubar=XmCreateMenuBar(mainwindow,"menubar",args,2);
  XtManageChild(menubar);

  n=0;
  pane=XmCreatePulldownMenu(menubar,"pane",args,n);
  n=0;
  but=XmCreatePushButton(pane,"Open",args,n);
  XtAddCallback(but,XmNactivateCallback,openCB,(XtPointer)NULL);
  XtManageChild(but);
  but=XmCreatePushButton(pane,"Save",args,n);
  XtManageChild(but);
  but=XmCreatePushButton(pane,"Save As",args,n);
  XtManageChild(but);
  but=XmCreatePushButton(pane,"Quit",args,n);
  XtAddCallback(but,XmNactivateCallback,byeCB,(XtPointer)NULL);
  XtManageChild(but);
  XtSetArg(args[0],XmNsubMenuId,pane);
  cascade=XmCreateCascadeButton(menubar,"File",args,1);
  XtManageChild(cascade);

  n=0;
  viewpane=XmCreatePulldownMenu(menubar,"pane",args,n);

  XtSetArg(args[0],XmNsubMenuId,viewpane);
  cascade=XmCreateCascadeButton(menubar,"Viewpoints",args,1);
  XtManageChild(cascade);

  but=XmCreatePushButton(viewpane,"Reset",0,0);
  XtAddCallback(but,XmNactivateCallback,viewCB,(XtPointer)-1);
  XtManageChild(but);

  but = XmCreateSeparator(viewpane,"",0,0);
  XtManageChild(but);

  n=0;
  pane=XmCreatePulldownMenu(menubar,"pane",args,n);

  n=0;
  but=XmCreatePushButton(pane,"About",args,n);
  XtAddCallback(but,XmNactivateCallback,aboutCB,(XtPointer)NULL);
  XtManageChild(but);
  XtSetArg(args[0],XmNsubMenuId,pane);
  cascade=XmCreateCascadeButton(menubar,"Help",args,1);
  XtManageChild(cascade);
  XtVaSetValues(menubar,XmNmenuHelpWidget,cascade,NULL);

  /* Main window form */
  n=0;
  XtSetArg(args[n],XmNmarginWidth,5); n++;
  XtSetArg(args[n],XmNmarginHeight,5); n++;
  mainform=XmCreateForm(mainwindow,"mainForm",args,n);
  XtManageChild(mainform);

  /* Main window frame */
  n = 0;
  XtSetArg(args[n],XmNleftAttachment,XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNrightAttachment,XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNtopAttachment,XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNbottomAttachment,XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNshadowType,XmSHADOW_IN); n++;
  XtSetArg(args[n],XmNtraversalOn,True); n++;
  XtSetArg(args[n],XmNwidth,320); n++;
  XtSetArg(args[n],XmNheight,240); n++;
  mainframe = XmCreateFrame(mainform,"mainFrame",args,n);
  XtManageChild(mainframe);

  Pixel bg;
  XtVaGetValues(mainform,XmNbackground,&bg,0);

  /* Message window */
  n = 0;
  XtSetArg(args[n],XmNeditable,False); ++n;
  XtSetArg(args[n],XmNcursorPositionVisible,False); ++n;
  XtSetArg(args[n],XmNbackground,bg); ++n;
  message = XmCreateTextField(mainwindow,"message",args,n);
  XtManageChild(message);

  n = 0;
  XtSetArg(args[n],XmNmessageWindow,message); ++n;
  XtSetValues(mainwindow,args,n);

  /* Use our system object */
  theSystem = &xmSystem;

  /* Vrml viewer */
  vrmlScene = new VrmlScene( argc > 1 ? argv[1] : 0 );
  /* Add scene callback, call it once since the initial world has already loaded */
  vrmlScene->addWorldChangedCallback( worldChangedCB );
  worldChangedCB( VrmlScene::REPLACE_WORLD );
  viewer = new ViewerXt( vrmlScene, "GL Widget", mainframe );
  if (! viewer)
    {
      fprintf(stderr,"Can't create OpenGL viewer.\n");
      exit(1);
    }
  viewer->update();

  /* Set into main window */
  XmMainWindowSetAreas(mainwindow,menubar,NULL,NULL,NULL,mainform);
  XtRealizeWidget(toplevel);

  /* Loop until were done */
  XtAppMainLoop(app_context);
  return 0;
}


static void worldChangedCB( int reason )
{
  switch (reason)
    {
    case VrmlScene::DESTROY_WORLD:
      delete viewer;
      delete vrmlScene;
      exit(0);
      break;

    case VrmlScene::REPLACE_WORLD:
      {
	const int MAXVIEWS = 20;
	static Widget btns[MAXVIEWS] = { 0 };
	int i, nviews = vrmlScene->nViewpoints();

	/* Get rid of any existing btns */
	for (i = 0; i<MAXVIEWS; ++i)
	  if (btns[i])
	    {
	      XtDestroyWidget(btns[i]);
	      btns[i] = 0;
	    }
      
	/* Query views */
	for (i = 0; i<nviews; ++i)
	  {
	    if (i == MAXVIEWS) break;

	    const char *name, *description;
	    char buf[64];
	    vrmlScene->getViewpoint( i, &name, &description );
	    if (name[0] == 0 && description[0] == 0)
	      continue;
	    strncpy(buf, name, sizeof(buf)-1);
	    if (buf[0] && description[0])
	      strncat(buf, ": ", sizeof(buf)-strlen(buf)-1);
	    strncat(buf, description, sizeof(buf)-strlen(buf)-1);
	    buf[sizeof(buf)-1] = '\0';

	    btns[i]=XmCreatePushButton(viewpane,buf,0,0);
	    XtAddCallback(btns[i],XmNactivateCallback,viewCB,(XtPointer)i);
	    XtManageChild(btns[i]);
	  }

	/* Window title */
	Doc *urlDoc = vrmlScene->urlDoc();
	if (urlDoc)
	  {
	    const char *title = urlDoc->urlBase();
	    if (title && *title)
	      XtVaSetValues(toplevel,XmNtitle,title,NULL);
	  }

	break;
      }
    }
}

static void cancelOpenCB(Widget fsb, XtPointer, XtPointer)
{
  XtUnmanageChild(fsb);
}

static void okOpenCB(Widget fsb, XtPointer , XtPointer calldata)
{
  XmFileSelectionBoxCallbackStruct *d =
    (XmFileSelectionBoxCallbackStruct *)calldata;
  char *fn = 0;

  if (XmStringGetLtoR(d->value, XmSTRING_DEFAULT_CHARSET, &fn))
    {
      vrmlScene->load(fn);
      viewer->update();
    }

  XtUnmanageChild(fsb);
}

static void openCB(Widget, XtPointer, XtPointer)
{
  static Widget fsb = 0;

  if (! fsb)
    {
      Arg args[1];
      int n = 0;
      char *name = "Open VRML97 File";

      XtSetArg(args[n],XmNtitle,name); ++n;
      fsb = XmCreateFileSelectionDialog( mainwindow, name, args, n);
      XtAddCallback(fsb,XmNcancelCallback,cancelOpenCB,(XtPointer)NULL);
      XtAddCallback(fsb,XmNokCallback,okOpenCB,(XtPointer)NULL);
    }

  XtManageChild(fsb);
}

/* Hasta la vista, baby */
static void byeCB(Widget, XtPointer, XtPointer)
{
  delete viewer;
  delete vrmlScene;
  exit(0);
}


/* Pop informative panel */
static void aboutCB(Widget, XtPointer, XtPointer)
{
  Arg args[10];
  XmString str;
  Widget box;
  str=XmStringCreateLtoR("Boilerplate Mixed Model Programming Example\n\n   (C) 1996 Jeroen van der Zijp \n\n    jvz@cyberia.cfdrc.com",XmSTRING_DEFAULT_CHARSET);
  XtSetArg(args[0],XmNnoResize,True);
  XtSetArg(args[1],XmNautoUnmanage,True);
  XtSetArg(args[2],XmNmessageString,str);
  XtSetArg(args[3],XmNdefaultPosition,False);
  box=XmCreateInformationDialog(toplevel,"About Boilerplate",args,4);
  XtManageChild(box);
  XtUnmanageChild(XmMessageBoxGetChild(box,XmDIALOG_HELP_BUTTON));
  XtUnmanageChild(XmMessageBoxGetChild(box,XmDIALOG_CANCEL_BUTTON));
  XmStringFree(str);
  }

/* Bind to the selected viewpoint */
static void viewCB(Widget, XtPointer clientData, XtPointer)
{
  int i = (int) clientData;
  if (vrmlScene)
    {
      if (i == -1)
	{
	  viewer->resetUserNavigation();
	}
      else
	{
	  const char *name, *description;
	  vrmlScene->getViewpoint( i, &name, &description );
	  vrmlScene->setViewpoint( name, description );
	}
    }
}


// Write to the message widget

void SystemXm::inform(const char *fmt, ...)
{
  static char lastbuf[1024] = { 0 };
  char buf[1024];

  va_list ap;
  va_start(ap, fmt);
  vsprintf(buf, fmt, ap);
  va_end(ap);
  if (strcmp(lastbuf, buf))
    {
      XtVaSetValues( message, XmNvalue, buf, 0 );
      strcpy(lastbuf, buf);
    }
}

