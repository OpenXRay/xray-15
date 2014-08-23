//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeBackground.h

#ifndef  _VRMLNODEBACKGROUND_
#define  _VRMLNODEBACKGROUND_

#include "VrmlNodeChild.h"
#include "VrmlField.h"

#include "VRMLImage.h"
#include "VrmlMFColor.h"
#include "VrmlMFFloat.h"
#include "VrmlMFString.h"
#include "VrmlSFString.h"
#include "Viewer.h"

//class Viewer;
class VrmlScene;

class VrmlNodeBackground : public VrmlNodeChild {

public:

  // Define the fields of Background nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeBackground(VrmlScene *);
  virtual ~VrmlNodeBackground();

  // Copy the node.
  virtual VrmlNode *cloneMe() const;

  virtual VrmlNodeBackground* toBackground() const;

  virtual void addToScene( VrmlScene *s, const char *relativeUrl );

  virtual ostream& printFields(ostream& os, int indent);

  // render backgrounds once per scene, not via the render() method
  void renderBindable(Viewer *);

  virtual void eventIn(double timeStamp,
		       const char *eventName,
		       const VrmlField *fieldValue);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  int nGroundAngles()		{ return d_groundAngle.size(); }
  float *groundAngle()		{ return d_groundAngle.get(); }
  float *groundColor()		{ return d_groundColor.get(); }

  int nSkyAngles()		{ return d_skyAngle.size(); }
  float *skyAngle()		{ return d_skyAngle.get(); }
  float *skyColor()		{ return d_skyColor.get(); }

private:

  VrmlMFFloat d_groundAngle;
  VrmlMFColor d_groundColor;

  VrmlMFString d_backUrl;
  VrmlMFString d_bottomUrl;
  VrmlMFString d_frontUrl;
  VrmlMFString d_leftUrl;
  VrmlMFString d_rightUrl;
  VrmlMFString d_topUrl;

  VrmlMFFloat d_skyAngle;
  VrmlMFColor d_skyColor;

  VrmlSFString d_relativeUrl;

  // Texture caches
  Image *d_texPtr[6];
  Image d_tex[6];

  // Display list object for background
  Viewer::Object d_viewerObject;

};

#endif // _VRMLNODEBACKGROUND_

