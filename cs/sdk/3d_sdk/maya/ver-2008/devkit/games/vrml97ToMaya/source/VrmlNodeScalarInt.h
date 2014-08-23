//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeScalarInt.h

#ifndef  _VRMLNODESCALARINT_
#define  _VRMLNODESCALARINT_

#include "VrmlNodeChild.h"

#include "VrmlSFFloat.h"
#include "VrmlMFFloat.h"

class VrmlScene;


class VrmlNodeScalarInt : public VrmlNodeChild {

public:

  // Define the fields of ScalarInt nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeScalarInt( VrmlScene *scene = 0 );
  virtual ~VrmlNodeScalarInt();

  virtual VrmlNode *cloneMe() const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void eventIn(double timeStamp,
		       const char *eventName,
		       const VrmlField *fieldValue);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  virtual VrmlNodeScalarInt* toScalarInt() const;
  virtual const VrmlMFFloat& getKey() const;
  virtual const VrmlMFFloat& getKeyValue() const;

private:

  // Fields
  VrmlMFFloat d_key;
  VrmlMFFloat d_keyValue;

  // State
  VrmlSFFloat d_value;
};

#endif // _VRMLNODESCALARINT_

