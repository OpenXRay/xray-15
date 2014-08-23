//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeChild.h

#ifndef  _VRMLNODECHILD_
#define  _VRMLNODECHILD_

#include "VrmlNode.h"
class VrmlNodeScene;

class VrmlNodeChild : public VrmlNode {

public:

  // Define the fields of all built in child nodes
  static VrmlNodeType *defineType(VrmlNodeType *t);

  VrmlNodeChild(VrmlScene *);

  virtual VrmlNodeChild* toChild() const;

protected:

};

#endif // _VRMLNODECHILD_

