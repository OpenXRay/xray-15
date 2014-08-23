//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeTextureTransform.h

#ifndef  _VRMLNODETEXTURETRANSFORM_
#define  _VRMLNODETEXTURETRANSFORM_

#include "VrmlNode.h"
#include "VrmlSFFloat.h"
#include "VrmlSFVec2f.h"

class Viewer;

class VrmlNodeTextureTransform : public VrmlNode {

public:

  // Define the fields of TextureTransform nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeTextureTransform(VrmlScene *);
  virtual ~VrmlNodeTextureTransform();

  virtual VrmlNode *cloneMe() const;

  virtual VrmlNodeTextureTransform* toTextureTransform()	const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void render(Viewer *);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  float *center()	{ return d_center.get(); }
  float  rotation()	{ return d_rotation.get(); }
  float *scale()	{ return d_scale.get(); }
  float *translation()	{ return d_translation.get(); }

private:

  VrmlSFVec2f d_center;
  VrmlSFFloat d_rotation;
  VrmlSFVec2f d_scale;
  VrmlSFVec2f d_translation;
  
};

#endif // _VRMLNODETEXTURETRANSFORM_

