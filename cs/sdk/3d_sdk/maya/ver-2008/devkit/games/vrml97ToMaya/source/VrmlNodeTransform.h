//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeTransform.h

#ifndef  _VRMLNODETRANSFORM_
#define  _VRMLNODETRANSFORM_

#include "VrmlNodeGroup.h"
#include "VrmlSFRotation.h"
#include "VrmlSFVec3f.h"

class VrmlNodeTransform : public VrmlNodeGroup {

public:

  // Define the fields of Transform nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeTransform(VrmlScene *);
  virtual ~VrmlNodeTransform();

  virtual VrmlNode *cloneMe() const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void render(Viewer *);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  virtual void accumulateTransform(VrmlNode*);
  virtual void inverseTransform(Viewer *);
  virtual void inverseTransform(double [4][4]);

  virtual VrmlNodeTransform* toTransform() const;     //LarryD Feb 24/99
  virtual const VrmlSFVec3f& getCenter() const;  //LarryD Feb 24/99
  virtual const VrmlSFRotation& getRotation() const;  //LarryD Feb 24/99
  virtual const VrmlSFVec3f& getScale() const;  //LarryD Feb 24/99
  virtual const VrmlSFRotation& getScaleOrientation() const;  //LarryD Feb 24/99
  virtual const VrmlSFVec3f& getTranslation() const;  //LarryD Feb 

private:

  VrmlSFVec3f d_center;
  VrmlSFRotation d_rotation;
  VrmlSFVec3f d_scale;
  VrmlSFRotation d_scaleOrientation;
  VrmlSFVec3f d_translation;

  Viewer::Object d_xformObject;
  
};

#endif // _VRMLNODETRANSFORM_

