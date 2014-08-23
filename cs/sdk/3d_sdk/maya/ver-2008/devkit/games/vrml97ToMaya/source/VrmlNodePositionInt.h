//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodePositionInt.h

#ifndef  _VRMLNODEPOSITIONINT_
#define  _VRMLNODEPOSITIONINT_

#include "VrmlNodeChild.h"

#include "VrmlSFFloat.h"
#include "VrmlMFFloat.h"
#include "VrmlSFVec3f.h"
#include "VrmlMFVec3f.h"

class VrmlScene;


class VrmlNodePositionInt : public VrmlNodeChild {

public:

  // Define the fields of PositionInt nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodePositionInt( VrmlScene *scene = 0);
  virtual ~VrmlNodePositionInt();

  virtual VrmlNode *cloneMe() const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void eventIn(double timeStamp,
		       const char *eventName,
		       const VrmlField *fieldValue);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  virtual VrmlNodePositionInt* toPositionInt() const;  
  virtual const VrmlMFFloat& getKey() const;   
  virtual const VrmlMFVec3f& getKeyValue() const;   

private:

  // Fields
  VrmlMFFloat d_key;
  VrmlMFVec3f d_keyValue;

  // State
  VrmlSFVec3f d_value;
};

#endif // _VRMLNODEPOSITIONINT_

