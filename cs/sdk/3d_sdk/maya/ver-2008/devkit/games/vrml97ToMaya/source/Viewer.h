//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  Viewer.h
//  Abstract base class for display of VRML models
//

#ifndef _VIEWER_
#define _VIEWER_

class VrmlScene;

class Viewer {

protected:
  // Explicitly instantiate a subclass object
  Viewer(VrmlScene *scene) : d_scene(scene) {}

public:

  virtual ~Viewer() = 0;

  // Options flags
  enum {
    MASK_NONE	= 0,
    MASK_CCW	= 1,
    MASK_CONVEX	= 2,
    MASK_SOLID	= 4,
    MASK_BOTTOM = 8,
    MASK_TOP	= 16,
    MASK_SIDE	= 32,
    MASK_COLOR_PER_VERTEX  = 64,
    MASK_NORMAL_PER_VERTEX = 128
  };

  // Object and texture keys. Don't mix them up.
  typedef long Object;
  typedef long TextureObject;

  // 
  VrmlScene *scene() { return d_scene; }

  // Query
  virtual void getPosition( float *x, float *y, float *z ) = 0;
  virtual void getOrientation( float *orientation ) = 0;

  enum {
    RENDER_MODE_DRAW,
    RENDER_MODE_PICK
  };

  // Return renderMode
  virtual int getRenderMode() = 0;

  virtual double getFrameRate() = 0;

  // Need a query user nav too...
  virtual void resetUserNavigation() = 0;

  // Open/close display lists
  virtual Object beginObject(const char *, bool = false) = 0;
  virtual void endObject() = 0;

  // Insert objects into the display list
  virtual Object insertBackground(int = 0    /*nGroundAngles*/,
				  float* = 0 /*groundAngle*/,
				  float* = 0 /*groundColor*/,
				  int = 0    /*nSkyAngles*/,
				  float* = 0 /*skyAngle*/,
				  float* = 0 /*skyColor*/,
				  int* = 0, unsigned char ** = 0) = 0;
			     
  virtual Object insertBox(float, float, float ) = 0;
  virtual Object insertCone(float, float, bool, bool) = 0;
  virtual Object insertCylinder(float, float, bool, bool, bool) = 0;

  virtual Object insertElevationGrid(unsigned int /*mask*/,
				     int  /*nx*/,
				     int  /*nz*/,
				     float * /*height*/,
				     float /*dx*/,
				     float /*dz*/,
				     float * /*tc*/,
				     float * /*normals*/,
				     float *colors ) = 0;

  virtual Object insertExtrusion(unsigned int,
				 int  /*nOrientation*/,
				 float * /*orientation*/,
				 int  /*nScale*/,
				 float * /*scale*/,
				 int  /*nCrossSection*/,
				 float * /*crossSection*/,
				 int   /*nSpine*/,
				 float * /*spine*/) = 0;

  virtual Object insertLineSet(int nCoords, float *coord,
			       int nCoordIndex, int *coordIndex,
			       bool colorPerVertex,
			       float *color,
			       int nColorIndex,
			       int *colorIndex) = 0;

  virtual Object insertPointSet(int nv, float *v, float *c) = 0;

  virtual Object insertShell(unsigned int mask,
			     int npoints, float *points,
			     int nfaces, int *faces,
			     float *tc,
			     int ntci, int *tci,
			     float *normal,
			     int nni, int *ni,
			     float *color,
			     int nci, int *ci) = 0;

  virtual Object insertSphere(float /*radius*/) = 0;

  virtual Object insertText(int*, float size, int n, char **s) = 0;

  // Lights
  virtual Object insertDirLight(float, float, float [], float []) = 0;

  virtual Object insertPointLight(float, float [], float [],
				  float, float [], float ) = 0;

  virtual Object insertSpotLight( float /*ambientIntensity*/ ,
				  float /*attenuation*/ [],
				  float /*beamWidth*/ ,
				  float /*color*/ [],
				  float /*cutOffAngle*/ ,
				  float /*direction*/ [],
				  float /*intensity*/ ,
				  float /*location*/ [],
				  float /*radius*/ ) = 0;

  // Lightweight copy
  virtual Object insertReference(Object /*existingObject*/) = 0;

  // Remove an object from the display list
  virtual void removeObject(Object) = 0;

  virtual void enableLighting(bool) = 0;

  // Set attributes
  virtual void setFog(float * /*color*/,
		      float   /*visibilityRange*/,
		      const char * /*fogType*/) = 0;

  virtual void setColor(float r, float g, float b, float a = 1.0) = 0;

  virtual void setMaterial(float, float[], float[], float, float[], float) = 0;

  virtual void setMaterialMode( int nTexComponents, bool geometryColor ) = 0;

  virtual void setSensitive(void *object) = 0;

  virtual void scaleTexture(int /*w*/, int /*h*/,
			    int /*newW*/, int /*newH*/,
			    int /*nc*/,
			    unsigned char* /*pixels*/) = 0;

  // Create a texture object
  virtual TextureObject insertTexture(int /*w*/, int /*h*/, int /*nc*/,
				      bool /*repeat_s*/,
				      bool /*repeat_t*/,
				      unsigned char* /*pixels*/,
				      bool retainHint = false) = 0;

  // Reference/remove a texture object
  virtual void insertTextureReference(TextureObject, int) = 0;
  virtual void removeTextureObject(TextureObject) = 0;

  virtual void setTextureTransform( float * /*center*/,
				    float /*rotation*/,
				    float * /*scale*/,
				    float * /*translation*/ ) = 0;

  virtual void setTransform(float * /*center*/,
			    float * /*rotation*/,
			    float * /*scale*/,
			    float * /*scaleOrientation*/,
			    float * /*translation*/) = 0;

  // This is a hack to work around the glPushMatrix() limit (32 deep on Mesa).
  // It has some ugly disadvantages: it is slower and the resulting transform
  // after a setTransform/unsetTransform may not be identical to the original.
  // It might be better to just build our own matrix stack...
  virtual void unsetTransform(float * /*center*/,
			      float * /*rotation*/,
			      float * /*scale*/,
			      float * /*scaleOrientation*/,
			      float * /*translation*/) = 0;

  virtual void setBillboardTransform(float * /*axisOfRotation*/) = 0;

  virtual void unsetBillboardTransform(float * /*axisOfRotation*/) = 0;

  virtual void setViewpoint(float * /*position*/,
			    float * /*orientation*/,
			    float /*fieldOfView*/,
			    float /*avatarSize*/,
			    float /*visLimit*/) = 0;

  // The viewer knows the current viewpoint
  virtual void transformPoints(int nPoints, float *points) = 0;


protected:

  void computeCylinder(double height,
		       double radius,
		       int numFacets,
		       float c[][3],
		       float tc[][3],
		       int faces[]);

  void computeExtrusion( int  /*nOrientation*/,
			 float * /*orientation*/,
			 int  /*nScale*/,
			 float * /*scale*/,
			 int  /*nCrossSection*/,
			 float * /*crossSection*/,
			 int   /*nSpine*/,
			 float * /*spine*/,
			 float *c,   // OUT: coordinates
			 float *tc,  // OUT: texture coords
			 int *faces);    // OUT: face list

  void computeSphere(double radius,
		     int numLatLong,
		     float c[][3],
		     float tc[][3],
		     int faces[]);

  void computeView(float position[3],
		   float orientation[3],
		   float distance,
		   float target[3],
		   float up[3]);

  VrmlScene *d_scene;

private:

  Viewer() {}			// Don't allow default constructors

};

#endif // _VIEWER_
