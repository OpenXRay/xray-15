#include "config.h"
#if HAVE_HOOPS
/*
 *  HOOPS graphics event handling code
 */

#include <stdio.h>
#include <hc.h>

/* A factor of 1/2 should track mouse exactly (if we select on geometry...) */
#define HW_DOLLY_ACCEL 0.5

/* This should take field of view into account to be useful ... */
#define HW_PAN_ACCEL 45.

/* A distance of 2 (entire window) corresponds to 180 degree rotate. */
#define HW_ROTATE_ACCEL 90.

#define ABS(a) ((a<0.)?-(a):(a))

static int wf_mode = 1; /* use wireframe mode */

static char picture[128];

static char wf_vis[256], wf_heu[256], wf_rend[256];


static void wireframe_start()
{
  /* Set update interrupts */
  HC_QSet_Driver_Options( "?Driver", "update interrupts");
  HC_Open_Segment(picture);

  if (wf_mode)
    {
      /* Turn off everything ... */
      if (HC_Show_Existence("visibility"))
	HC_Show_Visibility(wf_vis);
      else
	wf_vis[0] = '\0';
      HC_Set_Visibility("geometry=off");

      /* except the wireframe. */

      /* HOOPS 3.2 specific: edge visibility */
      HC_Set_Visibility("lines=on,edges=on"); /*edges=perimeters only");*/

      if (HC_Show_Existence("heuristics"))
	HC_Show_Heuristics(wf_heu);
      else
	wf_heu[0] = '\0';
      HC_Set_Heuristics("no hidden surfaces");

      /* And make it stick ... */
      if (HC_Show_Existence("rendering options"))
	HC_Show_Rendering_Options(wf_rend);
      else
	wf_rend[0] = '\0';
      HC_Set_Rendering_Options("software frame buffer options=no retention");
      HC_Set_Rendering_Options("attribute lock=(visibility,heuristics)");
    }
  else
    {
      HC_Set_Rendering_Options("color index interpolation=off,"
			       "color interpolation=(edges=off,faces=on)");
    }

  HC_Close_Segment();
}

static void wireframe_end()
{
  /* Update interrupts off */
  HC_QSet_Driver_Options( "?Driver", "no update interrupts");
  HC_Open_Segment(picture);

  if (wf_mode)
    {
      HC_UnSet_Visibility();
      if (wf_vis[0]) HC_Set_Visibility(wf_vis);
      HC_UnSet_Heuristics();
      if (wf_heu[0]) HC_Set_Heuristics(wf_heu);
      HC_UnSet_Rendering_Options();
      if (wf_rend[0]) HC_Set_Rendering_Options(wf_rend);
    }
  else
    {
      HC_Set_Rendering_Options("color index interpolation=on,color interpolation=off");
    }

  HC_Close_Segment();
}

static void mouse_dolly(char *segments)
{
  float x1, y1, x2, y2, ignore, w, h;
  char initial_segment[256], selection[256], event[32], action[4];
  float sign = 1.;
  int status;

  HC_Show_Selection_Pathname(initial_segment);
  HC_QShow_Net_Handedness(initial_segment, event);
  if (strcmp(event, "left") == 0)
    sign = -1.;

  HC_Show_Selection(initial_segment);
  HC_Show_Selection_Position(&x1, &y1, &ignore, &ignore, &ignore, &ignore);
  HC_Show_Net_Camera_Field(&w, &h);
  w *= sign * HW_DOLLY_ACCEL;
  h *= sign * HW_DOLLY_ACCEL;

  /*  If selections are done over geometry, the movement will
   *  exactly track the mouse, but will be slower (proportional
   *  to size of model).
   */

  do {
    HC_Await_Event(event);
    if (strcmp(event, "selection"))
      continue;

    HC_Show_Selection(selection);
    if (strcmp(selection, initial_segment)) /* Should translate coords ...*/
      continue;

    HC_Show_Selection_Source(selection, selection, action, &status);
    HC_Show_Selection_Position(&x2, &y2, &ignore, &ignore, &ignore, &ignore);
    if ((x2 == -1.) && (y2 == -1.) && (action[0] == '^'))
      break;

    HC_QDolly_Camera(segments, (x2 - x1) * w, (y2 - y1) * h, 0.);
    x1 = x2;
    y1 = y2;
  } while ((*action != '^') && (*action != 'O'));
}

static void mouse_orbit(char *segments)
{
  float x1, y1, z1, x2, y2, z2, ignore, right, up;
  char initial_segment[256], selection[256], event[32], action[4];
  float sign, c0[3], c1[3];
  int status;

  wireframe_start();

  HC_Show_Selection_Pathname(initial_segment);
  HC_QShow_Net_Handedness(initial_segment, event);
  sign = (strcmp(event, "left") == 0) ? -1. : 1.;

  HC_Show_Selection(initial_segment);
  HC_Show_Selection_Position(&x1, &y1, &z1, &ignore, &ignore, &ignore);

  do {
    HC_Await_Event(event);
    if (strcmp(event, "selection"))
      continue;

    HC_Show_Selection_Source(selection, selection, action, &status);
    HC_Show_Selection_Position(&x2, &y2, &z2, &ignore, &ignore, &ignore);
    if ((x2 == -1.) && (y2 == -1.) && (action[0] == '^'))
      break;

    HC_Show_Selection(selection);
    if (strcmp(selection, initial_segment))
      {
	HC_Show_Selection_Pathname(selection);
	c0[0] = x2;
	c0[1] = y2;
	c0[2] = 0.;
	if (! HC_Compute_Coordinates(selection, "local window", c0,
				     "outer window", c1) ||
	    ! HC_Compute_Coordinates(".", "outer window", c1,
				     "local window", c0))
	  continue;
	x2 = c0[0];
	y2 = c0[1];
      }

    right = sign * HW_ROTATE_ACCEL * (x2 - x1);
    up = HW_ROTATE_ACCEL * (y1 - y2);

    HC_QOrbit_Camera(segments, right, up);

    x1 = x2;
    y1 = y2;

  } while ((*action != '^') && (*action != 'O'));

  wireframe_end();

}


static void mouse_zoom(char *segments)
{
  float wx1, wy1, wz1, wx2, wy2, wz2, wx3, wy3, wz3; /* window coords */
  float cx1, cy1, cz1, cx2, cy2, cz2, cx3, cy3, cz3; /* camera coords */
  float position[3],target[3],up[3]; /* zoom box camera spec */
  float box[5][3]; /* zoom box */
  char initial_segment[256], select[256], event[32], action[4];
  double zoom_factor;
  int status, out = 0;
  float tx,ty,tz,px,py,pz;


  /* Should turn off double buffering for this to look OK,
   * but changing a driver option causes a redraw.
   */

  HC_Show_Selection(initial_segment);
  HC_Show_Selection_Position(&wx1, &wy1, &wz1, &cx1, &cy1, &cz1);

  HC_Open_Segment("zoom");
  HC_Set_Heuristics("quick moves");
  HC_Flush_Contents(".","geometry");
  HC_Set_Handedness("right");
  position[1] = position[0] = 0.0;
  position[2] = 5.0;
  target[2] = target[1] = target[0] = 0.0;
  up[0] = 0.0;
  up[1] = 1.0;
  up[0] = 0.0;
  HC_Set_Camera(position, target, up, 2.0, 2.0, "s");

  box[0][0] = wx1;
  box[0][1] = wy1;
  box[0][2] = 0.0;
  box[1][1] = wy1;
  box[1][2] = 0.0;
  box[2][2] = 0.0;
  box[3][0] = wx1;
  box[3][2] = 0.0;
  box[4][0] = wx1;
  box[4][1] = wy1;
  box[4][2] = 0.0;

  action[0] = '*';

  do {
    HC_Await_Event(event);

    if (strcmp(event, "selection"))
      continue;

    HC_Show_Selection(select);
    if (strcmp(select, initial_segment))
      continue;

    HC_Show_Selection_Position(&wx2, &wy2, &wz2, &cx2, &cy2, &cz2);
    HC_Show_Selection_Source(select, select, action, &status);

    if ((wx2 == -1.) && (wy2 == -1.) && (action[0] == '^'))
      {
	out = 1; /* Left the window */
	break;
      }

    box[1][0] = wx2;
    box[2][0] = wx2;
    box[2][1] = wy2;
    box[3][1] = wy2;

    HC_Flush_Contents(".","geometry");
    HC_Insert_Polyline(5, box);

  } while ((*action != '^') && (*action != 'O'));

  HC_Flush_Contents(".","geometry");
  HC_Close_Segment(); /* zoom */

  if (! out)
    {
      /* Recalculate target selection over geometry. */
      if (HC_Show_Existence("selectability"))
	HC_Show_Selectability(select);
      else
	(void) strcpy(select, "none");
      HC_Set_Selectability("geometry");

      if (HC_Compute_Selection(picture, ".", "v", wx1, wy1))
	HC_Show_Selection_Position(&wx1, &wy1, &wz1, &cx1, &cy1, &cz1);

      if (HC_Compute_Selection(picture, ".", "v", wx2, wy2))
	HC_Show_Selection_Position(&wx2, &wy2, &wz2, &cx2, &cy2, &cz2);

      if (HC_Compute_Selection(picture, ".", "v",
			       0.5*(wx1+wx2),
			       0.5*(wy1+wy2)))
	HC_Show_Selection_Position(&wx3, &wy3, &wz3, &cx3, &cy3, &cz3);

      HC_UnSet_Selectability();
      if (strcmp(select, "none") != 0)
	HC_Set_Selectability(select);

      /* Effectively dolly the camera (move both target and position). */
      HC_Show_Net_Camera_Target(&tx, &ty, &tz);
      HC_Show_Net_Camera_Position(&px, &py, &pz);
      px -= tx;
      py -= ty;
      pz -= tz;
      tx = cx3; /*0.5 * (cx1 + cx2);*/
      ty = cy3; /*0.5 * (cy1 + cy2);*/
      tz = cz3; /*0.5 * (cz1 + cz2);*/

      HC_QSet_Camera_Target(segments, tx, ty, tz);

      {
	float len = px*px + py*py + pz*pz;

	if (len < 0.1)
	  {
	    px *= 100.0;
	    py *= 100.0;
	    pz *= 100.0;
	  }
      }

      HC_QSet_Camera_Position(segments, tx+px, ty+py, tz+pz);

#if 0
      printf(" t [ %g, %g, %g] p [ %g, %g, %g]\n",
	     tx, ty, tz, tx+px, ty+py, tz+pz);
#endif

      /* This just looks at largest delta ... */
      if (ABS(wx2-wx1) > ABS(wy2-wy1))
	zoom_factor = 0.5 * ABS(wx2 - wx1);
      else
	zoom_factor = 0.5 * ABS(wy2 - wy1);

      /* Did we move significantly? */
      if (zoom_factor < 0.04)
	return;

      /* Should we zoom in or out? */
      if (wx2 > wx1)
	zoom_factor = 1. / zoom_factor;

      HC_QZoom_Camera(segments, zoom_factor);
    }
}

static int handle_button()
{
  char button[32], option[32];
  float right, up;

  HC_Show_Button(button);
  switch (button[0]) {
  case 'Q':
  case 'q':
    return 1;

    /* double buffering */
  case 'd':
    HC_Open_Segment(picture);
    HC_Show_One_Net_Driver_Option("double-buffering",button);
    if (strcmp(button,"on") == 0)
      HC_Set_Driver_Options("no double-buffering");
    else
      HC_Set_Driver_Options("double-buffering");
    HC_Close_Segment();
    break;

    /* hardcopy */
  case 'h':
    fprintf(stderr,"Writing hardcopy lookat.ps ...");
    HC_Open_Segment("?Driver/postscript/lookat.ps");
    HC_Include_Segment(picture);
    HC_Close_Segment();
    HC_Update_Display();
    HC_Delete_Segment("?Driver/postscript/lookat.ps");
    break;

    /* lighting */
  case 'l':
    HC_Open_Segment(picture);
    HC_Show_One_Net_Rendering_Optio("lighting interpolation",option);
    if (strcmp(option,"off") == 0)
      {
	fprintf(stderr," Turning on smooth shading.\n");
	HC_Set_Rendering_Options("lighting interpolation=(faces=gouraud,edges=off)");
      }
    else
      {
	fprintf(stderr," Turning off smooth shading.\n");
	HC_Set_Rendering_Options("no lighting interpolation");
      }
    HC_Close_Segment();
    break;

    /* metafile */
  case 'm':
    fprintf(stderr,"Writing metafile lookat.hmf ...");
    HC_Write_Metafile(picture,"lookat","follow cross-references");
    fprintf(stderr," done.\n");
    break;

    /* rotate */
  case 'r':
    right = 22.;
    up = -18.;
    HC_Open_Segment(picture);
    for (;;) {
      if (HC_Check_For_Events()) {
	HC_Await_Event(button);
	if (button[0] == 's' || button[0] == 'b')
	  break;
      }
      HC_Orbit_Camera(right, up);
      HC_Update_Display();
    }
    HC_Close_Segment();
    break;

    /* texture mapping */
  case 't':
    HC_Open_Segment(picture);
    HC_Show_One_Net_Rendering_Optio("texture interpolation",option);
    if (strcmp(option,"off") == 0)
      {
	fprintf(stderr," Turning on texture mapping.\n");
	HC_Set_Rendering_Options("texture interpolation=(faces=on,edges=off)");
      }
    else
      {
	fprintf(stderr," Turning off texture mapping.\n");
	HC_Set_Rendering_Options("no texture interpolation");
      }
    HC_Close_Segment();
    break;

    /* wireframe mode */
  case 'w':
    HC_Open_Segment(picture);
    HC_Show_One_Net_Visibility("faces", button);
    if (strcmp(button,"on") == 0)
      {
	fprintf(stderr," Turning on wireframe mode\n");
	HC_Set_Visibility("faces=off, edges=on");
	HC_Set_Heuristics("no hidden surfaces");
	wf_mode = 0;
      }
    else
      {
	fprintf(stderr," Turning off wireframe mode\n");
	HC_Set_Visibility("faces=on, edges=off");
	HC_Set_Heuristics("hidden surfaces");
	wf_mode = 1;
      }
    HC_Close_Segment();
    break;

    /* hardware/software/no z-buffer */
  case 'z':
    HC_Open_Segment(picture);
    HC_Show_One_Net_Heuristic("hidden surfaces",button);
    if (strcmp(button,"on") == 0)
      {
	HC_Show_One_Net_Rendering_Optio("hsr algorithm",option);
	if (strcmp(option,"hardware z buffer") == 0)
	  {
	    fprintf(stderr," Turning on software hidden surfaces\n");
	    HC_Set_Rendering_Options("hsr algorithm=software z-buffer");
	  }
	else
	  {
	    fprintf(stderr," Turning off hidden surfaces\n");
	    HC_Set_Heuristics("no hidden surfaces");
	  }
      }
    else
      {
	fprintf(stderr," Turning on hardware hidden surfaces\n");
	HC_Set_Heuristics("hidden surfaces");
	HC_Set_Rendering_Options("hsr algorithm=hardware z-buffer");
      }

    HC_Close_Segment();
    break;

  }
  return 0;
}

void HOOPSEvents()
{
  char select[64];
  int status;

  HC_Show_Pathname_Expansion(".",picture);

  /* Default camera */
  if (! HC_Show_Existence("camera"))
    {
      float p0[3], p1[3], dx, dy;
      HC_Compute_Circumcuboid(".", p0, p1);
      dx = 0.05 * (p1[0] - p0[0]);
      dy = 0.05 * (p1[1] - p0[1]);

      if (p0[0] < p1[0] && p0[1] < p1[1])
	HC_Set_Camera_By_Volume("perspective",
				p0[0]-dx, p1[0]+dx,
				p0[1]-dy, p1[1]+dy);
    }

  for (;;) {
    HC_Await_Event(select);
    if (select[0] == 's') /* selection */
      {
	HC_Show_Selection_Source(select,select,select,&status);
	HC_Show_Selection_Pathname(select);
	HC_Open_Segment(select);
	HC_Show_Selection(select);

	switch (status)
	  {
	  case 1:
	    mouse_orbit(".");
	    break;
	  case 2:
	    mouse_zoom(".");
	    break;
	  case 4:
	    mouse_dolly(".");
	    break;
	  }
	HC_Close_Segment();
      }
    else if (select[0] == 'b') /* button */
      if (handle_button())
	break;
  }
}

#endif /* HAVE_HOOPS */

