//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeSphere.h

#ifndef  _VRMLNODESPHERE_
#define  _VRMLNODESPHERE_

#include "VrmlNodeGeometry.h"
#include "VrmlSFFloat.h"


class VrmlNodeSphere : public VrmlNodeGeometry {

public:

  // Define the fields of sphere nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeSphere(VrmlScene *);
  virtual ~VrmlNodeSphere();

  virtual VrmlNode *cloneMe() const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual Viewer::Object insertGeometry(Viewer *);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  virtual VrmlNodeSphere* toSphere() const; //LarryD Mar 08/99
  virtual float getRadius() { return d_radius.get(); }   //LarryD Mar 08/99

protected:

  VrmlSFFloat d_radius;

};

#endif // _VRMLNODESPHERE_
