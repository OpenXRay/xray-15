//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeFontStyle.h

#ifndef  _VRMLNODEFONTSTYLE_
#define  _VRMLNODEFONTSTYLE_

#include "VrmlNode.h"
#include "VrmlMFString.h"
#include "VrmlSFBool.h"
#include "VrmlSFString.h"
#include "VrmlSFFloat.h"

class VrmlNodeFontStyle : public VrmlNode {

public:

  // Define the fields of FontStyle nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeFontStyle(VrmlScene *);
  virtual ~VrmlNodeFontStyle();

  virtual VrmlNode *cloneMe() const;

  virtual VrmlNodeFontStyle* toFontStyle() const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  VrmlMFString &justify() { return d_justify; }
  float size() { return d_size.get(); }

private:

  VrmlMFString d_family;
  VrmlSFBool d_horizontal;
  VrmlMFString d_justify;
  VrmlSFString d_language;
  VrmlSFBool d_leftToRight;
  VrmlSFFloat d_size;
  VrmlSFFloat d_spacing;
  VrmlSFString d_style;
  VrmlSFBool d_topToBottom;
  
};

#endif // _VRMLNODEFONTSTYLE_

