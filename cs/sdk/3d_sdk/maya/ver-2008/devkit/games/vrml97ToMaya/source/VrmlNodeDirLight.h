//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeDirLight.h

#ifndef  _VRMLNODEDIRLIGHT_
#define  _VRMLNODEDIRLIGHT_

#include "VrmlNodeLight.h"
#include "VrmlSFVec3f.h"


class VrmlNodeDirLight : public VrmlNodeLight {

public:

  // Define the fields of dirLight nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeDirLight(VrmlScene *);
  virtual ~VrmlNodeDirLight();

  virtual VrmlNode *cloneMe() const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void render(Viewer *);

  virtual void setField(const char *fieldName,
			const VrmlField &fieldValue);

  virtual const VrmlSFVec3f& getDirection() const;  //LarryD Mar 04/99
  virtual VrmlNodeDirLight* toDirLight() const;  //LarryD Mar 04/99

protected:

  VrmlSFVec3f d_direction;

};

#endif // _VRMLNODEDIRLIGHT_
