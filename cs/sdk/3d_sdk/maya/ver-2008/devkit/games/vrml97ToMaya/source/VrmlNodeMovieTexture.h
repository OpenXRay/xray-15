//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeMovieTexture.h

#ifndef  _VRMLNODEMOVIETEXTURE_
#define  _VRMLNODEMOVIETEXTURE_

#include "VrmlNodeTexture.h"
#include "VrmlMFString.h"
#include "VrmlSFBool.h"
#include "VrmlSFFloat.h"
#include "VrmlSFTime.h"
#include "Viewer.h"

class Image;

class VrmlNodeMovieTexture : public VrmlNodeTexture {

public:

  // Define the fields of MovieTexture nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeMovieTexture(VrmlScene *);
  virtual ~VrmlNodeMovieTexture();

  virtual VrmlNode *cloneMe() const;

  virtual VrmlNodeMovieTexture* toMovieTexture() const;

  virtual void addToScene( VrmlScene *s, const char *relUrl );

  virtual ostream& printFields(ostream& os, int indent);

  void update( VrmlSFTime &now );

  virtual void render(Viewer *);

  virtual void eventIn(double timeStamp,
		       const char *eventName,
		       const VrmlField *fieldValue);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  virtual int nComponents();
  virtual int width();
  virtual int height();
  virtual int nFrames();
  virtual unsigned char* pixels();

private:

  VrmlSFBool d_loop;
  VrmlSFFloat d_speed;
  VrmlSFTime d_startTime;
  VrmlSFTime d_stopTime;

  VrmlMFString d_url;
  VrmlSFBool d_repeatS;
  VrmlSFBool d_repeatT;

  VrmlSFTime d_duration;
  VrmlSFBool d_isActive;

  Image *d_image;
  int d_frame, d_lastFrame;
  double d_lastFrameTime;

  Viewer::TextureObject d_texObject;

};

#endif // _VRMLNODEMOVIETEXTURE_

