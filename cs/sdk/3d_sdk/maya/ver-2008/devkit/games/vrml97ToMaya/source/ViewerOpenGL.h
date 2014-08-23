//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  ViewerOpenGL.h
//  Abstract class for display of VRML models using OpenGL/Mesa.
//  A window-system specific subclass needs to redefine the pure
//  virtual methods.
//

#ifndef _VIEWEROPENGL_
#define _VIEWEROPENGL_

#include "config.h"
#include "Viewer.h"

#ifdef HAVE_GL

// Use the stencil buffer to set the SHAPE mask.
#define USE_STENCIL_SHAPE 0


#if defined(WIN32)
// Win32 needs to know that GL stuff lives in a DLL (OPENGL32.DLL)
#include <wtypes.h>
#include <winbase.h>
#include <windef.h>
#include <wingdi.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>


class ViewerOpenGL : public Viewer {

public:

  enum { MAX_LIGHTS = 8 };

  ViewerOpenGL(VrmlScene *);
  virtual ~ViewerOpenGL();

  // Queries
  virtual void getPosition( float *x, float *y, float *z );
  virtual void getOrientation( float *orientation );

  virtual int getRenderMode();
  virtual double getFrameRate();

  //
  virtual void resetUserNavigation();

  // Scope dirlights, open/close display lists
  virtual Object beginObject(const char *name, bool retain);
  virtual void endObject();

  // Insert objects into the display list
  virtual Object insertBackground(int nGroundAngles = 0,
				  float* groundAngle = 0,
				  float* groundColor = 0,
				  int nSkyAngles = 0,
				  float* skyAngle = 0,
				  float* skyColor = 0,
				  int* whc = 0,
				  unsigned char** pixels = 0);


  virtual Object insertBox(float x, float y, float z);
  virtual Object insertCone(float h, float r, bool bottom, bool side);
  virtual Object insertCylinder(float h, float r, bool, bool, bool);

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
				 int   nOrientation,
				 float *orientation,
				 int   nScale,
				 float *scale,
				 int   nCrossSection,
				 float *crossSection,
				 int   nSpine,
				 float *spine );

  virtual Object insertLineSet(int, float *, int, int *,
			       bool colorPerVertex,
			       float *color,
			       int nColorIndex,
			       int *colorIndex);

  virtual Object insertPointSet(int npts, float *points, float *colors);
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

  virtual Object insertText(int *, float, int n, char **s);

  // Lights
  virtual Object insertDirLight(float a, float i, float rgb[], float xyz[]);

  virtual Object insertPointLight( float ambientIntensity,
				   float attenuation[],
				   float color[],
				   float intensity,
				   float location[],
				   float radius );

  virtual Object insertSpotLight( float /*ambientIntensity*/ ,
				  float /*attenuation*/ [],
				  float /*beamWidth*/ ,
				  float /*color*/ [],
				  float /*cutOffAngle*/ ,
				  float /*direction*/ [],
				  float /*intensity*/ ,
				  float /*location*/ [],
				  float /*radius*/ );


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

  virtual void setMaterialMode( int nTexComponents, bool geometryColor );

  virtual void setSensitive(void *object);

  virtual void scaleTexture(int /*w*/, int /*h*/,
			    int /*newW*/, int /*newH*/,
			    int /*nc*/,
			    unsigned char* /*pixels*/);

  virtual TextureObject insertTexture(int w, int h, int nc,
				      bool repeat_s,
				      bool repeat_t,
				      unsigned char *pixels,
				      bool retainHint = false);

  // Reference/remove a texture object
  virtual void insertTextureReference(TextureObject, int);
  virtual void removeTextureObject(TextureObject);

  virtual void setTextureTransform( float * /*center*/,
				    float /*rotation*/,
				    float * /*scale*/,
				    float * /*translation*/ );

  virtual void setTransform(float * /*center*/,
			    float * /*rotation*/,
			    float * /*scale*/,
			    float * /*scaleOrientation*/,
			    float * /*translation*/);

  virtual void unsetTransform(float * /*center*/,
			      float * /*rotation*/,
			      float * /*scale*/,
			      float * /*scaleOrientation*/,
			      float * /*translation*/);

  virtual void setBillboardTransform(float * /*axisOfRotation*/);
  virtual void unsetBillboardTransform(float * /*axisOfRotation*/);


  virtual void setViewpoint(float *position,
			    float *orientation,
			    float fieldOfView,
			    float avatarSize,
			    float visLimit);

  // The viewer knows the current viewpoint
  virtual void transformPoints(int nPoints, float *points);


  //
  // Viewer callbacks (not for public consumption)

  // Update the model.
  void update( double time = 0.0 );

  // Redraw the screen.
  virtual void redraw();
  void resize(int w, int h);

  // Event types
  typedef enum {
    EVENT_KEY_DOWN,
    EVENT_MOUSE_MOVE,
    EVENT_MOUSE_CLICK,
    EVENT_MOUSE_DRAG,
    EVENT_MOUSE_RELEASE
  } EventType;

  enum {
    KEY_HOME,
    KEY_LEFT,
    KEY_UP,
    KEY_RIGHT,
    KEY_DOWN,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN
  };

  typedef struct {
    EventType event;
    int what;			// key or button number
    int x, y;
  } EventInfo;
    
  void input( EventInfo *);

  // Structs used in tesselator callbacks
  typedef struct IndexData {
    float *v;			// data values
    int ni, *i;			// number of indices, index pointer
  } IndexData;

  typedef struct ShellData {
    unsigned int mask;
    float *points;
    int nfaces, *faces;		// face list
    IndexData texCoord;		// texture coordinates and indices
    IndexData normal;		// normals and indices
    IndexData color;		// colors and indices
    int *texAxes;
    float *texParams;
    int nf, i;
  } ShellData;


protected:
  typedef enum {
    CURSOR_INHERIT,
    CURSOR_INFO,
    CURSOR_CYCLE,
    CURSOR_UP_DOWN,
    CURSOR_CROSSHAIR
  } CursorStyle;

  // Window system specific methods

  virtual void wsPostRedraw() = 0;
  virtual void wsSetCursor( CursorStyle c) = 0;
  virtual void wsSwapBuffers() = 0;
  virtual void wsSetTimer( double ) = 0;

private:

  // Initialize OpenGL state
  void initialize();

  // Geometry insertion setup & cleanup methods
  void beginGeometry();
  void endGeometry();

  // Text rendering
  void text2(int x, int y, float scale, char *text);
  void text3(int *justify, float size, int n, const char **s);

  // IndexedFaceSet helpers
  void insertShellConvex(ShellData *);
  void insertShellTess(ShellData *);

  // User interaction
  void step(float, float, float);
  void handleKey(int);
  void handleButton(EventInfo*);
  void handleMouseDrag(int, int);

  // GL status
  bool d_GLinitialized;
  bool d_blend;
  bool d_lit;
  bool d_texture;
  bool d_wireframe;

  int d_window;
  int d_winWidth, d_winHeight;
  float d_background[3];

  // Groups used to be put in display lists, now these counters
  // trigger various initializations and clean ups
  int d_nObjects, d_nestedObjects;

  // Tessellation
  GLUtriangulatorObj *d_tess;
  void insertExtrusionCaps( unsigned int mask, int nSpine, float *c,
			    int nCrossSection, float *cs );

  // Check for pickable entity selection
  bool checkSensitive(int x, int y, EventType );

  // Pickable entities
  int d_nSensitive;
  int d_activeSensitive;
  int d_overSensitive;

  enum { MAXSENSITIVE = 200 };	// make dynamic?...
  void *d_sensitiveObject[ MAXSENSITIVE ];

  bool d_selectMode;
  double d_selectZ;		// window Z coord of last selection

  // Lights
  typedef enum { LIGHT_UNUSED, LIGHT_DIRECTIONAL, LIGHT_POSITIONAL } LightType;
  typedef struct lightinfo {
    LightType lightType;
    int nestingLevel;
    float radiusSquared;
    float location[3];
  } LightInfo;

  LightInfo d_lightInfo[MAX_LIGHTS];

  // View manipulation
  float d_position[3], d_target[3];

  int d_beginx, d_beginy;
  float d_scale;
  float d_translatex, d_translatey, d_translatez;

  // quaternion representations of last, current rotation
  float d_lastquat[4], d_curquat[4];
  bool d_rotationChanged;
  float d_rotationMatrix[4][4];

  bool d_rotating, d_scaling, d_translating;

  bool d_reportFPS;
  double d_renderTime, d_renderTime1;

};

#endif // HAVE_GL
#endif // _VIEWEROPENGL_


