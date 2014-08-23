//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeCylinderSensor.h

#ifndef  _VRMLNODECYLINDERSENSOR_
#define  _VRMLNODECYLINDERSENSOR_

#include "VrmlNodeChild.h"
#include "VrmlSFBool.h"
#include "VrmlSFFloat.h"
#include "VrmlSFRotation.h"
#include "VrmlSFVec3f.h"

class VrmlScene;


class VrmlNodeCylinderSensor : public VrmlNodeChild {

public:

  // Define the fields of CylinderSensor nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeCylinderSensor( VrmlScene *scene = 0);
  virtual ~VrmlNodeCylinderSensor();

  virtual VrmlNode *cloneMe() const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  bool isEnabled() { return d_enabled.get(); }

private:

  // Fields
  VrmlSFBool d_autoOffset;
  VrmlSFFloat d_diskAngle;
  VrmlSFBool d_enabled;
  VrmlSFFloat d_maxAngle;
  VrmlSFFloat d_minAngle;
  VrmlSFFloat d_offset;

  VrmlSFBool d_isActive;
  VrmlSFRotation d_rotation;
  VrmlSFVec3f d_trackPoint;

};

#endif // _VRMLNODECYLINDERSENSOR_

