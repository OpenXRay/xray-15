//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeOrientationInt.h

#ifndef  _VRMLNODEORIENTATIONINT_
#define  _VRMLNODEORIENTATIONINT_

#include "VrmlNodeChild.h"

#include "VrmlSFFloat.h"
#include "VrmlMFFloat.h"
#include "VrmlSFRotation.h"
#include "VrmlMFRotation.h"

class VrmlScene;


class VrmlNodeOrientationInt : public VrmlNodeChild {

public:

  // Define the fields of OrientationInt nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeOrientationInt( VrmlScene *scene = 0);
  virtual ~VrmlNodeOrientationInt();

  virtual VrmlNode *cloneMe() const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void eventIn(double timeStamp,
		       const char *eventName,
		       const VrmlField *fieldValue);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  virtual VrmlNodeOrientationInt* toOrientationInt() const;  
  virtual const VrmlMFFloat& getKey() const;   
  virtual const VrmlMFRotation& getKeyValue() const;   

private:

  // Fields
  VrmlMFFloat d_key;
  VrmlMFRotation d_keyValue;

  // State
  VrmlSFRotation d_value;
};

#endif // _VRMLNODEORIENTATIONINT_

