//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  ViewerHOOPS.h
//  Class for display of VRML models using HOOPS (www.hoops3d.com)
//

#ifndef _VIEWERHOOPS_
#define _VIEWERHOOPS_

#include "Viewer.h"

class ViewerHOOPS : public Viewer {

public:

  ViewerHOOPS(VrmlScene *);
  virtual ~ViewerHOOPS();


  // Open/close display lists
  virtual Object beginObject(const char *name, bool retain);
  virtual void endObject();

  // Query
  virtual void getPosition( float *x, float *y, float *z );
  virtual void getOrientation( float *orientation );

  virtual int getRenderMode();

  virtual double getFrameRate();


  // Insert objects into the display list
  virtual Object insertBackground(int /*nGroundAngles*/,
				  float* /*groundAngle*/,
				  float* /*groundColor*/,
				  int /*nSkyAngles*/,
				  float* /*skyAngle*/,
				  float* /*skyColor*/,
				  int* /*whc*/,
				  unsigned char*[]) { return 0; }
			     
  virtual Object insertBox(float x, float y, float z);
  virtual Object insertCone(float h, float r, bool bottom, bool side);
  virtual Object insertCylinder(float h, float r, bool bottom, bool side, bool top);

  virtual Object insertElevationGrid(unsigned int mask,
				     int nx,
				     int nz,
				     float *height,
				     float dx,
				     float dz,
				     float *tc,
				     float *normals,
				     float *colors);

  virtual Object insertExtrusion(unsigned int mask,
				 int  /*nOrientation*/,
				 float */*orientation*/,
				 int  /*nScale*/,
				 float */*scale*/,
				 int  /*nCrossSection*/,
				 float */*crossSection*/,
				 int /*nSpine*/,
				 float */*spine*/ );

  virtual Object insertLineSet(int, float *, int, int *,
			       bool colorPerVertex,
			       float *color,
			       int nColorIndex,
			       int *colorIndex);

  virtual Object insertPointSet(int npts, float *pts, float *color);

  virtual Object insertShell(unsigned int mask,
			     int npoints, float *points,
			     int nfaces, int *faces,
			     float *tc,
			     int ntci, int *tci,
			     float *normal,
			     int nni, int *ni,
			     float *color,
			     int nci, int *ci);

  virtual Object insertSphere(float radius);

  virtual Object insertText(int n, char **s);

  // Lights
  virtual Object insertDirLight(float a, float i, float rgb[], float xyz[]);

  virtual Object insertPointLight(float, float [], float [],
				  float, float [], float ) { return 0; }

  virtual Object insertSpotLight( float /*ambientIntensity*/ ,
				  float /*attenuation*/ [],
				  float /*beamWidth*/ ,
				  float /*color*/ [],
				  float /*cutOffAngle*/ ,
				  float /*direction*/ [],
				  float /*intensity*/ ,
				  float /*location*/ [],
				  float /*radius*/ )  { return 0; }

  // Lightweight copy
  virtual Object insertReference(Object existingObject);

  // Remove an object from the display list
  virtual void removeObject(Object key);

  virtual void enableLighting(bool);

  // Set attributes
  virtual void setColor(float r, float g, float b, float a = 1.0);

  virtual void setFog(float * /*color*/,
		      float   /*visibilityRange*/,
		      const char * /*fogType*/);

  virtual void setMaterial(float, float[], float[], float, float[], float);

  virtual void scaleTexture(int /*w*/, int /*h*/,
			    int /*newW*/, int /*newH*/,
			    int /*nc*/,
			    unsigned char* /*pixels*/) {}

  virtual Object insertTexture(int w, int h, int nc,
			       bool repeat_s,
			       bool repeat_t,
			       unsigned char *pixels);

  // Transform
  virtual void setTransform(float * /*center*/,
			    float * /*rotation*/,
			    float * /*scale*/,
			    float * /*scaleOrientation*/,
			    float * /*translation*/);

  virtual void setViewpoint(float *position,
			    float *orientation,
			    float fieldOfView,
			    float avatarSize,
			    float visLimit);

  // The viewer knows the current viewpoint
  virtual void transformPoints(int nPoints, float *points);

  // Update the display
  virtual void updateDisplay();

private:

  long d_windowKey;
  int d_nTextures;

};

#endif // _VIEWERHOOPS_


