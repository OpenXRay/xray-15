//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeBillboard.h

#ifndef  _VRMLNODEBILLBOARD_
#define  _VRMLNODEBILLBOARD_

#include "VrmlNodeGroup.h"
#include "VrmlSFVec3f.h"


class VrmlNodeBillboard : public VrmlNodeGroup {

public:

  // Define the fields of Billboard nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeBillboard(VrmlScene *);
  virtual ~VrmlNodeBillboard();

  virtual VrmlNode *cloneMe() const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void render(Viewer *);

  virtual void accumulateTransform(VrmlNode*);
  virtual VrmlNode* getParentTransform();
  virtual void inverseTransform(Viewer *);
  virtual void inverseTransform(double [4][4]);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

private:

  VrmlSFVec3f d_axisOfRotation;

  VrmlNode *d_parentTransform;
  Viewer::Object d_xformObject;
  
};

#endif // _VRMLNODEBILLBOARD_

