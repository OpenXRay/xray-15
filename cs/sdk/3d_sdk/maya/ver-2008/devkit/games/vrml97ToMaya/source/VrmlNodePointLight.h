//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodePointLight.h

#ifndef  _VRMLNODEPOINTLIGHT_
#define  _VRMLNODEPOINTLIGHT_

#include "VrmlNodeLight.h"
#include "VrmlSFFloat.h"
#include "VrmlSFVec3f.h"


class VrmlNodePointLight : public VrmlNodeLight {

public:

  // Define the fields of pointLight nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodePointLight(VrmlScene *);
  virtual ~VrmlNodePointLight();

  virtual VrmlNode *cloneMe() const;

  virtual VrmlNodePointLight* toPointLight() const;

  // Bindable/scoped nodes must notify the scene of their existence.
  virtual void addToScene( VrmlScene *s, const char *relUrl );

  virtual ostream& printFields(ostream& os, int indent);

  virtual void renderScoped(Viewer *);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  virtual const VrmlSFVec3f& getAttenuation() const;  //LarryD Mar 04/99
  virtual const VrmlSFVec3f& getLocation() const;  //LarryD Mar 04/99
  virtual float getRadius(){ return d_radius.get(); }    //LarryD Mar 04/99

protected:

  VrmlSFVec3f d_attenuation;
  VrmlSFVec3f d_location;
  VrmlSFFloat d_radius;

};

#endif // _VRMLNODEPOINTLIGHT_
