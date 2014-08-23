//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeCollision.h

#ifndef  _VRMLNODECOLLISION_
#define  _VRMLNODECOLLISION_

#include "VrmlNodeGroup.h"
#include "VrmlSFBool.h"
#include "VrmlSFNode.h"
#include "VrmlSFTime.h"


class VrmlNodeCollision : public VrmlNodeGroup {

public:

  // Define the fields of Collision nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeCollision(VrmlScene *);
  virtual ~VrmlNodeCollision();

  virtual VrmlNode *cloneMe() const;
  virtual void cloneChildren(VrmlNamespace *);

  virtual bool isModified() const;
  virtual void clearFlags();	// Clear childrens flags too.

  virtual void addToScene( VrmlScene *s, const char *rel );

  virtual void copyRoutes(VrmlNamespace *ns) const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

private:

  VrmlSFBool d_collide;
  VrmlSFNode d_proxy;

  // eventOut
  VrmlSFTime d_collideTime;
  
};

#endif // _VRMLNODECOLLISION_

