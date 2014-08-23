//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeSpotLight.h

#ifndef  _VRMLNODESPOTLIGHT_
#define  _VRMLNODESPOTLIGHT_

#include "VrmlNodeLight.h"
#include "VrmlSFFloat.h"
#include "VrmlSFVec3f.h"


class VrmlNodeSpotLight : public VrmlNodeLight {

public:

  // Define the fields of spotLight nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeSpotLight(VrmlScene *);
  virtual ~VrmlNodeSpotLight();

  virtual VrmlNode *cloneMe() const;

  virtual VrmlNodeSpotLight* toSpotLight() const;

  virtual void addToScene( VrmlScene *s, const char * );

  virtual ostream& printFields(ostream& os, int indent);

  virtual void renderScoped(Viewer *);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  virtual const VrmlSFVec3f& getAttenuation() const;  //LarryD Mar 04/99
  virtual const VrmlSFVec3f& getDirection() const;  //LarryD Mar 04/99
  virtual const VrmlSFVec3f& getLocation() const;  //LarryD Mar 04/99
  virtual float getBeamWidth() { return d_beamWidth.get(); }  //LarryD Mar 04/99
  virtual float getCutOffAngle() { return d_cutOffAngle.get(); } //LarryD Mar 04/99
  virtual float getRadius() { return d_radius.get(); }  //LarryD Mar 04/99

protected:

  VrmlSFVec3f d_attenuation;
  VrmlSFFloat d_beamWidth;
  VrmlSFFloat d_cutOffAngle;
  VrmlSFVec3f d_direction;
  VrmlSFVec3f d_location;
  VrmlSFFloat d_radius;

};

#endif // _VRMLNODESPOTLIGHT_
