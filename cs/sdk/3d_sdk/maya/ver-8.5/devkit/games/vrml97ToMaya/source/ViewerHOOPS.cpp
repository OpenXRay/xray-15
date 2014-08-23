//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  ViewerHOOPS.cpp
//  Display of VRML models using HOOPS (www.hoops3d.com).
//

#include "config.h"

#if HAVE_HOOPS
#include "ViewerHOOPS.h"

#include <hc.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "HOOPSEvent.h"

#include "VrmlScene.h"


ViewerHOOPS::ViewerHOOPS(VrmlScene *scene) : Viewer(scene)
{
  char window[100];
  int nWindows = 0;

  // Avoid immediate exits, gratuitous call to ps
  HC_Define_System_Options("no message limit,application=VRML");

  // Generate a unique top level segment name by counting top level segments
  HC_Show_Alias("?Picture", window);

  HC_Begin_Segment_Search(window);
  HC_Show_Segment_Count(&nWindows);
  HC_End_Segment_Search();	// Get rid of the nominal Picture
  if (nWindows != 0)		// if it still exists, as we will
    HC_Delete_Segment(window);	// create our own.

  strcat(window,"*");

  HC_Begin_Segment_Search(window);
  HC_Show_Segment_Count(&nWindows);
  HC_End_Segment_Search();

  sprintf(&window[strlen(window)-1], "+%d", nWindows);

  // One-time initializations and top level settings
  if (nWindows == 0)
    {
      HC_Open_Segment("/driver"); {
	HC_Set_Handedness("right");
	HC_Set_Color("geometry=white,windows=black,light=gray");

	// Enable back plane culling of shells
	HC_Set_Heuristics("polygon handedness=left");

	// Lighting of faces only.
	HC_Set_Rendering_Options("color index interpolation=off,"
				 "color interpolation=(edges=off,faces=on)");

	HC_Set_Selectability("windows=v*^");
	HC_Set_Visibility("markers=off,faces=on,edges=off");
	HC_Set_Visibility("lighting=(edges=off,markers=off)");

      } HC_Close_Segment();
    }

  // Open and initialize the window
  d_windowKey = HC_KOpen_Segment(window); {
    // Window size (left,right,bottom,top) from -1 to 1 in each direction
    HC_Set_Driver_Options("subscreen=(-.5,.5,-.5,.5)");

    char title[40], driver[20];
    HC_Show_Device_Info(".", "driver type", driver);
    sprintf(title,"title='VRML HOOPS/%s Viewer [%d]'", driver, nWindows);
    HC_Set_Driver_Options(title);
    HC_Set_Window(-1.0,1.0,-1.0,1.0);

    HC_Enable_Selection_Events(".", ".");
    HC_Enable_Button_Events(".", "everything");

  } HC_Close_Segment();

  d_nTextures = 0;
}


ViewerHOOPS::~ViewerHOOPS()
{
  HC_Delete_By_Key(d_windowKey);
}


void ViewerHOOPS::beginUpdate()
{
  d_nTextures = 0;
  HC_Open_Segment_By_Key(d_windowKey);
}

void ViewerHOOPS::endUpdate()
{
  HC_Close_Segment();
}

//
//  Object creation.
//  Objects can be referred to later to avoid duplicating geometry
//

Viewer::Object ViewerHOOPS::beginObject(const char *name, bool)
{
  return HC_KOpen_Segment(name);
}

void ViewerHOOPS::endObject()
{
  HC_Close_Segment();
}

// Queries

void ViewerHOOPS::getPosition( float *x, float *y, float *z ) // ...
{
  *x = 0.0;			
  *y = 0.0;
  *z = 0.0;
}

void ViewerHOOPS::getOrientation( float *orientation ) // ...
{
  orientation[0] = 0.0;
  orientation[1] = 1.0;
  orientation[2] = 0.0;
  orientation[3] = 0.0;
}

int ViewerHOOPS::getRenderMode() { return RENDER_MODE_DRAW; }

double ViewerHOOPS::getRenderMode() { return 0.0; }

//
//  Geometry insertion. HOOPS can't insert a reference to a geometric object
//  directly, it only supports references to segments. That's why all of the 
//  geometry insert functions return null objects.
//

Viewer::Object ViewerHOOPS::insertBox(float x, float y, float z)
{
  float dx = 0.5 * x;
  float dy = 0.5 * y;
  float dz = 0.5 * z;

  // All the points are duplicated so the texture coords can be
  // set independently for each face.
  float pts[24][3] = {
    { -dx, -dy, -dz }, { -dx, -dy,  dz }, { -dx,  dy,  dz }, { -dx,  dy, -dz },
    { -dx, -dy,  dz }, {  dx, -dy,  dz }, {  dx,  dy,  dz }, { -dx,  dy,  dz },
    {  dx, -dy,  dz }, {  dx, -dy, -dz }, {  dx,  dy, -dz }, {  dx,  dy,  dz },
    {  dx, -dy, -dz }, { -dx, -dy, -dz }, { -dx,  dy, -dz }, {  dx,  dy, -dz },
    { -dx, -dy, -dz }, {  dx, -dy, -dz }, {  dx, -dy,  dz }, { -dx, -dy,  dz },
    { -dx,  dy,  dz }, {  dx,  dy,  dz }, {  dx,  dy, -dz }, { -dx,  dy, -dz }
  };

  int faces[] = { 4, 0, 1, 2, 3,
		  4, 4, 5, 6, 7,
		  4, 8, 9, 10, 11,
		  4, 12, 13, 14, 15,
		  4, 16, 17, 18, 19,
		  4, 20, 21, 22, 23 };

  float tc[4][3] = { { 0., 0., 0. },
		     { 1., 0., 0. },
		     { 1., 1., 0. },
		     { 0., 1., 0. } };
  long key;

  // We lose specularities, but it's faster.
  HC_Set_Rendering_Options("no lighting interpolation");
  key = HC_KInsert_Shell( 24, &pts[0], 30, &faces[0]);

  // Texture coordinates for each face
  HC_MSet_Vertex_Parameters( key, 0, 4, 3, &tc[0][0]);
  HC_MSet_Vertex_Parameters( key, 4, 4, 3, &tc[0][0]);
  HC_MSet_Vertex_Parameters( key, 8, 4, 3, &tc[0][0]);
  HC_MSet_Vertex_Parameters( key, 12, 4, 3, &tc[0][0]);
  HC_MSet_Vertex_Parameters( key, 16, 4, 3, &tc[0][0]);
  HC_MSet_Vertex_Parameters( key, 20, 4, 3, &tc[0][0]);

  return 0;
}


// Convert -1 terminated face list to counted vertex format.
// This converts the array in place, meaning that it can't be
// used to render with an OpenGL-based viewer after going through here...

static void convert_to_shell(int *faces, int nfaces)
{
  int np, n;

  for (np=0, n = nfaces-1; n > 0; --n)
    {
      if (faces[n] == -1)		// last pt of poly
	{
	  if (np > 0)
	    {
	      faces[n+1] = np-1;
	    }
	  np = 0;
	}

      faces[n] = faces[n-1];
      ++np;
    }
  faces[0] = np;
}


Viewer::Object ViewerHOOPS::insertCone(float h,
				       float r,
				       bool bottom, bool side)
{
  // Build a cylinder then push all the top points together.
  if (side)
    {
      const int nfacets = 8;		// Number of polygons for sides
      const int npts = 2 * nfacets;
      const int nfaces = nfacets * 5;

      float c[ npts ][ 3 ];		// coordinates
      float tc[ npts ][ 3 ];		// texture coordinates
      int faces[ nfaces ];		// face lists

      // should only compute tc if a texture is present...
      computeCylinder( h, r, nfacets, c, tc, faces);
      convert_to_shell( faces, nfaces);

      for (int i=0; i<nfacets; ++i)
	c[i][0] = c[i][2] = 0.0;

      long sideKey = HC_KInsert_Shell( npts, c, nfaces, faces);
      HC_MSet_Vertex_Parameters( sideKey, 0, npts, 3, &tc[0][0]);
    }

  if (bottom)
    {
      float p0[3] = { 0.0, -0.5 * h, -r };
      float p1[3] = { r,   -0.5 * h, 0.0 };
      float p2[3] = { 0.0, -0.5 * h, r };

      HC_Insert_Circle( p0, p1, p2); // texture coords...
    }

  if (! bottom || ! side)
    HC_Set_Heuristics( "no backplane cull"); // Not solid

  return 0;
}

Viewer::Object ViewerHOOPS::insertCylinder(float h,
					   float r,
					   bool bottom, bool side, bool top)
{
  if (side)
    {
      const int nfacets = 8;		// Number of polygons for sides
      const int npts = 2 * nfacets;
      const int nfaces = nfacets * 5;

      float c[ npts ][ 3 ];		// coordinates
      float tc[ npts ][ 3 ];		// texture coordinates
      int faces[ nfaces ];		// face lists

      // should only compute tc if a texture is present...
      computeCylinder( h, r, nfacets, c, tc, faces);
      convert_to_shell( faces, nfaces);

      long sideKey = HC_KInsert_Shell( npts, c, nfaces, faces);
      HC_MSet_Vertex_Parameters( sideKey, 0, npts, 3, &tc[0][0]);
    }

  if (bottom)
    {
      float p0[3] = { 0.0, -0.5 * h, -r };
      float p1[3] = { r,   -0.5 * h, 0.0 };
      float p2[3] = { 0.0, -0.5 * h, r };

      HC_Insert_Circle( p0, p1, p2); // texture coords...
    }

  if (top)
    {
      float p0[3] = { 0.0, 0.5 * h, r };
      float p1[3] = { r,   0.5 * h, 0.0 };
      float p2[3] = { 0.0, 0.5 * h, -r };

      HC_Insert_Circle( p0, p1, p2); // texture coords...
    }

  if (! bottom || ! side || ! top)
    HC_Set_Heuristics( "no backplane cull"); // Not solid


  return 0;
}


Viewer::Object ViewerHOOPS::insertDirLight(float ambient,
					   float intensity,
					   float rgb[],
					   float xyz[])
{
  Object key = HC_KOpen_Segment( "" ); {

    if (ambient > 0.0)
      HC_Set_Color_By_Value("ambient light", "RGB",
			    rgb[0]*ambient, rgb[1]*ambient, rgb[2]*ambient);

    HC_Set_Color_By_Value( "lights", "RGB",
			   rgb[0]*intensity, rgb[1]*intensity, rgb[2]*intensity);
    HC_Insert_Distant_Light( xyz[0], xyz[1], -xyz[2]);

  } HC_Close_Segment();

  return key;
}


Viewer::Object ViewerHOOPS::insertElevationGrid(unsigned int mask,
						int nx,
						int nz,
						float *height,
						float dx,
						float dz,
						float *texture_coords,
						float *normals,
						float */*colors*/)
{
  float *points = new float[nx * nz * 3];
  float *p = points;
  float x = 0.0;

  for (int i=0; i<nx; ++i)
    {
      float z = 0.0;
      for (int j=0; j<nz; ++j)
	{
	  *p++ = x;
	  *p++ = *height++;
	  *p++ = z;
	  z += dz;
	}
      x += dx;
    }

  if (! (mask & MASK_CCW) )
    HC_Set_Heuristics("polygon handedness=right");
  if (! (mask & MASK_SOLID) )
    HC_Set_Heuristics("no backplane cull");

  long key = HC_KInsert_Mesh( nx, nz, points );

  // HOOPS only supports 3 component tcs...
  float *tc = new float [3*nx*nz];

  p = tc;
  for (int i=0; i<nx; ++i)
    {
      for (int j=0; j<nz; ++j)
	{
	  if (texture_coords)
	    {
	      *p++ = *texture_coords++;
	      *p++ = *texture_coords++;
	    }
	  else
	    {
	      *p++ = ((float) i) / (nx-1);
	      *p++ = ((float) j) / (nz-1);
	    }
	  *p++ = 0.0;
	}
    }

  HC_MSet_Vertex_Parameters(key, 0, nx*nz, 3, tc);
  delete [] tc;

  // Assumes normals are per-vertex...
  if (normals)
    HC_MSet_Vertex_Normals(key, 0, nx*nz, normals);

  return 0;
}

Viewer::Object ViewerHOOPS::insertExtrusion( unsigned int mask,
					     int  nOrientation,
					     float *orientation,
					     int  nScale,
					     float *scale,
					     int  nCrossSection,
					     float *crossSection,
					     int nSpine,
					     float *spine )
{
  float *c  = new float[nCrossSection * nSpine * 3];
  float *tc = new float[nCrossSection * nSpine * 3];
  int nfaces = (nCrossSection-1) * (nSpine-1) * 5;
  int *faces = new int [nfaces];
  
  computeExtrusion( nOrientation, orientation,
		    nScale, scale,
		    nCrossSection, crossSection,
		    nSpine, spine,
		    (float [][3]) c,
		    (float [][3]) tc,
		    faces );
  convert_to_shell( faces, nfaces );

  int npts = nCrossSection * nSpine;
  long sideKey = HC_KInsert_Shell( npts, c, nfaces, faces);
  HC_MSet_Vertex_Parameters( sideKey, 0, npts, 3, tc);

  // Handle normals, ...

  if (! (mask & MASK_CCW) )
    HC_Set_Heuristics("polygon handedness=right");
  if (! (mask & MASK_CONVEX) )
    HC_Set_Heuristics("concave polygons");
  if (! (mask & MASK_SOLID) )
    HC_Set_Heuristics("no backplane cull");

  if (mask & MASK_BOTTOM)
    {
      HC_Insert_Polygon( nCrossSection, &c[0] );
    }
      
  if (mask & MASK_TOP)
    {
      HC_Insert_Polygon( nCrossSection, &c[3*(nSpine-1)*nCrossSection] );
    }

  delete [] c;
  delete [] tc;
  delete [] faces;

  return 0; 
}


Viewer::Object ViewerHOOPS::insertLineSet(int /*npoints*/,
					  float *points,
					  int nlines,
					  int *lines,
					  bool /*colorPerVertex*/,
					  float */*color*/,
					  int /*nColorIndex*/,
					  int */*colorIndex*/)
{
  HC_Restart_Ink();
  for (int i=0; i<nlines; ++i)
    {
      if (lines[i] == -1)
	{
	  HC_Restart_Ink();
	}
      else
	{
	  HC_Insert_Ink( points[ 3*lines[i] ],
			 points[ 3*lines[i]+1 ],
			 points[ 3*lines[i]+2 ] );
	}
    }
      
  return 0;
}

Viewer::Object ViewerHOOPS::insertPointSet(int npoints,
					   float *points,
					   float */*colors*/)
{
  float *p = points;

  for (int i=0; i<npoints; ++i, p += 3)
    HC_Insert_Marker( p[0], p[1], p[2] );

  return 0;
}


Viewer::Object
ViewerHOOPS::insertShell(unsigned int mask,
			 int npoints,
			 float *points,
			 int nfaces,
			 int *faces,
			 float *tc,
			 int ntci, int * /*tci*/,
			 float *normal,
			 int nni, int * /*ni*/,
			 float *color,
			 int nci, int * /*ci*/)
{
  if (faces[nfaces-1] == -1)
    convert_to_shell(faces, nfaces);

  if (! (mask & MASK_CCW) )
    HC_Set_Heuristics("polygon handedness=right");
  if (! (mask & MASK_CONVEX) )
    HC_Set_Heuristics("concave polygons");
  if (! (mask & MASK_SOLID) )
    HC_Set_Heuristics("no backplane cull");

  Object key = HC_KInsert_Shell_By_Ref(npoints,
				       points,
				       nfaces,
				       faces);

  // HOOPS only supports 3 component tcs, with a single value
  // per vertex, so texCoordIndex won't work...
  if (tc && ntci == 0)
    {
      float *tc3 = new float [3*npoints];
      for (int i=0; i<npoints; ++i)
	{
	  tc3[3*i+0] = *tc++;
	  tc3[3*i+1] = *tc++;
	  tc3[3*i+2] = 0.0;
	}
      HC_MSet_Vertex_Parameters(key, 0, npoints, 3, tc3);
      delete [] tc3;
    }

  // HOOPS only supports a single value per vtx, so no normalIndex...
  if (normal && (mask & MASK_NORMAL_PER_VERTEX) && nni == 0)
    HC_MSet_Vertex_Normals(key, 0, npoints, normal);

  // HOOPS only supports a single value per vtx, so no colorIndex...
  if (color && (mask & MASK_COLOR_PER_VERTEX) && nci == 0)
    HC_MSet_Vertex_Colors_By_Value(key, "faces", 0, "RGB", npoints, color);

  // per-face values...

  return 0;
}



Viewer::Object ViewerHOOPS::insertSphere(float radius)
{
  const int numLatLong = 9;
  const int npts = numLatLong * numLatLong;
  const int nfaces = (numLatLong - 1) * numLatLong * 5;

  float c[ npts ][ 3 ];
  float tc[ npts ][ 3 ];
  int faces[ nfaces ];

  // should only compute tc if a texture is present...
  computeSphere(radius, numLatLong, c, tc, faces);
  convert_to_shell(faces, nfaces);

  Object key = HC_KInsert_Shell( npts, c, nfaces, faces);
  HC_MSet_Vertex_Parameters(key, 0, npts, 3, &tc[0][0]);

  return 0;
}

Viewer::Object ViewerHOOPS::insertText(int , char **)
{
}

// Lightweight copy

Viewer::Object ViewerHOOPS::insertReference(Object existingObject)
{
  return HC_KInclude_Segment_By_Key(existingObject);
}

// Remove an object from the display list

void ViewerHOOPS::removeObject(Object key)
{
  HC_Delete_By_Key(key);
}


void ViewerHOOPS::enableLighting(bool lightsOn) 
{
  if (! lightsOn) HC_Set_Visibility("lights=off");
}

// Set attributes


void ViewerHOOPS::setColor(float r, float g, float b, float /*a*/) 
{
  HC_Set_Color_By_Value("geometry", "RGB", r, g, b); // alpha...
}

void ViewerHOOPS::setFog(float * /*color*/,
			 float   visibilityRange,
			 const char * /*fogType*/)
{
  char rendOption[64];
  float fogStart = 1.5;		// What should this be?...

  // HOOPS linearly interpolates between geometry color and background
  // color, so we can either set the background to the fog color or
  // just ignore fog color...
  sprintf(rendOption, "atmospheric attenuation=(hither=%g,yon=%g)",
	  fogStart, visibilityRange);

  HC_Set_Rendering_Options( rendOption );

}

void ViewerHOOPS::setMaterial(float ambientIntensity,
			      float diffuseColor[],
			      float /*emissiveColor*/[],
			      float /*shininess*/,
			      float /*specularColor*/[],
			      float /*transparency*/)
{
  setColor(ambientIntensity*diffuseColor[0],
	   ambientIntensity*diffuseColor[1],
	   ambientIntensity*diffuseColor[2]);
}

Viewer::Object ViewerHOOPS::insertTexture(int w, int h, int nc,
					  bool repeat_s,
					  bool repeat_t,
					  unsigned char *pixels)
{
  char image_spec[64];
  char image_name[32];
  char segment[32];

  // The only format HOOPS supports is RGB.
  if (nc != 3)
    {
      fprintf(stderr,"Unsupported texture depth (%d)\n", nc);
      return;
    }

  sprintf(image_name,"texture%d", ++d_nTextures);
  sprintf(segment,"?Style Library/%s", image_name);

  HC_Open_Segment(segment); {
    HC_Flush_Geometry(".");
    sprintf(image_spec,"RGB,name=%s", image_name);
    HC_Insert_Image(0.0, 0.0, 0.0, image_spec, w, h, pixels);
  } HC_Close_Segment();

  // tiling? HOOPS doesn't support separate s,t repeat flags...
  sprintf(image_spec,"tiling=%s,source='%s'",
	  (repeat_s || repeat_t) ? "on" : "off",
	  image_name);

  HC_Define_Texture(image_name, image_spec);

  sprintf(image_spec,"faces = ( diffuse = %s )", image_name);
  HC_Set_Color(image_spec);

  return 0;
}

// 

void ViewerHOOPS::setViewpoint(float *position,
			       float *orientation,
			       float fieldOfView,
			       float avatarSize,
			       float visibilityLimit)
{
  // Guess some distance along the sight line to use as a target...
  float zfar = visibilityLimit > 0.0 ? visibilityLimit : 200.0;
  float d = 0.25 * (avatarSize + zfar);

  float target[3], up[3];
  computeView(position, orientation, d, target, up);

  float w = 2.0 * d * tan( 0.5 * fieldOfView );
  HC_Set_Camera( position, target, up, w, w, "Perspective" );
}

// Transform

void ViewerHOOPS::setTransform(float * center,
			       float * rotation,
			       float * scale,
			       float * scaleOrientation,
			       float * translation) 
{
  HC_Translate_Object(-center[0], -center[1], -center[2]);

  if (scaleOrientation[3] != 0.0)
    HC_Rotate_Object_Offaxis(scaleOrientation[0],
			     scaleOrientation[1],
			     scaleOrientation[2],
			     -scaleOrientation[3]*180. / M_PI);

  HC_Scale_Object(scale[0], scale[1], scale[2]);

  if (scaleOrientation[3] != 0.0)
    HC_Rotate_Object_Offaxis(scaleOrientation[0],
			     scaleOrientation[1],
			     scaleOrientation[2],
			     scaleOrientation[3]*180. / M_PI);
  
  if (rotation[3] != 0.0)
    HC_Rotate_Object_Offaxis(rotation[0],
			     rotation[1],
			     rotation[2],
			     rotation[3]*180. / M_PI);
  
  HC_Translate_Object(center[0], center[1], center[2]);
  HC_Translate_Object(translation[0], translation[1], translation[2]);
}


// The viewer knows the current viewpoint
void ViewerHOOPS::transformPoints(int /*nPoints*/, float */*points*/)
{
  ;
}




#endif // HAVE_HOOPS
