//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeBindable.h

#ifndef  _VRMLNODEBINDABLE_
#define  _VRMLNODEBINDABLE_

#include "VrmlNodeChild.h"

class VrmlNodeType;
class VrmlField;
class VrmlScene;

class VrmlNodeBindable : public VrmlNodeChild {

public:

  VrmlNodeBindable::VrmlNodeBindable(VrmlScene *s = 0) : VrmlNodeChild(s) {}

  // Define the fields of all built in bindable nodes
  static VrmlNodeType *defineType(VrmlNodeType *t)
    { return VrmlNode::defineType(t); }

  virtual bool isBindableNode()		{ return true; }

};

#endif // _VRMLNODEBINDABLE_

