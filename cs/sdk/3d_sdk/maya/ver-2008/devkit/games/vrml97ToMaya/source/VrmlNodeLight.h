//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeLight.h

#ifndef  _VRMLNODELIGHT_
#define  _VRMLNODELIGHT_

#include "VrmlNodeChild.h"
#include "VrmlSFBool.h"
#include "VrmlSFColor.h"
#include "VrmlSFFloat.h"


class VrmlNodeLight : public VrmlNodeChild {

public:

  // Define the fields of light nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeLight(VrmlScene *);
  virtual ~VrmlNodeLight();

  virtual VrmlNodeLight* toLight() const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void renderScoped(Viewer *);

  virtual void setField(const char *fieldName,
			const VrmlField &fieldValue);

  virtual float getAmbientIntensity() { return d_ambientIntensity.get(); } //LarryD Mar 04/99
  virtual float getIntensity() { return d_intensity.get(); } //LarryD Mar 04/99
  virtual bool  getOn() { return d_on.get(); } //LarryD Mar 04/99
  virtual float *getColor(){ return d_color.get(); }  //LarryD Mar 04/99

protected:

  VrmlSFFloat d_ambientIntensity;
  VrmlSFColor d_color;
  VrmlSFFloat d_intensity;
  VrmlSFBool d_on;
};

#endif // _VRMLNODELIGHT_
