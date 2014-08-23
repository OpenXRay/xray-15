//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeCone.h

#ifndef  _VRMLNODECONE_
#define  _VRMLNODECONE_

#include "VrmlNodeGeometry.h"
#include "VrmlSFBool.h"
#include "VrmlSFFloat.h"

class VrmlScene;

class VrmlNodeCone : public VrmlNodeGeometry {

public:

  // Define the fields of cone nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeCone(VrmlScene *);
  virtual ~VrmlNodeCone();

  virtual VrmlNode *cloneMe() const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual Viewer::Object insertGeometry(Viewer *);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  virtual VrmlNodeCone* toCone() const; //LarryD Mar 08/99
  virtual bool getBottom() { return d_bottom.get(); } //LarryD Mar 08/99
  virtual bool getSide() { return d_side.get(); } //LarryD Mar 08/99
  virtual float getBottomRadius() { return d_bottomRadius.get(); } //LarryD Mar 08/99
  virtual float getHeight() { return d_height.get(); }  //LarryD Mar 08/99

protected:

  VrmlSFBool d_bottom;
  VrmlSFFloat d_bottomRadius;
  VrmlSFFloat d_height;
  VrmlSFBool d_side;

};

#endif // _VRMLNODECONE_
