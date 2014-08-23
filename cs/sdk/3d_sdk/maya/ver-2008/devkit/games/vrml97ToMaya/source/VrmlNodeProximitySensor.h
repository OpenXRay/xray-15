//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeProximitySensor.h

#ifndef  _VRMLNODEPROXIMITYSENSOR_
#define  _VRMLNODEPROXIMITYSENSOR_

#include "VrmlNodeChild.h"
#include "VrmlSFBool.h"
#include "VrmlSFRotation.h"
#include "VrmlSFTime.h"
#include "VrmlSFVec3f.h"

class VrmlScene;


class VrmlNodeProximitySensor : public VrmlNodeChild {

public:

  // Define the fields of ProximitySensor nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeProximitySensor( VrmlScene *scene = 0);
  virtual ~VrmlNodeProximitySensor();

  virtual VrmlNode *cloneMe() const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void render(Viewer *);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

private:

  // Fields
  VrmlSFVec3f d_center;
  VrmlSFBool d_enabled;
  VrmlSFVec3f d_size;

  // Internal state
  VrmlSFBool d_isActive;
  VrmlSFVec3f d_position;
  VrmlSFRotation d_orientation;
  VrmlSFTime d_enterTime;
  VrmlSFTime d_exitTime;
};

#endif // _VRMLNODEPROXIMITYSENSOR_

