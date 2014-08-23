//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeViewpoint.h

#ifndef  _VRMLNODEVIEWPOINT_
#define  _VRMLNODEVIEWPOINT_

#include "VrmlNodeChild.h"
#include "VrmlField.h"
#include "VrmlSFBool.h"
#include "VrmlSFFloat.h"
#include "VrmlSFRotation.h"
#include "VrmlSFString.h"
#include "VrmlSFVec3f.h"

class VrmlScene;

class VrmlNodeViewpoint : public VrmlNodeChild {

public:

  // Define the fields of Viewpoint nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeViewpoint(VrmlScene *);
  virtual ~VrmlNodeViewpoint();

  virtual VrmlNode *cloneMe() const;

  virtual VrmlNodeViewpoint* toViewpoint() const;

  virtual void addToScene( VrmlScene *s, const char *relUrl );

  virtual ostream& printFields(ostream& os, int indent);

  virtual void eventIn(double timeStamp,
		       const char *eventName,
		       const VrmlField *fieldValue);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  virtual void accumulateTransform( VrmlNode* );
  virtual VrmlNode* getParentTransform();


  float fieldOfView()		{ return d_fieldOfView.get(); }
  float orientationX()		{ return d_orientation.x(); }
  float orientationY()		{ return d_orientation.y(); }
  float orientationZ()		{ return d_orientation.z(); }
  float orientationR()		{ return d_orientation.r(); }
  float positionX()		{ return d_position.x(); }
  float positionY()		{ return d_position.y(); }
  float positionZ()		{ return d_position.z(); }

  const char *description() { return d_description.get() ? d_description.get() : ""; }

private:

  VrmlSFFloat d_fieldOfView;
  VrmlSFBool d_jump;
  VrmlSFRotation d_orientation;
  VrmlSFVec3f d_position;
  VrmlSFString d_description;

  VrmlNode *d_parentTransform;

};

#endif //  _VRMLNODEVIEWPOINT_

