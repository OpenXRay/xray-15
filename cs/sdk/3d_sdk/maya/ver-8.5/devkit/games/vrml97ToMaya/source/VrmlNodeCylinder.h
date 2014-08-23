//
//  Vrml97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeCylinder.h

#ifndef  _VRMLNODECYLINDER_
#define  _VRMLNODECYLINDER_

#include "VrmlNodeGeometry.h"
#include "VrmlSFBool.h"
#include "VrmlSFFloat.h"

class VrmlNodeCylinder : public VrmlNodeGeometry {

public:

  // Define the fields of cylinder nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeCylinder(VrmlScene *);
  virtual ~VrmlNodeCylinder();

  virtual VrmlNode *cloneMe() const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual Viewer::Object insertGeometry(Viewer *);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  virtual VrmlNodeCylinder* toCylinder() const; //LarryD Mar 08/99
  virtual bool getBottom() { return d_bottom.get(); }  //LarryD Mar 08/99
  virtual bool getSide() { return d_side.get(); }  //LarryD Mar 08/99
  virtual bool getTop() { return d_top.get(); }  //LarryD Mar 08/99
  virtual float getHeight() { return d_height.get(); }  //LarryD Mar 08/99
  virtual float getRadius() { return d_radius.get(); }  //LarryD Mar 08/99

protected:

  VrmlSFBool d_bottom;
  VrmlSFFloat d_height;
  VrmlSFFloat d_radius;
  VrmlSFBool d_side;
  VrmlSFBool d_top;

};

#endif // _VRMLNODECYLINDER_
