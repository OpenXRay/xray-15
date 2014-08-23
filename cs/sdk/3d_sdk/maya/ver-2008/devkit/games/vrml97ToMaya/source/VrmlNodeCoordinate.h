//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeCoordinate.h

#ifndef  _VRMLNODECOORDINATE_
#define  _VRMLNODECOORDINATE_

#include "VrmlNode.h"
#include "VrmlMFVec3f.h"

class VrmlNodeCoordinate : public VrmlNode {

public:

  // Define the fields of Coordinate nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeCoordinate(VrmlScene *);
  virtual ~VrmlNodeCoordinate();

  virtual VrmlNode *cloneMe() const;

  virtual VrmlNodeCoordinate* toCoordinate() const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  VrmlMFVec3f &coordinate()	{ return d_point; }

private:

  VrmlMFVec3f d_point;
  
};

#endif // _VRMLNODECOORDINATE_

