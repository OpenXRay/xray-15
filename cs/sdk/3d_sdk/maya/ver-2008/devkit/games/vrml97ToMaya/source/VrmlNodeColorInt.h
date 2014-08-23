//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeColorInt.h

#ifndef  _VRMLNODECOLORINT_
#define  _VRMLNODECOLORINT_

#include "VrmlNodeChild.h"

#include "VrmlSFFloat.h"
#include "VrmlMFFloat.h"
#include "VrmlSFColor.h"
#include "VrmlMFColor.h"

class VrmlScene;


class VrmlNodeColorInt : public VrmlNodeChild {

public:

  // Define the fields of ColorInt nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeColorInt( VrmlScene *scene = 0 );
  virtual ~VrmlNodeColorInt();

  virtual VrmlNode *cloneMe() const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void eventIn(double timeStamp,
		       const char *eventName,
		       const VrmlField *fieldValue);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

private:

  // Fields
  VrmlMFFloat d_key;
  VrmlMFColor d_keyValue;

  // State
  VrmlSFColor d_value;
};

#endif // _VRMLNODECOLORINT_

