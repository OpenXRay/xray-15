//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeColor.h

#ifndef  _VRMLNODECOLOR_
#define  _VRMLNODECOLOR_

#include "VrmlNode.h"
#include "VrmlMFColor.h"

class VrmlScene;

class VrmlNodeColor : public VrmlNode {

public:

  // Define the fields of Color nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeColor(VrmlScene *);
  virtual ~VrmlNodeColor();

  virtual VrmlNode *cloneMe() const;

  virtual VrmlNodeColor* toColor() const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  VrmlMFColor &color()	{ return d_color; }

private:

  VrmlMFColor d_color;
  
};

#endif // _VRMLNODECOLOR_

