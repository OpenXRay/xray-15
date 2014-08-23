//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeCoordinateInt.h

#ifndef  _VRMLNODECOORDINATEINT_
#define  _VRMLNODECOORDINATEINT_

#include "VrmlNodeChild.h"

#include "VrmlSFFloat.h"
#include "VrmlMFFloat.h"
#include "VrmlMFVec3f.h"

class VrmlScene;


class VrmlNodeCoordinateInt : public VrmlNodeChild {

public:

  // Define the fields of CoordinateInt nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeCoordinateInt( VrmlScene *scene = 0);
  virtual ~VrmlNodeCoordinateInt();

  virtual VrmlNode *cloneMe() const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void eventIn(double timeStamp,
		       const char *eventName,
		       const VrmlField *fieldValue);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

private:

  // Fields
  VrmlMFFloat d_key;
  VrmlMFVec3f d_keyValue;

  // State
  VrmlMFVec3f d_value;
};

#endif // _VRMLNODECOORDINATEINT_

