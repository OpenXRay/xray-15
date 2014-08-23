//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeTextureCoordinate.h

#ifndef  _VRMLNODETEXTURECOORDINATE_
#define  _VRMLNODETEXTURECOORDINATE_

#include "VrmlNode.h"
#include "VrmlMFVec2f.h"


class VrmlNodeTextureCoordinate : public VrmlNode {

public:

  // Define the fields of TextureCoordinate nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeTextureCoordinate(VrmlScene *);
  virtual ~VrmlNodeTextureCoordinate();

  virtual VrmlNode *cloneMe() const;

  virtual VrmlNodeTextureCoordinate* toTextureCoordinate() const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  VrmlMFVec2f &coordinate()	{ return d_point; }

private:

  VrmlMFVec2f d_point;
  
};

#endif // _VRMLNODETEXTURECOORDINATE_

